/* Copyright [2013-2018] [Aaron Springstroh, Minimal Graphics Library]

This file is part of the Beyond Dying Skies.

Beyond Dying Skies is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Beyond Dying Skies is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Beyond Dying Skies.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __WORLD__
#define __WORLD__

#include <chrono>
#include <cmath>
#include <cstdint>
#include <game/cgrid.h>
#include <game/drones.h>
#include <game/drops.h>
#include <game/explosive.h>
#include <game/id.h>
#include <game/load_state.h>
#include <game/missiles.h>
#include <game/particle.h>
#include <game/player.h>
#include <game/sky.h>
#include <game/sound.h>
#include <game/static_instance.h>
#include <game/swatch.h>
#include <game/terrain.h>
#include <game/uniforms.h>
#include <min/camera.h>
#include <min/grid.h>
#include <min/physics_nt.h>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace game
{

class world
{
  private:
    static constexpr float _damage_beam = 25.0;
    static constexpr float _damage_charge = 100.0;
    static constexpr float _damage_ex = 50.0;
    static constexpr float _damage_miss = 100.0;
    static constexpr float _damping = 0.1;
    static constexpr float _explode_size = 100.0;
    static constexpr float _explode_speed = 5.0;
    static constexpr float _explode_time = 5.0;
    static constexpr float _time_step = 1.0 / 180.0;
    static constexpr float _grav_mag = 10.0;
    static constexpr size_t _pre_max_scale = 5;
    static constexpr size_t _pre_max_vol = _pre_max_scale * _pre_max_scale * _pre_max_scale;
    static constexpr size_t _ray_max_dist = 100;
    static constexpr float _explode_scale = 0.9;

    // Terrain stuff
    cgrid _grid;
    terrain _terrain;
    particle *_particles;
    sound *_sound;
    std::vector<size_t> _view_chunk_index;

    // Physics stuff
    min::vec3<float> _gravity;
    min::physics<float, uint16_t, uint32_t, min::vec3, min::aabbox, min::aabbox, min::grid> _simulation;
    size_t _char_id;

    // Terrain control stuff
    min::mesh<float, uint32_t> _terr_mesh;
    min::vec3<int> _cached_offset;
    min::vec3<int> _preview_offset;
    min::vec3<float> _preview;
    min::vec3<unsigned> _scale;
    bool _edit_mode;
    int8_t _atlas_id;
    swatch _swatch;
    bool _swatch_mode;
    bool _swatch_copy_place;

    // Player
    player _player;
    const min::vec3<unsigned> _ex_radius;

    // Skybox
    sky _sky;

    // Static instances for drones and drops
    static_instance _instance;
    drones _drones;
    drops _drops;
    explosives _explosives;
    missiles _missiles;

    // Random stuff
    std::uniform_int_distribution<uint8_t> _drop_dist;
    std::uniform_real_distribution<float> _drop_off_dist;
    std::uniform_real_distribution<float> _grid_dist;
    std::uniform_real_distribution<float> _health_dist;
    std::uniform_real_distribution<float> _scat_dist;
    std::mt19937 _gen;

    inline unsigned block_remove(const min::vec3<float> &p, const min::vec3<unsigned> &scale)
    {
        const min::vec3<int> offset(1, 1, 1);

        // Offset remove radius for geometry removal
        return _grid.set_geometry(_grid.snap(center_radius(p, scale)), scale, offset, -1, nullptr);
    }
    static inline min::vec3<float> center_radius(const min::vec3<float> &p, const min::vec3<unsigned> &scale)
    {
        const min::vec3<float> offset(scale.x() / 2, scale.y() / 2, scale.z() / 2);
        const min::vec3<float> center = p - offset;

        // return center position
        return center;
    }
    inline size_t character_load(const load_state &state)
    {
        // Get spawn point
        const min::vec3<float> &p = state.get_spawn();

        // Spawn character position
        const min::vec3<float> spawn = ray_spawn(p);

        // Create the physics body
        _char_id = _simulation.add_body(cgrid::player_box(spawn), 10.0);

        // Update recent chunk
        _grid.update_current_chunk(spawn);

        // Remove 3x3 blocks
        block_remove(spawn, _ex_radius);

        // Return the character body id
        return _char_id;
    }
    inline void drone_damage(const size_t drone_index, const min::vec3<unsigned> &scale, const min::vec3<float> &dir, const float size, const float damage)
    {
        // Cache the drone position, no reference here
        const min::vec3<float> p = _drones.position(drone_index);

        // Drone may be removed, do not used drone index after this!!!
        const min::vec3<float> flip = dir * -1.0;
        if (_drones.damage(drone_index, dir, damage))
        {
            // Get experience for each mob kill
            const float exp = _player.get_stats().get_mob_exp();

            // Add player experience
            _player.get_stats().add_exp(exp);

            // Do explode animation for sodium
            explode_block(p, flip, min::vec3<unsigned>(3, 5, 3), id_value(block_id::SODIUM), size);
        }
        else
        {
            // Play missile explosion sound
            this->_sound->play_miss_ex(p);

            // Do explode animation
            explode_block(p, flip, scale, id_value(block_id::IRON), size);
        }
    }
    inline auto drone_respawn_call()
    {
        // On collision explode callback
        return [this](void) {
            return this->event_spawn();
        };
    }
    inline min::vec3<float> event_spawn()
    {
        const float x = _grid_dist(_gen);
        const float y = 50.0;
        const float z = _grid_dist(_gen);

        return min::vec3<float>(x, y, z);
    }
    inline auto explode_player_call()
    {
        // Block explosion callback
        return [this](const std::pair<min::aabbox<float, min::vec3>, int8_t> &cell) {

            // Check for exploding mines
            if (cell.second == id_value(block_id::SODIUM))
            {
                // Get box center for explosion point
                const min::vec3<float> point = cell.first.get_center();

                // Calculate direction
                const min::vec3<float> dir = (_player.position() - point).normalize();

                // Explode the block with radius
                this->explode_block(point, dir, _ex_radius, cell.second, _explode_size);
            }
        };
    }
    inline auto explode_call()
    {
        // On collision explode callback
        return [this](const min::vec3<float> &point,
                      const min::vec3<unsigned> &scale,
                      const int8_t value) {

            // Play missile explosion sound
            this->_sound->play_miss_ex(point);

            // Calculate direction
            const min::vec3<float> dir = (_player.position() - point).normalize();

            // Explode the block
            this->explode_block(point, dir, scale, value, _explode_size);
        };
    }
    inline void explode_anim(const min::vec3<float> &point, const min::vec3<float> &dir, const min::vec3<unsigned> &scale, const int8_t value, const float size)
    {
        // Calculate explosion speed
        const min::vec3<float> speed = dir * _explode_speed;

        // Add particle effects
        _particles->load_static_explode(point, speed, _explode_time, size);

        // Get player position
        const min::vec3<float> &p = _player.position();

        // Check if character is too close to the explosion
        const auto pack = in_range_explode(p, point, scale);
        const bool in_range = std::get<0>(pack);
        const float ex_size = std::get<1>(pack);
        const float sq_dist = std::get<2>(pack);

        // If block is lava, play exploding sound
        // Prefer stereo if close to the explosion
        float power = 25.0;
        if (in_range && value == id_value(block_id::SODIUM))
        {
            // Play explode sound
            power *= 16.0;
            _sound->play_explode_stereo(point);
        }
        else if (value == id_value(block_id::SODIUM))
        {
            // Play explode sound
            _sound->play_explode_mono(point);
        }

        // If explode hasn't been flagged yet
        if (!_player.is_exploded() && in_range)
        {
            _player.explode(dir, power, ex_size, sq_dist, value);
        }
    }
    inline void explode_block(const min::vec3<float> &point, const min::vec3<float> &dir, const min::vec3<unsigned> &scale, const int8_t value, const float size)
    {
        // Offset explosion radius for geometry removal
        const min::vec3<float> center = center_radius(point, scale);

        // On remove callback
        const auto f = [this, dir](const min::vec3<float> &point, const int8_t value) {

            // Add a drop
            this->_drops.add(this->random_drop_offset(point), dir, value);

            // Randomly drop a powerup
            const uint8_t ran_drop = this->random_drop();
            if (ran_drop < 2)
            {
                const int8_t drop_id = id_value(block_id::CRYSTAL_R) + ran_drop;
                this->_drops.add(this->random_drop_offset(point), dir, drop_id);
            }
        };

        // If we removed geometry do explode animation
        const min::vec3<int> offset(1, 1, 1);
        _grid.set_geometry(center, scale, offset, -1, f);

        // Do explode animation
        explode_anim(point, dir, scale, value, size);
    }
    bool explode_ray(const min::ray<float, min::vec3> &r,
                     const min::vec3<float> &target,
                     const int8_t value,
                     const min::vec3<unsigned> &scale,
                     const std::function<void(const min::vec3<float> &, min::body<float, min::vec3> &)> &f,
                     const float size, const bool is_charge)
    {
        // Check for collisions with a physics body
        const std::vector<uint16_t> &map = _simulation.get_index_map();
        const std::vector<std::pair<uint16_t, min::vec3<float>>> &cols = _simulation.get_collisions(r);

        // Iterate through all the hits
        const size_t hits = cols.size();
        for (size_t i = 0; i < hits; i++)
        {
            // Get the body
            const min::body<float, min::vec3> &b = _simulation.get_body(map[cols[i].first]);

            // Check if body isn't dead and if body is a drone
            if (!b.is_dead() && b.get_id() == 1)
            {
                // Get the drone index from the body
                const size_t drone_index = b.get_data().index;

                // Get the explosion direction, cache
                const min::vec3<float> &dir = r.get_direction();

                // Choose damage type and add the damage multiplier to it
                const float damage = _player.get_stats().do_damage((is_charge) ? _damage_charge : _damage_beam);

                // Do damage to the drone
                drone_damage(drone_index, scale, dir, size, damage);

                // Exit out early no terrain hit
                return false;
            }
        }

        // Get the atlas of target block, remove if hit
        if (value >= 0)
        {
            // Invoke the function callback if provided
            if (f)
            {
                // Get character body
                min::body<float, min::vec3> &body = _simulation.get_body(_char_id);

                // Call function callback
                f(target, body);
            }

            // Calculate direction
            const min::vec3<float> dir = (_player.position() - target).normalize();

            // Explode block
            if (value == id_value(block_id::SODIUM))
            {
                explode_block(target, dir, _ex_radius, value, size);
            }
            else
            {
                explode_block(target, dir, scale, value, size);
            }
        }

        // Return terrain hit
        return true;
    }
    // Generates the preview geometry and adds it to preview buffer
    inline void generate_preview()
    {
        // Lock in the preview offset
        _preview_offset = _cached_offset;

        // Load data into mesh
        if (_swatch_mode)
        {
            // Update the swatch scale
            _swatch.set_length(_scale);

            // Update the swatch offset
            _swatch.set_offset(_preview_offset);

            // If generating a swatch preview
            _grid.preview_swatch(_terr_mesh, _swatch);
        }
        else
        {
            // If generating a block preview
            _grid.preview_atlas(_terr_mesh, _preview_offset, _scale, _atlas_id);
        }

        // Upload preview geometry
        _terrain.upload_preview(_terr_mesh);
    }
    inline std::tuple<bool, float, float> in_range_explode(const min::vec3<float> &p1, const min::vec3<float> &p2, const min::vec3<unsigned> &scale) const
    {
        // Calculate the size of the explosion
        const float ex_squared_radius = scale.dot(scale);

        // Calculate squared distance from explosion center
        const min::vec3<float> dp = p2 - p1;
        const float sq_dist = dp.dot(dp);

        // Check if character is too close to the explosion
        return {sq_dist < ex_squared_radius, ex_squared_radius, sq_dist};
    }
    inline void item_extra(inventory &inv, const int8_t atlas)
    {
        // Add extra pickup to inventory
        uint8_t count = 1;
        switch (atlas)
        {
        case id_value(block_id::GRASS1):
            inv.add(id_value(item_id::AN_PHOS), count);
            break;
        case id_value(block_id::GRASS2):
            inv.add(id_value(item_id::AN_SULPH), count);
            break;
        case id_value(block_id::DIRT1):
            inv.add(id_value(item_id::CAT_K), count);
            break;
        case id_value(block_id::DIRT2):
            inv.add(id_value(item_id::CAT_NH4), count);
            break;
        case id_value(block_id::SAND1):
            inv.add(id_value(item_id::CAT_NA), count);
            break;
        case id_value(block_id::SAND2):
            inv.add(id_value(item_id::CAT_FE), count);
            break;
        case id_value(block_id::IRON):
            inv.add(id_value(item_id::POWD_RUST), count);
            break;
        case id_value(block_id::WOOD1):
        case id_value(block_id::WOOD2):
            inv.add(id_value(item_id::POWD_CHARCOAL), count);
            break;
        case id_value(block_id::LEAF1):
        case id_value(block_id::LEAF2):
        case id_value(block_id::LEAF3):
        case id_value(block_id::LEAF4):
            inv.add(id_value(item_id::POWD_GUANO), count);
            break;
        case id_value(block_id::STONE1):
        case id_value(block_id::STONE2):
            inv.add(id_value(item_id::POWD_SALT), count);
            break;
        }
    }
    inline uint8_t random_drop()
    {
        return _drop_dist(_gen);
    }
    inline min::vec3<float> random_drop_offset(const min::vec3<float> &p)
    {
        // Calculate offset
        const float x = _drop_off_dist(_gen);
        const float y = _drop_off_dist(_gen);
        const float z = _drop_off_dist(_gen);

        // Add offset to point
        return p + min::vec3<float>(x, y, z);
    }
    inline min::vec3<float> ray_spawn(const min::vec3<float> &p)
    {
        // Create a ray point down
        const min::ray<float, min::vec3> r(p, p - min::vec3<float>::up());

        // Trace a ray to the destination point to find placement position, return point is snapped
        return _grid.ray_trace_prev(r, _ray_max_dist);
    }
    inline void reserve_memory(const size_t view_chunk_size)
    {
        // Reserve space in the simulation
        _simulation.reserve(static_instance::max_alloc());

        // Reserve space in the preview mesh
        _terr_mesh.vertex.reserve(_pre_max_vol);
        _terr_mesh.index.reserve(_pre_max_vol);

        // Reserve space for view chunks
        _view_chunk_index.reserve(view_chunk_size * view_chunk_size * view_chunk_size);
    }
    inline void set_collision_callbacks()
    {
        // Player collision callback
        const auto f = [this](min::body<float, min::vec3> &b1, min::body<float, min::vec3> &b2) {

            // Get other body id, b1 is player body
            // 1 is drone
            // 2 is drop
            const size_t id = b2.get_id();
            if (id == 1)
            {
                // If player is not dead attack
                if (!this->_player.is_dead())
                {
                    // Player collided with a drone
                    this->_player.drone_collide(b2.get_position());

                    // Play the zap sound
                    this->_sound->play_zap();
                }
            }
            else if (id == 2)
            {
                // Get the drop index from the body
                const size_t index = b2.get_data().index;

                // Get the player inventory
                inventory &inv = this->_player.get_inventory();

                // Get block atlas id
                const int8_t atlas = this->_drops.atlas(index);

                // Get the atlas id from the drop
                const uint8_t inv_id = inv.id_from_atlas(atlas);

                // Add drop to inventory
                uint8_t count = 1;
                inv.add(inv_id, count);

                // If we picked it up
                if (count == 0)
                {
                    // Calculate random extra item
                    if (random_drop() < 8)
                    {
                        item_extra(inv, atlas);
                    }

                    // Play the pickup sound
                    this->_sound->play_pickup();

                    // Remove drop from drop buffer
                    this->_drops.remove(index);

                    // Give player experience
                    const float exp = _player.get_stats().get_drop_exp();

                    // Add player experience
                    _player.get_stats().add_exp(exp);
                }
            }
        };

        // Register player collision callback
        _simulation.register_callback(_char_id, f);

        // Explosive collision callback
        const auto h = [this](min::body<float, min::vec3> &b1, min::body<float, min::vec3> &b2) {

            // Get other body id, b1 is explosive, 1 is drone
            if (b2.get_id() == 1)
            {
                // Get the explode index from the body
                const size_t exp_index = b1.get_data().index;

                // Remove this explosive
                this->_explosives.explode(exp_index, id_value(block_id::IRON), nullptr);

                // Get the drone index from the body
                const size_t drone_index = b2.get_data().index;

                // Get the explosion direction
                const min::vec3<float> dir = (b2.get_position() - b1.get_position()).normalize();

                // Add the damage multiplier to damage
                const float damage = _player.get_stats().do_damage(_damage_ex);

                // Do damage to the drone
                drone_damage(drone_index, _ex_radius, dir, _explode_size, damage);
            }
        };

        _explosives.set_collision_callback(h);

        // Missile collision callback
        const auto j = [this](min::body<float, min::vec3> &b1, min::body<float, min::vec3> &b2) {

            // Get other body id, b1 is explosive, 1 is drone
            if (b2.get_id() == 1)
            {
                // Get the explode index from the body
                const size_t miss_index = b1.get_data().index;

                // Remove this missile
                this->_missiles.explode(miss_index, id_value(block_id::IRON), nullptr);

                // Get the drone index from the body
                const size_t drone_index = b2.get_data().index;

                // Get the explosion direction
                const min::vec3<float> dir = (b2.get_position() - b1.get_position()).normalize();

                // Add the damage multiplier to damage
                const float damage = _player.get_stats().do_damage(_damage_miss);

                // Do damage to the drone
                drone_damage(drone_index, _ex_radius, dir, _explode_size, damage);
            }
        };

        _missiles.set_collision_callback(j);
    }
    inline void update_all_chunks()
    {
        // For all chunk meshes
        const size_t size = _grid.get_chunks();
        for (size_t i = 0; i < size; i++)
        {
            // If the chunk needs updating
            if (_grid.is_update_chunk(i))
            {
                // Upload contents to the vertex buffer
                _terrain.upload_geometry(i, _grid.get_chunk(i));

                // Flag that we updated the chunk
                _grid.update_chunk(i);
            }
        }
    }
    inline void update_world_physics(const float dt)
    {
        // Friction Coefficient
        const size_t steps = std::round(dt / _time_step);
        const float friction = -10.0 / steps;
        const float drop_friction = friction * 0.5;

        // Solve the physics simulation
        const min::vec3<float> &p = _player.position();

        // Send drones after the player
        _drones.set_destination(p);

        // Solve all physics timesteps
        for (size_t i = 0; i < steps; i++)
        {
            // Update the player on this frame
            _player.update_frame(_grid, friction, explode_player_call());

            // Update drones on this frame
            _drones.update_frame(_grid, drone_respawn_call());

            // Update drops on this frame
            _drops.update_frame(_grid, drop_friction);

            // Update explosives on this frame
            _explosives.update_frame(_grid, explode_call());

            // Update missiles on this frame
            _missiles.update_frame(_grid, explode_call());

            // Solve all collisions
            _simulation.solve(_time_step, _damping);
        }

        // Update the drones positions
        _drones.update(_grid, p);

        // Update the drop positions
        _drops.update(_grid, dt);

        // Update the explosive positions
        _explosives.update(_grid, dt);

        // Update any missiles
        _missiles.update(_grid);
    }

  public:
    world(const load_state &state, particle &particles, sound &s, const uniforms &uniforms,
          const size_t chunk_size, const size_t grid_size, const size_t view_chunk_size)
        : _grid(chunk_size, grid_size, view_chunk_size),
          _terrain(uniforms, _grid.get_chunks(), chunk_size),
          _particles(&particles),
          _sound(&s),
          _gravity(0.0, -_grav_mag, 0.0),
          _simulation(_grid.get_world(), _gravity),
          _terr_mesh("atlas"),
          _cached_offset(1, 1, 1),
          _preview_offset(1, 1, 1),
          _scale(1, 1, 1),
          _edit_mode(false), _swatch_mode(false),
          _player(&_simulation, state, character_load(state)),
          _ex_radius(3, 3, 3),
          _sky(uniforms),
          _instance(uniforms),
          _drones(_simulation, _instance, s),
          _drops(_simulation, _instance),
          _explosives(_simulation, _instance),
          _missiles(_simulation, particles, _instance, s),
          _drop_dist(0, 20),
          _drop_off_dist(-0.5, 0.5),
          _grid_dist((grid_size * -1.0) + 1.0, grid_size - 1.0),
          _health_dist(0.75, 1.5),
          _scat_dist(-0.1, 0.1),
          _gen(std::chrono::high_resolution_clock::now().time_since_epoch().count())
    {
        // Set the collision elasticity of the physics simulation
        _simulation.set_elasticity(0.1);

        // Set collision callbacks
        set_collision_callbacks();

        // Reserve space for used vectors
        reserve_memory(view_chunk_size);

        // Update chunks
        update_all_chunks();
    }
    inline void add_block(const min::ray<float, min::vec3> &r)
    {
        // Add to grid
        if (_swatch_mode)
        {
            _grid.set_geometry(_swatch, _preview);
        }
        else
        {
            _grid.set_geometry(_preview, _scale, _preview_offset, _atlas_id, nullptr);
        }
    }
    void draw(const uniforms &uniforms) const
    {
        // Draw the static instances
        _instance.draw(uniforms);

        // Binds textures and uses program
        _terrain.bind();

        // Draw the world geometry
        _terrain.draw_terrain(uniforms, _view_chunk_index);

        // Only draw if toggled
        if (_edit_mode)
        {
            // Draw the placemark
            _terrain.draw_placemark(uniforms);
        }

        // Draw the sky, uses geometry VAO -- HACK!
        _sky.draw();
    }
    int8_t explode_ray(const min::vec3<unsigned> &scale,
                       const std::function<void(const min::vec3<float> &, min::body<float, min::vec3> &)> &f,
                       const float size, const bool is_charge)
    {
        // Use player's target as ray destination
        const min::vec3<float> &target = _player.get_target();

        // Check if ray points to a valid target
        const int8_t value = (_player.is_target_valid()) ? _player.get_target_value() : -2;

        // Launch a ray along player's viewing ray
        const bool hit = explode_ray(_player.ray(), target, value, scale, f, size, is_charge);

        // Return the block atlas id if hit
        return (hit) ? value : -2;
    }
    inline void scatter_ray(const min::vec3<unsigned> &scale,
                            const std::function<void(const min::vec3<float> &, min::body<float, min::vec3> &)> &f,
                            const float size)
    {
        // Values for casting rays
        min::vec3<float> target;
        size_t target_key;
        int8_t target_value;

        // Launch N explode rays
        for (size_t i = 0; i < 4; i++)
        {
            // Generate a random scatter offset
            const float x = _scat_dist(_gen);
            const float y = _scat_dist(_gen);
            const float z = _scat_dist(_gen);
            const min::vec3<float> offset(x, y, z);

            // Generate a random ray from the projection
            const min::vec3<float> dest = _player.projection() + offset;
            const min::ray<float, min::vec3> r(_player.position(), dest);

            // Trace a ray to the destination point to find placement position, return point is snapped
            const bool target_valid = _grid.ray_trace_last_key(r, _ray_max_dist, target, target_key, target_value);

            // Check if ray points to a valid target
            const int8_t value = (target_valid) ? target_value : -2;

            // Cast an explode ray on this random ray
            explode_ray(r, target, value, scale, f, size, false);
        }
    }
    inline int8_t get_atlas_id() const
    {
        return _atlas_id;
    }
    inline bool get_swatch_mode() const
    {
        return _swatch_mode;
    }
    inline size_t get_chunks_in_view() const
    {
        return _view_chunk_index.size();
    }
    inline bool get_edit_mode() const
    {
        return _edit_mode;
    }
    inline const cgrid &get_grid() const
    {
        return _grid;
    }
    inline const static_instance &get_instances() const
    {
        return _instance;
    }
    inline size_t get_inst_in_view() const
    {
        return _instance.get_inst_in_view();
    }
    inline player &get_player()
    {
        return _player;
    }
    inline const player &get_player() const
    {
        return _player;
    }
    inline const drones &get_drones() const
    {
        return _drones;
    }
    inline const drops &get_drops() const
    {
        return _drops;
    }
    inline const min::mat4<float> get_preview_matrix() const
    {
        return min::mat4<float>(_preview);
    }
    inline bool hook_set()
    {
        return _player.set_hook();
    }
    inline bool in_range_explosion(const min::vec3<float> &point) const
    {
        return std::get<0>(in_range_explode(_player.position(), point, _ex_radius));
    }
    inline bool launch_explosive()
    {
        // Get player look direction
        const min::vec3<float> p = _player.projection();
        const min::vec3<float> dir = _player.forward();

        // Launch an explosive in front of player
        return _explosives.launch(p, dir, id_value(block_id::SODIUM));
    }
    inline bool launch_missile()
    {
        // Get player look direction
        const min::vec3<float> p = _player.projection();
        const min::vec3<float> dir = _player.forward();

        // Launch a missile in front of player
        return _missiles.launch_missile(p, dir);
    }
    inline void load_swatch()
    {
        // Load data into swatch
        _grid.load_swatch(_swatch, _preview, _preview_offset, _scale);

        // Generate new preview
        generate_preview();
    }
    inline void portal(const load_state &state)
    {
        // Get default spawn point
        const min::vec3<float> &p = state.get_top();

        // Generate a new world in grid
        _grid.portal();

        // Spawn character position
        const min::vec3<float> spawn = ray_spawn(p);

        // Warp player
        _player.warp(spawn);

        // Remove geometry around player
        block_remove(spawn, _ex_radius);

        // Update chunks
        update_all_chunks();
    }
    inline void respawn(const min::vec3<float> &p)
    {
        // Respawn player
        _player.respawn();

        // Spawn character position
        _player.warp(ray_spawn(p));

        // Zero out character velocity
        _player.velocity(min::vec3<float>());
    }
    inline void reset_scale()
    {
        // Reset the scale and the cached offset
        _scale = min::vec3<unsigned>(1, 1, 1);
        _cached_offset = min::vec3<int>(1, 1, 1);

        // Only applicable in edit mode
        if (_edit_mode)
        {
            // Regenerate the preview mesh
            generate_preview();
        }
        else
        {
            _preview_offset = _cached_offset;
        }
    }
    inline uint8_t get_scale_size() const
    {
        return _scale.x() * _scale.y() * _scale.z();
    }
    inline void set_atlas_id(const int8_t id)
    {
        // Only applicable in edit mode
        if (_edit_mode)
        {
            _atlas_id = id;

            // Regenerate the preview mesh
            generate_preview();
        }
    }
    inline void set_edit_mode(const bool edit, const bool swatch, const bool copy)
    {
        // Set flag and return result
        _edit_mode = edit;

        // Set block mode
        _swatch_mode = swatch;

        // Set swatch copy place
        _swatch_copy_place = copy;
    }
    inline void set_scale_x(unsigned dx)
    {
        // Only applicable in edit mode
        if (_edit_mode)
        {
            if (_cached_offset.x() != _preview_offset.x())
            {
                // Regenerate the preview mesh
                generate_preview();
            }
            else if (_scale.x() < _pre_max_scale)
            {
                _scale.x(_scale.x() + dx);

                // Regenerate the preview mesh
                generate_preview();
            }
        }
    }
    inline void set_scale_y(unsigned dy)
    {
        // Only applicable in edit mode
        if (_edit_mode)
        {
            if (_cached_offset.y() != _preview_offset.y())
            {
                // Regenerate the preview mesh
                generate_preview();
            }
            else if (_scale.y() < _pre_max_scale)
            {
                _scale.y(_scale.y() + dy);

                // Regenerate the preview mesh
                generate_preview();
            }
        }
    }
    inline void set_scale_z(unsigned dz)
    {
        // Only applicable in edit mode
        if (_edit_mode)
        {
            if (_cached_offset.z() != _preview_offset.z())
            {
                // Regenerate the preview mesh
                generate_preview();
            }
            else if (_scale.z() < _pre_max_scale)
            {
                _scale.z(_scale.z() + dz);

                // Regenerate the preview mesh
                generate_preview();
            }
        }
    }
    inline void spawn_asteroid()
    {
        const min::vec3<float> down(0.0, -1.0, 0.0);
        _explosives.launch(event_spawn(), down, id_value(block_id::SODIUM));
    }
    inline void kill_drones()
    {
        // Kill all the drones
        _drones.clear();
    }
    inline void spawn_drone()
    {
        // Get the drone health for this player level
        const float drone_health = _player.get_stats().get_drone_health() * _health_dist(_gen);

        // Spawn one drone
        _drones.spawn(event_spawn(), drone_health);
    }
    inline void toggle_swatch_copy_place()
    {
        _swatch_copy_place = !_swatch_copy_place;
    }
    void update(min::camera<float> &cam, const float dt)
    {
        // Update the physics and AI in world
        update_world_physics(dt);

        // Get player position
        const min::vec3<float> &p = _player.position();

        // Reset explosion state
        _player.reset_explode();

        // Set player current target
        _player.set_target(_grid, cam, _ray_max_dist);

        // Detect if we crossed a chunk boundary
        _grid.update_current_chunk(p);

        // Get surrounding chunks for drawing
        _grid.update_view_chunk_index(cam, _view_chunk_index);

// Only used for instance rendering
#ifdef USE_INST_RENDER
        _terrain.update_matrices(cam.get_pv_matrix(), get_preview_matrix());
#endif

        // For all chunk meshes
        for (const auto &i : _view_chunk_index)
        {
            // If the chunk needs updating
            if (_grid.is_update_chunk(i))
            {
                // Upload contents to the vertex buffer
                _terrain.upload_geometry(i, _grid.get_chunk(i));

                // Flag that we updated the chunk
                _grid.update_chunk(i);
            }
        }

        // Update the static instance frustum culling
        _instance.update(_simulation, _grid, cam);

        // Get ray from camera to destination
        const min::ray<float, min::vec3> &r = _player.ray();

        // Trace a ray to the destination point to find placement position, return point is snapped
        // swatch_copy_place == true is copy, == false is default place mode
        if (_swatch_copy_place)
        {
            int8_t value;
            _preview = _grid.ray_trace_last(r, 6, value);
        }
        else
        {
            _preview = _grid.ray_trace_prev(r, 6);
        }

        // Update offset x-vector
        if (cam.get_forward().x() >= 0.0)
        {
            _cached_offset.x(1);
        }
        else
        {
            _cached_offset.x(-1);
        }

        // Update offset z-vector
        if (cam.get_forward().z() >= 0.0)
        {
            _cached_offset.z(1);
        }
        else
        {
            _cached_offset.z(-1);
        }
    }
};
}

#endif
