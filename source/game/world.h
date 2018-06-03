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
#include <game/callback.h>
#include <game/cgrid.h>
#include <game/chests.h>
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
    static constexpr float _spawn_limit = 5.0;
    static constexpr float _time_step = 1.0 / _physics_frames;
    static constexpr size_t _pre_max_scale = 5;
    static constexpr size_t _pre_max_vol = _pre_max_scale * _pre_max_scale * _pre_max_scale;
    static constexpr size_t _ray_max_dist = 100;
    static constexpr float _explode_scale = 0.9;

    // Terrain stuff
    cgrid _grid;
    terrain _terrain;
    particle *const _particles;
    sound *const _sound;
    std::vector<size_t> _view_chunk_index;

    // Physics stuff
    const min::vec3<unsigned> _ex_radius;
    const float _top;
    const min::vec3<float> _gravity;
    min::physics<float, uint_fast16_t, uint_fast32_t, min::vec3, min::aabbox, min::aabbox, min::grid> _simulation;
    size_t _char_id;

    // Terrain control stuff
    min::mesh<float, uint32_t> _terr_mesh;
    min::vec3<int> _cached_offset;
    min::vec3<int> _preview_offset;
    min::vec3<float> _preview;
    min::vec3<unsigned> _scale;
    bool _edit_mode;
    block_id _atlas_id;
    swatch _swatch;
    unsigned _swatch_cost;
    bool _swatch_mode;
    bool _swatch_copy_place;

    // Player
    player _player;

    // Skybox
    sky _sky;

    // Static instances for drones and drops
    static_instance _instance;
    chests _chests;
    drones _drones;
    drops _drops;
    explosives _explosives;
    missiles _missiles;
    const std::string _invalid_str;

    // Random stuff
    std::uniform_real_distribution<float> _crit_dist;
    std::uniform_int_distribution<uint_fast8_t> _drop_dist;
    std::uniform_real_distribution<float> _drop_off_dist;
    std::uniform_real_distribution<float> _ex_mult;
    std::uniform_real_distribution<float> _grid_dist;
    std::uniform_real_distribution<float> _health_dist;
    std::uniform_real_distribution<float> _miss_dist;
    std::uniform_real_distribution<float> _scat_dist;
    std::mt19937 _gen;

    // Private methods
    inline unsigned block_remove(const min::vec3<float> &p, const min::vec3<unsigned> &scale)
    {
        const min::vec3<int> offset(1, 1, 1);

        // Offset remove radius for geometry removal
        return _grid.set_geometry(_grid.snap(center_radius(p, scale)), scale, offset, block_id::EMPTY, nullptr);
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
        // Is this a new game?
        const bool new_game = state.is_new_game();

        // Get spawn point
        const min::vec3<float> &p = state.get_spawn();

        // Spawn character position
        const min::vec3<float> spawn = (new_game) ? ray_spawn(p) : p;

        // Create the physics body
        _char_id = _simulation.add_body(cgrid::player_box(spawn), 10.0);

        // Update recent chunk
        _grid.update_current_chunk(spawn);

        // Remove 3x3 blocks if new game
        if (new_game)
        {
            block_remove(spawn, _ex_radius);
        }

        // Return the character body id
        return _char_id;
    }
    inline dmg_call dmg_default_call()
    {
        // On explode callback, return default damage
        return [this](const float sq_dist, const float ex_size, const block_id atlas) -> std::pair<float, float> {
            // Calculate damage
            // Max power = 400
            // Max size = 63
            // Min dist = 1
            // Max Num = 400 * 63 * 3 = 75600
            // Denom = 400 * 63 = 25200
            // Damage multipler = 0.1 - 3
            const float fraction = this->_ex_mult(_gen) / 25200.0;
            const float min_dist = 1.0;
            const float power = (atlas == block_id::SODIUM) ? 400.0 : 100.0;
            const float force = power * ex_size / std::max(sq_dist, min_dist);
            const float dmg_frac = force * fraction;

            // Calculate fractional damage and explosion force
            return std::make_pair(force, dmg_frac);
        };
    }
    inline dmg_call dmg_drone_call()
    {
        // On damage callback, return drone damage
        return [this](const float sq_dist, const float ex_size, const block_id atlas) -> std::pair<float, float> {
            // Calculate damage
            // Max power = 40
            // Max size = 63
            // Min dist = 1
            // Max Num = 40 * 63 * 3 = 7560
            // Denom = 400 * 63 = 25200
            // Damage multipler = 0.1 - 3
            const float fraction = this->_ex_mult(_gen) / 25200.0;
            const float min_dist = 1.0;
            const float power = (atlas == block_id::SODIUM) ? 40.0 : 10.0;
            const float force = power * ex_size / std::max(sq_dist, min_dist);
            const float dmg_frac = force * fraction;

            // Calculate fractional damage and explosion force
            return std::make_pair(force, dmg_frac);
        };
    }
    inline min::vec3<float> direction(const min::vec3<float> &to, const min::vec3<float> &from) const
    {
        // Calculate direction, safe if zero vector
        return (to - from).normalize_safe(min::vec3<float>::up());
    }
    // Callback functions
    inline auto drone_respawn_call()
    {
        // On collision explode callback
        return [this](void) {
            return this->spawn_event();
        };
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

            // Block atlas ID
            const block_id atlas = block_id::SODIUM;

            // Do explode animation for sodium
            min::vec3<unsigned> ex_scale(3, 5, 3);
            explode(p, flip, ex_scale, atlas, size, nullptr, dmg_drone_call(), sound_choose_call());

            // Drop block
            drop_block(p + flip, flip, atlas);
        }
        else
        {
            // Block atlas ID
            const block_id atlas = block_id::IRON;

            // Do explode animation
            explode(p, flip, scale, atlas, size, nullptr, dmg_drone_call(), sound_ex_call());

            // Drop block
            drop_block(p + flip, flip, atlas);
        }
    }
    inline void drop_block(const min::vec3<float> &p, const min::vec3<float> &dir, const block_id atlas)
    {
        // Add a drop
        _drops.add(random_drop_offset(p), dir, atlas);

        // Randomly drop a powerup
        const uint_fast8_t ran_drop = random_drop();
        if (ran_drop < 4)
        {
            const block_id drop_id = static_cast<block_id>(id_value(block_id::CRYSTAL_R) + ran_drop);
            _drops.add(random_drop_offset(p), dir, drop_id);
        }
    }
    inline ex_scale_call explode_call(const dmg_call &d, const sound_call &s)
    {
        // On collision explode callback
        return [this, d, s](const min::vec3<float> &p,
                            const min::vec3<unsigned> &scale,
                            const block_id atlas) {
            // Calculate direction
            const min::vec3<float> dir = this->direction(this->_player.position(), p);

            // Explode the block
            this->explode(p, dir, scale, atlas, this->_explode_size, nullptr, d, s);
        };
    }
    inline ex_scale_call explode_block_call(const dmg_call &d, const sound_call &s)
    {
        // On collision explode callback
        return [this, d, s](const min::vec3<float> &p,
                            const min::vec3<unsigned> &scale,
                            const block_id atlas) {
            // Calculate direction
            const min::vec3<float> dir = this->direction(this->_player.position(), p);

            // Explode the block
            this->explode_block(p, dir, scale, atlas, this->_explode_size, d, s);
        };
    }
    inline ex_call explode_default_call()
    {
        // Block explosion callback
        return [this](const min::vec3<float> &p, const block_id atlas) {
            return this->explode_call(this->dmg_default_call(), this->sound_default_call())(p, _ex_radius, atlas);
        };
    }
    inline ex_call explode_drop_call()
    {
        // Block explosion callback
        return [this](const min::vec3<float> &p, const block_id atlas) {
            return this->explode_call(this->dmg_default_call(), this->sound_ex_call())(p, _ex_radius, atlas);
        };
    }
    inline void explode_block(
        const min::vec3<float> &p, const min::vec3<float> &dir, const min::vec3<unsigned> &scale,
        const block_id atlas, const float size, const dmg_call &d, const sound_call &s)
    {
        // On remove callback
        const auto f = [this, dir](const min::vec3<float> &p, const block_id atlas) {
            this->drop_block(p, dir, atlas);
        };

        // Do explode with callback
        explode(p, dir, scale, atlas, size, f, d, s);
    }
    inline void explode(
        const min::vec3<float> &p, const min::vec3<float> &dir, const min::vec3<unsigned> &scale,
        const block_id atlas, const float size, const set_call &f, const dmg_call &d, const sound_call &s)
    {
        // Offset explosion radius for geometry removal
        const min::vec3<float> center = center_radius(p, scale);

        // If we removed geometry do explode animation
        const min::vec3<int> offset(1, 1, 1);
        _grid.set_geometry(center, scale, offset, block_id::EMPTY, f);

        // Calculate explosion speed
        const min::vec3<float> speed = dir * _explode_speed;

        // Add particle effects
        _particles->load_static_explode(p, speed, _explode_time, size);

        // Check if character is too close to the explosion
        const auto pack = in_range_explode(_player.position(), p, scale);
        const bool in_range = std::get<0>(pack);

        // Play the sound call
        if (s)
        {
            s(p, in_range, atlas);
        }

        // If explode hasn't been flagged yet
        if (!_player.is_exploded() && in_range && d)
        {
            // Calculate damage from function
            const float ex_size = std::get<1>(pack);
            const float sq_dist = std::get<2>(pack);
            const std::pair<float, float> dp = d(sq_dist, ex_size, atlas);

            // Player takes damage
            _player.explode(dir, dp.first, dp.second, atlas);
        }
    }
    bool explode_ray_body(min::body<float, min::vec3> &b, const min::ray<float, min::vec3> &r,
                          const min::vec3<unsigned> &scale, const float size, const bool is_charge)
    {
        // Check if body isn't dead
        if (!b.is_dead())
        {
            // Get the explosion direction, cache
            const min::vec3<float> &dir = r.get_direction();

            // Switch on body type
            const size_t body_id = b.get_id();
            switch (body_id)
            {
            case id_value(static_id::PLAYER):
                break;
            case id_value(static_id::CHEST):
                break;
            case id_value(static_id::DRONE):
            {
                // Get the drone index from the body
                const size_t index = b.get_data().index;

                // Choose damage type and add the damage multiplier to it
                const float damage = _player.get_stats().do_damage((is_charge) ? _damage_charge : _damage_beam, _crit_dist(_gen));

                // Do damage to the drone
                drone_damage(index, scale, dir, size, damage);
                break;
            }
            default:
            {
                // Apply force to the body per mass along ray direction
                b.add_force(dir * b.get_mass() * 5000.0);
                break;
            }
            }

            // We hit a body
            return true;
        }

        // Didn't hit anything
        return false;
    }
    void explode_ray_block(const min::vec3<float> &p, const block_id atlas,
                           const min::vec3<unsigned> &scale, const float size,
                           const ray_call &f)
    {
        // Invoke the function callback if provided
        if (f)
        {
            // Get character body
            min::body<float, min::vec3> &body = _simulation.get_body(_char_id);

            // Call function callback
            f(body, p);
        }

        // Calculate direction
        const min::vec3<float> dir = direction(_player.position(), p);

        // Explode block
        if (atlas == block_id::SODIUM)
        {
            explode_block(p, dir, _ex_radius, atlas, size, dmg_default_call(), sound_default_call());
        }
        else
        {
            explode_block(p, dir, scale, atlas, size, dmg_default_call(), sound_default_call());
        }
    }
    block_id explode_ray(const min::ray<float, min::vec3> &r, const target &t,
                         const min::vec3<unsigned> &scale, const float size, const bool is_charge, const ray_call &f)
    {
        // Get the target id
        const target_id tid = t.get_id();
        switch (tid)
        {
        case target_id::BLOCK:
        {
            // Check if ray points to a valid target
            const block_id atlas = t.get_atlas();

            // Call explode ray for block collision at target position
            explode_ray_block(t.get_position(), atlas, scale, size, f);

            return atlas;
        }
        case target_id::BODY:
        {
            // Get the target body id
            const uint_fast16_t body_id = t.get_body_index();

            // Get the body being targeted
            min::body<float, min::vec3> &b = _simulation.get_body(body_id);

            // Call explode ray for body collision
            if (explode_ray_body(b, r, scale, size, is_charge))
            {
                // We hit a body
                return block_id::INVALID;
            }
            break;
        }
        default:
            break;
        }

        // Didn't hit anything
        return block_id::EMPTY;
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
    inline void item_extra(inventory &inv, const block_id atlas)
    {
        // Add extra pickup to inventory
        uint_fast8_t count = 1;
        switch (atlas)
        {
        case block_id::GRASS1:
            inv.add(item_id::AN_PHOS, count);
            break;
        case block_id::GRASS2:
            inv.add(item_id::AN_SULPH, count);
            break;
        case block_id::DIRT1:
            inv.add(item_id::CAT_K, count);
            break;
        case block_id::DIRT2:
            inv.add(item_id::CAT_NH4, count);
            break;
        case block_id::SAND1:
            inv.add(item_id::CAT_CA, count);
            break;
        case block_id::SAND2:
            inv.add(item_id::AN_CARB, count);
            break;
        case block_id::IRON:
            inv.add(item_id::POWD_RUST, count);
            break;
        case block_id::WOOD1:
        case block_id::WOOD2:
            inv.add(item_id::POWD_CHARCOAL, count);
            break;
        case block_id::LEAF1:
        case block_id::LEAF2:
        case block_id::LEAF3:
        case block_id::LEAF4:
            inv.add(item_id::POWD_BGUANO, count);
            break;
        case block_id::STONE1:
        case block_id::STONE2:
            inv.add(item_id::POWD_SALT, count);
            break;
        default:
            break;
        }
    }
    inline auto launch_missile_call()
    {
        return [this](const min::vec3<float> &p, const min::vec3<float> &proj) {
            // Generate a random scatter offset
            const float x = this->_miss_dist(this->_gen);
            const float y = this->_miss_dist(this->_gen);
            const float z = this->_miss_dist(this->_gen);
            const min::vec3<float> offset(x, y, z);

            // Launch a missile at proj in dir direction to avoid blowing up self
            const min::vec3<float> dir = this->direction(proj + offset, p);
            return this->_missiles.launch_missile(proj, dir, min::vec3<float>());
        };
    }
    inline void load_chests(const load_state &state)
    {
        if (state.is_new_game())
        {
            while (spawn_chest(spawn_random()))
            {
            }
        }
        else
        {
            // Get the chests to load
            const std::vector<min::vec3<float>> &chests = state.get_chests();

            // Load the persisted chests
            const size_t size = chests.size();
            for (size_t i = 0; i < size; i++)
            {
                spawn_chest(chests[i]);
            }
        }
    }
    inline void play_sodium_blast(const min::vec3<float> &p, const bool in_range, const block_id atlas)
    {
        // Prefer stereo if close to the explosion
        if (in_range)
        {
            _sound->play_blast_stereo(p);
        }
        else
        {
            _sound->play_blast_mono(p);
        }
    }
    inline uint_fast8_t random_drop()
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
        // Reserve space in the simulation for static instances and player
        _simulation.reserve(static_instance::max_alloc() + 1);

        // Reserve space in the preview mesh
        _terr_mesh.vertex.reserve(_pre_max_vol);
        _terr_mesh.index.reserve(_pre_max_vol);

        // Reserve space for view chunks
        _view_chunk_index.reserve(view_chunk_size * view_chunk_size * view_chunk_size);
    }
    inline sound_call sound_default_call()
    {
        // On explode callback, play sodium blast if sodium
        return [this](const min::vec3<float> &p, const bool in_range, const block_id atlas) -> void {
            if (atlas == block_id::SODIUM)
            {
                this->play_sodium_blast(p, in_range, atlas);
            }
        };
    }
    inline sound_call sound_ex_call()
    {
        // On explode callback, play explosion sound
        return [this](const min::vec3<float> &p, const bool in_range, const block_id atlas) -> void {
            this->_sound->play_explode(p);
        };
    }
    inline sound_call sound_choose_call()
    {
        // On explode callback, choose explosion sound
        return [this](const min::vec3<float> &p, const bool in_range, const block_id atlas) -> void {
            // Play sodium blast if sodium
            if (atlas == block_id::SODIUM)
            {
                this->play_sodium_blast(p, in_range, atlas);
            }
            else
            {
                // Play explosion sound
                this->_sound->play_explode(p);
            }
        };
    }
    inline void set_collision_callbacks()
    {
        // Player collision callback
        const auto f = [this](min::body<float, min::vec3> &b1, min::body<float, min::vec3> &b2) {
            // Get other body id, b1 is player body
            const size_t id = b2.get_id();
            if (id == id_value(static_id::DRONE))
            {
                // If player is not dead attack
                if (this->_player.is_damageable())
                {
                    // Player collided with a drone
                    this->_player.drone_collide(b2.get_position());

                    // Play the zap sound
                    this->_sound->play_zap();
                }
            }
            else if (id == id_value(static_id::DROP))
            {
                // Get the drop index from the body
                const size_t index = b2.get_data().index;

                // Get the player inventory
                inventory &inv = this->_player.get_inventory();

                // Get block atlas id
                const block_id atlas = this->_drops.atlas(index);

                // Get the atlas id from the drop
                const item_id it_id = id_from_atlas(atlas);

                // Add drop to inventory
                uint_fast8_t count = 1;
                inv.add(it_id, count);

                // If we picked it up
                if (count == 0)
                {
                    // Calculate random extra item
                    if (this->random_drop() < 16)
                    {
                        this->item_extra(inv, atlas);
                    }

                    // Play the pickup sound
                    this->_sound->play_pickup();

                    // Remove drop from drop buffer
                    this->_drops.remove(index);

                    // Get the player stats
                    stats &stat = this->_player.get_stats();

                    // Give player experience
                    const float exp = stat.get_drop_exp();

                    // Add player experience
                    stat.add_exp(exp);
                }
            }
        };

        // Register player collision callback
        _simulation.register_callback(_char_id, f);

        // Explosive collision callback
        const auto h = [this](min::body<float, min::vec3> &b1, min::body<float, min::vec3> &b2) {
            // Get the explode index from the body
            const size_t exp_index = b1.get_data().index;

            // Get other body id, b1 is explosive
            if (b2.get_id() == id_value(static_id::PLAYER))
            {
                if (this->_player.is_explodeable())
                {
                    // Remove this explosive
                    this->_explosives.explode(exp_index);

                    // Explode player
                    this->explode_call(this->dmg_drone_call(), this->sound_ex_call())(this->_explosives.position(exp_index), this->_explosives.get_scale(), block_id::EMPTY);
                }
            }
            else if (b2.get_id() == id_value(static_id::DRONE))
            {
                // Remove this explosive
                this->_explosives.explode(exp_index);

                // Get the drone index from the body
                const size_t drone_index = b2.get_data().index;

                // Get the explosion direction
                const min::vec3<float> dir = this->direction(b2.get_position(), b1.get_position());

                // Add the damage multiplier to damage
                const float damage = this->_player.get_stats().do_damage(this->_damage_ex, _crit_dist(_gen));

                // Do damage to the drone
                this->drone_damage(drone_index, this->_ex_radius, dir, this->_explode_size, damage);
            }
        };

        _explosives.set_collision_callback(h);

        // Missile collision callback
        const auto j = [this](min::body<float, min::vec3> &b1, min::body<float, min::vec3> &b2) {
            // Get the missile index from the body
            const size_t miss_index = b1.get_data().index;

            // Get other body id, b1 is missile
            if (b2.get_id() == id_value(static_id::PLAYER))
            {
                if (this->_player.is_explodeable())
                {
                    // Remove this missile
                    this->_missiles.explode(miss_index);

                    // Explode player
                    this->explode_call(this->dmg_drone_call(), this->sound_ex_call())(this->_missiles.position(miss_index), this->_missiles.get_scale(), block_id::EMPTY);
                }
            }
            else if (b2.get_id() == id_value(static_id::DRONE))
            {
                // Remove this missile
                this->_missiles.explode(miss_index);

                // Get the drone index from the body
                const size_t drone_index = b2.get_data().index;

                // Get the explosion direction
                const min::vec3<float> dir = this->direction(b2.get_position(), b1.get_position());

                // Add the damage multiplier to damage
                const float damage = this->_player.get_stats().do_damage(this->_damage_miss, _crit_dist(_gen));

                // Do damage to the drone
                this->drone_damage(drone_index, this->_ex_radius, dir, this->_explode_size, damage);
            }
        };

        _missiles.set_collision_callback(j);
    }
    inline min::vec3<float> spawn_event()
    {
        const float x = _grid_dist(_gen);
        const float y = _top - _spawn_limit;
        const float z = _grid_dist(_gen);

        return min::vec3<float>(x, y, z);
    }
    inline min::vec3<float> spawn_random()
    {
        const float x = _grid_dist(_gen);
        const float y = _grid_dist(_gen);
        const float z = _grid_dist(_gen);

        return min::vec3<float>(x, y, z);
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
        const float drop_friction = friction * 2.0;

        // Get player position and player level
        const min::vec3<float> &p = _player.position();
        const uint_fast16_t player_level = _player.get_stats().level();

        // Send drones after the player
        _drones.set_destination(p);

        // Solve all physics timesteps
        for (size_t i = 0; i < steps; i++)
        {
            // Update the player on this frame
            _player.update_frame(_grid, friction, explode_default_call());

            // Update chests on this frame
            _chests.update_frame();

            // Update drones on this frame
            _drones.update_frame(_grid, player_level, drone_respawn_call(), explode_call(dmg_default_call(), sound_choose_call()));

            // Update drops on this frame
            _drops.update_frame(_grid, drop_friction, explode_drop_call());

            // Update explosives on this frame
            _explosives.update_frame(_grid, explode_call(dmg_default_call(), sound_choose_call()));

            // Update missiles on this frame
            _missiles.update_frame(_grid, explode_call(dmg_default_call(), sound_choose_call()));

            // Solve all collisions
            _simulation.solve(_time_step, _damping);
        }

        // Update the chest positions
        _chests.update();

        // Update the drones positions
        _drones.update(_grid, p, player_level, launch_missile_call());

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
          _ex_radius(3, 3, 3),
          _top(state.get_top().y()),
          _gravity(0.0, -_grav_mag, 0.0),
          _simulation(_grid.get_world(), _gravity),
          _terr_mesh("atlas"),
          _cached_offset(1, 1, 1),
          _preview_offset(1, 1, 1),
          _scale(1, 1, 1),
          _edit_mode(false),
          _atlas_id(block_id::EMPTY),
          _swatch_cost(0), _swatch_mode(false), _swatch_copy_place(false),
          _player(&_simulation, state, character_load(state)),
          _sky(uniforms),
          _instance(uniforms),
          _chests(_simulation, _instance),
          _drones(_simulation, _instance, s),
          _drops(_simulation, _instance),
          _explosives(_simulation, _instance),
          _missiles(_simulation, particles, _instance, s),
          _invalid_str("Invalid"),
          _crit_dist(0.5, 2.0),
          _drop_dist(0, 80),
          _drop_off_dist(-0.5, 0.5),
          _ex_mult(0.1, 3.0),
          _grid_dist((grid_size * -1.0) + _spawn_limit, grid_size - _spawn_limit),
          _health_dist(0.75, 1.5),
          _miss_dist(-0.5, 0.5),
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

        // Load chests
        load_chests(state);
    }
    inline void reset(const load_state &state, const size_t chunk_size, const size_t grid_size, const size_t view_chunk_size)
    {
        // Reload grid
        _grid.reset();

        // Reset to default
        _cached_offset = min::vec3<int>(1, 1, 1);
        _preview_offset = min::vec3<int>(1, 1, 1);
        _scale = min::vec3<unsigned>(1, 1, 1);
        _edit_mode = false;
        _atlas_id = block_id::EMPTY;
        _swatch_cost = 0;
        _swatch_mode = false;
        _swatch_copy_place = false;

        // Reset instances
        _chests.reset();
        _drones.reset();
        _drops.reset();
        _explosives.reset();
        _missiles.reset();

        // Prune physics bodies from simulation
        _simulation.clear();

        // Reset player
        _player = player(&_simulation, state, character_load(state));

        // Set collision callbacks
        set_collision_callbacks();

        // Update chunks
        update_all_chunks();

        // Load chests
        load_chests(state);
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
    inline bool can_add_block() const
    {
        // Abort if player is intersecting preview point
        if (_grid.player_box(_player.position()).point_inside(_preview))
        {
            return false;
        }

        return true;
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
    block_id explode_ray(const min::vec3<unsigned> &scale, const float size, const bool is_charge, const ray_call &f)
    {
        return explode_ray(_player.ray(), _player.get_target(), scale, size, is_charge, f);
    }
    inline block_id get_atlas_id() const
    {
        return _atlas_id;
    }
    inline size_t get_chunks_in_view() const
    {
        return _view_chunk_index.size();
    }
    inline const drones &get_drones() const
    {
        return _drones;
    }
    inline const drops &get_drops() const
    {
        return _drops;
    }
    inline bool get_edit_mode() const
    {
        return _edit_mode;
    }
    inline const cgrid &get_grid() const
    {
        return _grid;
    }
    inline const static_instance &get_instance() const
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
    inline const min::mat4<float> get_preview_matrix() const
    {
        return min::mat4<float>(_preview);
    }
    inline uint_fast8_t get_scale_size() const
    {
        return _scale.x() * _scale.y() * _scale.z();
    }
    inline unsigned get_swatch_cost() const
    {
        return _swatch_cost;
    }
    inline bool get_swatch_mode() const
    {
        return _swatch_mode;
    }
    inline std::pair<const std::string *, float> get_target_info(const target &t) const
    {
        const target_id tid = t.get_id();
        const float no_health = -1.0;

        switch (tid)
        {
        case target_id::BLOCK:
        {
            // Get the player inventory
            const inventory &inv = _player.get_inventory();

            // Create item from atlas
            const item_id id = id_from_atlas(t.get_atlas());

            // Look up item name
            return std::make_pair(&inv.get_name(id), no_health);
        }
        case target_id::BODY:
        {
            // Get the body being targeted
            const min::body<float, min::vec3> &b = _simulation.get_body(t.get_body_index());

            // Get the body id
            const size_t body_id = b.get_id();

            // Get the body id
            switch (body_id)
            {
            case id_value(static_id::CHEST):
                return std::make_pair(&_chests.get_string(), no_health);
            case id_value(static_id::DRONE):
            {
                const size_t drone_index = b.get_data().index;
                const float percent = _drones.get_health_percent(drone_index);
                return std::make_pair(&_drones.get_string(), percent);
            }
            case id_value(static_id::DROP):
                return std::make_pair(&_drops.get_string(), no_health);
            case id_value(static_id::EXPLOSIVE):
                return std::make_pair(&_explosives.get_string(), no_health);
            case id_value(static_id::MISSILE):
                return std::make_pair(&_missiles.get_string(), no_health);
            default:
                break;
            }
        }
        default:
            break;
        }

        // Invalid string
        return std::make_pair(&_invalid_str, no_health);
    }
    inline bool hook_set()
    {
        return _player.set_hook();
    }
    inline bool in_range_explosion(const min::vec3<float> &p) const
    {
        return std::get<0>(in_range_explode(_player.position(), p, _ex_radius));
    }
    inline bool is_edit_mode() const
    {
        return _edit_mode;
    }
    inline void kill_drones()
    {
        // Kill all the drones
        _drones.reset();
    }
    inline bool launch_explosive(const min::vec3<float> &up)
    {
        // Get player look direction
        const min::vec3<float> &p = _player.projection();
        const min::vec3<float> &dir = _player.forward();
        const min::vec3<float> &v = _player.velocity();

        // Make player immune to explosions for short time
        _player.set_explode_cd();

        // Launch an explosive in front of player
        return _explosives.launch(p, dir, v, up, block_id::SODIUM);
    }
    inline bool launch_missile()
    {
        // Get player look direction
        const min::vec3<float> &p = _player.projection();
        const min::vec3<float> &dir = _player.forward();
        const min::vec3<float> &v = _player.velocity();

        // Make player immune to explosions for short time
        _player.set_explode_cd();

        // Launch a missile in front of player
        return _missiles.launch_missile(p, dir, v);
    }
    inline void load_swatch()
    {
        // Load data into swatch
        _swatch_cost = _grid.load_swatch(_swatch, _preview, _preview_offset, _scale);

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

        // Remove all chests and spawn new ones
        _chests.reset();
        while (spawn_chest(spawn_random()))
        {
        }

        // Update chunks
        update_all_chunks();
    }
    inline void random_item()
    {
        _player.get_inventory().random_item();
    }
    inline void respawn(const load_state &state)
    {
        // Respawn player
        _player.respawn(state);

        // Spawn character position
        _player.warp(ray_spawn(state.get_default_spawn()));

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
    inline void save()
    {
        _grid.save();
    }
    inline size_t scatter_ray(const min::vec3<unsigned> &scale, const float size, const ray_call &f)
    {
        size_t count = 0;

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
            const min::ray<float, min::vec3> r(_player.ray().get_origin(), dest);

            // Launch a target ray
            const target t = _player.target_ray(_grid, r, _ray_max_dist);

            // Cast an explode ray on this random ray
            if (explode_ray(r, t, scale, size, false, f) != block_id::EMPTY)
            {
                count++;
            }
        }

        return count;
    }
    inline std::pair<bool, static_id> select_target(const target &t)
    {
        const target_id tid = t.get_id();
        switch (tid)
        {
        case target_id::BODY:
        {
            // Get the body being targeted
            const min::body<float, min::vec3> &b = _simulation.get_body(t.get_body_index());

            // Get the body id
            const size_t body_id = b.get_id();

            // Get the body id
            switch (body_id)
            {
            case id_value(static_id::CHEST):
            {
                // If player has keys
                uint_fast8_t count = 1;
                if (_player.get_inventory().consume(item_id::CONS_KEY, count))
                {
                    // Get the index from the body
                    const size_t index = b.get_data().index;
                    _chests.remove(index);

                    // Give player a random item
                    random_item();

                    // Chest selected
                    return std::make_pair(true, static_id::CHEST);
                }
                else
                {
                    return std::make_pair(false, static_id::CHEST);
                }
            }
            default:
                break;
            }
        }
        default:
            break;
        }

        // No selection
        return std::make_pair(false, static_id::PLAYER);
    }
    inline void set_atlas_id(const block_id id)
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
        const min::vec3<float> zero;
        const min::vec3<float> down(0.0, -1.0, 0.0);
        _explosives.launch(spawn_event(), down, zero, zero, block_id::SODIUM);
    }
    inline bool spawn_chest(const min::vec3<float> &position)
    {
        // This point is snapped to the grid
        const min::vec3<float> p = _grid.set_geometry_box_3x3(position, block_id::STONE3);

        // Add the chest
        return _chests.add(min::vec3<float>(p.x(), p.y() - 1.0, p.z()));
    }
    inline void spawn_drone()
    {
        // Get the drone health for this player level
        const float drone_health = _player.get_stats().get_drone_health() * _health_dist(_gen);

        // Spawn one drone
        _drones.spawn(spawn_event(), drone_health);
    }
    inline void toggle_swatch_copy_place()
    {
        _swatch_copy_place = !_swatch_copy_place;
    }
    void update(min::camera<float> &cam, const bool track_target, const float dt)
    {
        // Update the physics and AI in world
        update_world_physics(dt);

        // Get player position
        const min::vec3<float> &p = _player.position();

        // Reset explosion state
        _player.reset_explode();

        // Update player vectors and set target
        _player.update(cam);
        _player.update_target(_grid, track_target, _ray_max_dist);

        // Detect if we crossed a chunk boundary
        _grid.update_current_chunk(p);

        // Get surrounding chunks for drawing
        _grid.update_view_chunk_index(cam, _view_chunk_index);

        // Flush out the update chunks
        _grid.flush_chunk_updates();

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
            block_id value;
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
