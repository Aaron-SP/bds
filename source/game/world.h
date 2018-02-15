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
    static constexpr float _damping = 0.1;
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
    std::vector<size_t> _view_chunks;

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
    std::uniform_int_distribution<size_t> _drop_dist;
    std::uniform_real_distribution<float> _grid_dist;
    std::mt19937 _gen;

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
        const min::vec3<float> spawned = ray_spawn(p);

        // Create the physics body
        _char_id = _simulation.add_body(cgrid::player_box(spawned), 10.0);

        // Update recent chunk
        _grid.update_current_chunk(spawned);

        // Set scale to 3x3x3
        _scale = min::vec3<unsigned>(3, 3, 3);

        // Remove geometry around player, requires position snapped to grid and calculated direction vector
        _grid.set_geometry(spawned, _scale, _preview_offset, -1);

        // Reset scale to default value
        _scale = min::vec3<unsigned>(1, 1, 1);

        // Return the character body id
        return _char_id;
    }
    inline auto ex_player_call(const min::vec3<float> &p)
    {
        // Block explosion callback
        return [this, p](const std::pair<min::aabbox<float, min::vec3>, int8_t> &cell) {

            // Check for exploding mines
            if (cell.second == id_value(block_id::SODIUM))
            {
                // Get box center for explosion point
                const min::vec3<float> point = cell.first.get_center();

                // Calculate position and direction
                const min::vec3<float> dir = (p - point).normalize();

                // Explode the block with radius
                this->explode_block(point, dir, _ex_radius, cell.second, 100.0);
            }
        };
    }
    inline auto ex_anim_call()
    {
        // On collision explode callback
        return [this](const min::vec3<float> &point,
                      const min::vec3<float> &dir,
                      const min::vec3<unsigned> &scale,
                      const int8_t value) {

            // Play missile explosion sound
            this->_sound->play_miss_ex(point);

            // Explode the block
            this->explode_anim(point, dir, scale, value, 100.0);
        };
    }
    inline auto ex_block_call()
    {
        // On collision explode callback
        return [this](const min::vec3<float> &point,
                      const min::vec3<float> &dir,
                      const min::vec3<unsigned> &scale,
                      const int8_t value) {

            // Play missile explosion sound
            this->_sound->play_miss_ex(point);

            // Explode the block
            this->explode_block(point, dir, scale, value, 100.0);
        };
    }
    inline void explode_anim(const min::vec3<float> &point, const min::vec3<float> &dir, const min::vec3<unsigned> &scale, const int8_t value, const float size = 100.0)
    {
        // Add a drop
        _drops.add(point, dir, value);

        // Randomly drop a powerup
        const size_t ran_drop = random_drop();
        if (ran_drop < 4)
        {
            _drops.add(point, dir, id_value(block_id::CRYSTAL_R) + ran_drop);
        }

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
        float power = 115.5 * ex_size;
        if (in_range && value == id_value(block_id::SODIUM))
        {
            // Play explode sound
            power = 5000.0;
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
            _player.explode(dir, sq_dist, ex_size, power, value);
        }
    }
    inline void explode_block(const min::vec3<float> &point, const min::vec3<float> &dir, const min::vec3<unsigned> &scale, const int8_t value, const float size = 100.0)
    {
        // Offset explosion radius for geometry removal
        const min::vec3<float> center = center_radius(point, scale);

        // If we removed geometry do explode animation
        const unsigned removed = _grid.set_geometry(center, scale, _preview_offset, -1);
        if (removed > 0)
        {
            explode_anim(point, dir, scale, value, size);
        }
    }
    // Generates the preview geometry and adds it to preview buffer
    inline void generate_preview()
    {
        // Lock in the preview offset
        _preview_offset = _cached_offset;

        // Load data into mesh
        _grid.atlas_preview(_terr_mesh, _preview_offset, _scale);

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
    inline size_t random_drop()
    {
        return _drop_dist(_gen);
    }
    inline min::vec3<float> random_spawn()
    {
        const float x = _grid_dist(_gen);
        const float y = 50.0;
        const float z = _grid_dist(_gen);

        return min::vec3<float>(x, y, z);
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
        _view_chunks.reserve(view_chunk_size * view_chunk_size * view_chunk_size);
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
                // Player collided with a drone
                this->_player.drone_collide(b2.get_position());
            }
            else if (id == 2)
            {
                // Get the drop index from the body
                const size_t index = b2.get_data().index;

                // Get the player inventory
                inventory &inv = this->_player.get_inventory();

                // Get the atlas id from the drop
                const uint8_t inv_id = inv.id_from_atlas(this->_drops.atlas(index));

                // Add drop to inventory
                uint8_t count = 1;
                inv.add(inv_id, count);

                // If we picked it up
                if (count == 0)
                {
                    // Play the pickup sound
                    this->_sound->play_pickup();

                    // Remove drop from drop buffer
                    this->_drops.remove(index);
                }
            }
        };

        // Register player collision callback
        _simulation.register_callback(_char_id, f);

        // Drone collision callback
        const auto g = [this](min::body<float, min::vec3> &b1, min::body<float, min::vec3> &b2) {

            // Get other body id, b1 is drone, 3 is grenade, 4 is missile
            const size_t b2_id = b2.get_id();
            if (b2_id == 3 || b2_id == 4)
            {
                // Get the drone index from the body
                const size_t drone_index = b1.get_data().index;

                // Remove this drone
                this->_drones.remove(drone_index);
            }
        };

        _drones.set_collision_callback(g);

        // Explosive collision callback
        const auto h = [this](min::body<float, min::vec3> &b1, min::body<float, min::vec3> &b2) {

            // Get other body id, b1 is explosive, 1 is drone
            if (b2.get_id() == 1)
            {
                // Get the explode index from the body
                const size_t exp_index = b1.get_data().index;

                // Remove this drone
                this->_explosives.explode(exp_index, id_value(block_id::SODIUM), this->ex_anim_call());
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

                // Remove this drone
                this->_missiles.explode(miss_index, id_value(block_id::SODIUM), this->ex_anim_call());
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

        // Solve the physics simulation
        const min::vec3<float> &p = _player.position();

        // Collision explode block callback
        const auto ex_call = ex_block_call();

        // Send drones after the player
        _drones.set_destination(p);

        // Solve all physics timesteps
        for (size_t i = 0; i < steps; i++)
        {
            // Update the player on this frame
            _player.update_frame(_grid, friction, ex_player_call(p));

            // Update drones on this frame
            _drones.update_frame(_grid);

            // Update drops on this frame
            _drops.update_frame(_grid);

            // Update explosives on this frame
            _explosives.update_frame(_grid, ex_call);

            // Update missiles on this frame
            _missiles.update_frame(_grid, ex_call);

            // Solve all collisions
            _simulation.solve(_time_step, _damping);
        }

        // Update the drones positions
        _drones.update(_grid);

        // Update the drop positions
        _drops.update(_grid);

        // Update the explosive positions
        _explosives.update(_grid);

        // Update any missiles
        _missiles.update(_grid);
    }

  public:
    world(const load_state &state, particle *const particles, sound *const s, const uniforms &uniforms,
          const size_t chunk_size, const size_t grid_size, const size_t view_chunk_size)
        : _grid(chunk_size, grid_size, view_chunk_size),
          _terrain(uniforms, _grid.get_chunks(), chunk_size),
          _particles(particles),
          _sound(s),
          _gravity(0.0, -_grav_mag, 0.0),
          _simulation(_grid.get_world(), _gravity),
          _terr_mesh("atlas"),
          _cached_offset(1, 1, 1),
          _preview_offset(1, 1, 1),
          _scale(1, 1, 1),
          _edit_mode(false),
          _player(&_simulation, state, character_load(state)),
          _ex_radius(3, 3, 3),
          _sky(uniforms),
          _instance(uniforms),
          _drones(&_simulation, &_instance),
          _drops(&_simulation, &_instance),
          _explosives(&_simulation, &_instance),
          _missiles(&_simulation, particles, &_instance, s),
          _drop_dist(0, 20),
          _grid_dist((grid_size * -1.0) + 1.0, grid_size - 1.0),
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

        // Spawn maximum drones
        spawn_drones();
    }
    inline void add_block(const min::ray<float, min::vec3> &r)
    {
        // Trace a ray to the destination point to find placement position, return point is snapped
        const min::vec3<float> traced = _grid.ray_trace_prev(r, 6);

        // Add to grid
        _grid.set_geometry(traced, _scale, _preview_offset, _grid.get_atlas());
    }
    void draw(const uniforms &uniforms) const
    {
        // Draw the static instances
        _instance.draw(uniforms);

        // Binds textures and uses program
        _terrain.bind();

        // Draw the world geometry
        _terrain.draw_terrain(uniforms, _view_chunks);

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
                       const std::function<void(const min::vec3<float> &, min::body<float, min::vec3> &)> &f = nullptr, const float size = 100.0)
    {
        if (!_player.is_target_valid())
        {
            return -2;
        }

        // Use player's target as ray destination
        const min::vec3<float> target = _player.get_target();
        const int8_t value = _player.get_target_value();

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

            // If block is lava override explode scale
            min::vec3<unsigned> ex_scale = scale;
            if (value == id_value(block_id::SODIUM))
            {
                ex_scale = _ex_radius;
            }

            // Get direction for particle spray
            const min::vec3<float> direction = (_player.position() - target).normalize();

            // Explode block
            explode_block(target, direction, ex_scale, value, size);
        }

        // return the block atlas id
        return value;
    }
    inline int8_t explode_ray(const std::function<void(const min::vec3<float> &, min::body<float, min::vec3> &)> &f = nullptr, const float size = 100.0)
    {
        return explode_ray(_scale, f, size);
    }
    inline int8_t get_atlas_id() const
    {
        return _grid.get_atlas();
    }
    inline size_t get_chunks_in_view() const
    {
        return _view_chunks.size();
    }
    inline bool get_edit_mode()
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
    inline player &get_player()
    {
        return _player;
    }
    inline const player &get_player() const
    {
        return _player;
    }
    inline drones &get_drones()
    {
        return _drones;
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
    inline void set_atlas_id(const int8_t id)
    {
        // Only applicable in edit mode
        if (_edit_mode)
        {
            _grid.set_atlas(id);

            // Regenerate the preview mesh
            generate_preview();
        }
    }
    inline void set_edit_mode(const bool flag)
    {
        // Toggle flag and return result
        _edit_mode = flag;
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
    inline void spawn_drones()
    {
        while (_drones.spawn(random_spawn()))
        {
        }
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
        _grid.get_view_chunks(cam, _view_chunks);

// Only used for instance rendering
#ifdef USE_INST_RENDER
        _terrain.update_matrices(cam.get_pv_matrix(), get_preview_matrix());
#endif

        // For all chunk meshes
        for (const auto &i : _view_chunks)
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

        // Calculate new placemark point
        const min::vec3<float> dest = cam.project_point(3.0);

        // Create a ray from camera to destination
        const min::ray<float, min::vec3> r(cam.get_position(), dest);

        // Trace a ray to the destination point to find placement position, return point is snapped
        _preview = _grid.ray_trace_prev(r, 6);

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
