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
#ifndef __PLAYER__
#define __PLAYER__

#include <game/callback.h>
#include <game/cgrid.h>
#include <game/id.h>
#include <game/inventory.h>
#include <game/load_state.h>
#include <game/skills.h>
#include <game/sound.h>
#include <game/stats.h>
#include <min/aabbox.h>
#include <min/grid.h>
#include <min/physics_nt.h>
#include <min/vec3.h>
#include <vector>

namespace game
{

enum play_mode
{
    none,
    gun,
    place,
    skill,
};

enum class target_id
{
    BLOCK,
    BODY,
    INVALID
};

class target
{
  private:
    union target_value {
        uint_fast16_t body_id;
        block_id atlas;
        target_value(const uint_fast16_t i) : body_id(i) {}
        target_value(const block_id a) : atlas(a) {}
    };

  private:
    target_id _id;
    size_t _key;
    min::vec3<float> _position;
    target_value _value;

  public:
    target()
        : _id(target_id::INVALID), _key(0), _value(static_cast<block_id>(-2)) {}

    block_id &atlas()
    {
        return _value.atlas;
    }
    target_id get_id() const
    {
        return _id;
    }
    const min::vec3<float> &get_position() const
    {
        return _position;
    }
    block_id get_atlas() const
    {
        return _value.atlas;
    }
    uint_fast16_t get_body_index() const
    {
        return _value.body_id;
    }
    size_t &key()
    {
        return _key;
    }
    min::vec3<float> &position()
    {
        return _position;
    }
    void set_body_index(const uint_fast16_t id)
    {
        _value.body_id = id;
    }
    void set_id(const target_id id)
    {
        _id = id;
    }
    void set_position(const min::vec3<float> &p)
    {
        _position = p;
    }
};

class player
{
  private:
    static constexpr float _air_threshold = 1.0;
    static constexpr float _fall_threshold = -1.0;
    static constexpr float _grav_mag = 10.0;
    static constexpr float _project_dist = 1.59;

    physics *_sim;
    sound *_sound;
    size_t _body_id;
    std::vector<std::pair<min::aabbox<float, min::vec3>, block_id>> _col_cells;
    inventory _inv;
    unsigned _damage_cd;
    unsigned _explode_cd;
    bool _exploded;
    block_id _explode_id;
    bool _hooked;
    min::vec3<float> _hook;
    float _hook_length;
    min::vec3<float> _forward;
    min::vec3<float> _project;
    min::ray<float, min::vec3> _ray;
    target _target;
    target _track_target;
    bool _target_update;
    bool _airborn;
    bool _falling;
    size_t _land_count;
    size_t _jump_count;
    bool _landed;
    min::vec3<float> _land_vel;
    bool _jet;
    play_mode _mode;
    skills _skills;
    stats _stats;

    inline void reset_land_info()
    {
        // Reset landed flags
        _land_count = 0;
        _landed = false;

        // Cache the land velocity at time of landing
        _land_vel = velocity();
    }
    inline void reserve_memory()
    {
        _col_cells.reserve(36);
    }
    inline void swing()
    {
        // Get player position
        const min::vec3<float> &p = position();

        // Get player velocity
        const min::vec3<float> &vel = velocity();

        // Compute the hook vector
        const min::vec3<float> hook_dir = (_hook - p);
        const float d = hook_dir.magnitude();

        // Calculate swing direction
        if (d > 1.0)
        {
            // Normalize swing direction
            const min::vec3<float> swing_dir = hook_dir * (1.0 / d);

            // Compute pendulum double spring force
            // Spring F=-k(x-x0)
            const float over = _hook_length + 1.0;
            const float under = _hook_length - 1.0;
            if (d > over)
            {
                const float k = 30.0;
                const min::vec3<float> x = swing_dir * (d - over);
                force(x * k);
            }
            else if (d < under)
            {
                const float k = 15.0;
                const min::vec3<float> x = swing_dir * (d - under);
                force(x * k);
            }

            // Calculate the square velocity
            const float vt = vel.magnitude();
            const float vt2 = vt * vt * 1.25;

            // Gravity acceleration, a = g*cos_theta
            // cos_theta = -swing_dir.dot(gravity) == swing_dir.y()
            const float a1 = _grav_mag * swing_dir.y();

            // Centripetal acceleration a = (vt^2)/L
            const float a2 = vt2 / _hook_length;

            // Combined both forces, along the radius
            const min::vec3<float> tension = swing_dir * ((a1 + a2));

            // Pendulum F=-mg.dot(r) + mvt^2/L
            force(tension);
        }
    }
    inline void update_land(const bool landed)
    {
        // Reset the jump condition if collided with cell, and moving in Y axis
        const min::vec3<float> &v = velocity();
        const float abs_v_y = std::abs(v.y());
        _airborn = abs_v_y >= _air_threshold;
        _falling = v.y() <= _fall_threshold;

        // If we collided with a block and we are not falling, signal landing
        if (landed && !_falling)
        {
            _land_count++;

            // If we have just landed
            if (_land_count == 1)
            {
                // Reset the jump count
                _jump_count = 0;

                // Flag we just landed
                _landed = true;

                // Check for fall damage
                const float speed = _land_vel.magnitude();
                if (speed > 20.0)
                {
                    // Lethal damage
                    const float percent = speed * 0.05;
                    _stats.consume_health(percent * _stats.get_max_health());
                }
                else if (speed > 10.0)
                {
                    const float percent = speed * 0.025;
                    _stats.consume_health(percent * _stats.get_max_health());
                }
            }
            else if (abs_v_y < 0.25)
            {
                // Clamp Y velocity
                body().set_linear_velocity(min::vec3<float>(v.x(), 0.0, v.z()));

                // Apply normal force at surface
                const min::vec3<float> inv_g(0.0, _grav_mag, 0.0);
                force(inv_g);
            }
        }
        else if (!_landed && _falling)
        {
            // Rest land info
            reset_land_info();
        }
    }
    inline void update_position(const float friction)
    {
        // If hooked, add hook forces
        if (_hooked)
        {
            // Calculate forces to make character swing
            swing();
        }
        else if (_jet)
        {
            if (_stats.can_consume_jet())
            {
                // Consume energy
                _stats.consume_jet();

                // Apply force to player body
                force(min::vec3<float>(0.0, 11.0, 0.0));
            }
            else
            {
                _jet = false;
            }
        }
        else
        {
            // Add friction force
            const min::vec3<float> &vel = velocity();
            const min::vec3<float> xz(vel.x(), 0.0, vel.z());

            // If we are in speed mode
            if (_skills.is_speed_mode())
            {
                // Add reduced friction force opposing lateral motion
                force(xz * friction * 0.5);
            }
            else
            {
                // Add friction force opposing lateral motion
                force(xz * friction);
            }
        }
    }
    inline void update_stats()
    {
        // Regen energy
        if (!_skills.is_locked())
        {
            // Rate is units / second
            _stats.regen_energy();
        }

        // Regen health
        if (!_stats.is_dead())
        {
            // Rate is units / second
            _stats.regen_health();
        }

        // Consume oxygen
        _stats.consume_oxygen();
    }

  public:
    player(physics &sim, sound *const s, const load_state &state, const size_t body_id)
        : _sim(&sim), _sound(s), _body_id(body_id),
          _damage_cd(0), _explode_cd(0),
          _exploded(false), _explode_id(block_id::EMPTY),
          _hooked(false), _hook_length(0.0),
          _target_update(false), _airborn(false), _falling(false),
          _land_count(0), _jump_count(0), _landed(false), _jet(false),
          _mode(play_mode::none)
    {
        // Reserve space for collision cells
        reserve_memory();

        // If resuming game
        if (!state.is_new_game())
        {
            // Copy loaded stats
            _stats.fill(state.get_stats(), state.get_energy(),
                        state.get_exp(), state.get_health(),
                        state.get_oxygen(), state.get_stat_points());

            // Copy loaded inventory
            _inv.fill(state.get_inventory(), _stats.level());
        }
    }
    inline const min::body<float, min::vec3> &body() const
    {
        return _sim->get_body(_body_id);
    }
    inline min::body<float, min::vec3> &body()
    {
        return _sim->get_body(_body_id);
    }
    inline void clear_landed()
    {
        _landed = false;
    }
    inline void clear_target_update()
    {
        _target_update = false;
    }
    inline void drone_collide(const min::vec3<float> &p)
    {
        // Apply damage to player
        const float dmg_frac = _stats.level() * 0.01;
        _stats.damage(_stats.get_max_health() * dmg_frac);

        // Calculate collision direction towards player
        const min::vec3<float> dir = (position() - p).normalize();

        // Add a kick force
        force(dir * 1000.0);

        // Set the damage cooldown
        _damage_cd = _physics_frames;
    }
    inline void explode(const min::vec3<float> &dir, const float ex_force, const float dmg_frac, const block_id value)
    {
        // If we haven't been exploded take damage
        if (!_exploded)
        {
            // Signal explode signal
            _exploded = true;

            // Record what we hit
            _explode_id = value;

            // Apply damage and force to the player
            _stats.damage(_stats.get_max_health() * dmg_frac);
            force(dir * ex_force);
        }
    }
    inline void force(const min::vec3<float> &f)
    {
        // Get the drop body
        min::body<float, min::vec3> &b = body();

        // Apply force to the body per mass
        b.add_force(f * b.get_mass());
    }
    inline block_id get_explode_id() const
    {
        return _explode_id;
    }
    inline const min::vec3<float> &get_hook_point() const
    {
        return _hook;
    }
    inline inventory &get_inventory()
    {
        return _inv;
    }
    inline const inventory &get_inventory() const
    {
        return _inv;
    }
    inline const skills &get_skills() const
    {
        return _skills;
    }
    inline skills &get_skills()
    {
        return _skills;
    }
    inline const stats &get_stats() const
    {
        return _stats;
    }
    inline stats &get_stats()
    {
        return _stats;
    }
    inline const target &get_target() const
    {
        return _target;
    }
    inline const target &get_track_target() const
    {
        return _track_target;
    }
    inline block_id get_target_atlas() const
    {
        return _target.get_atlas();
    }
    inline void hook_abort()
    {
        // abort hooking
        _hooked = false;
    }
    inline bool is_airborn() const
    {
        return _airborn;
    }
    inline bool is_action_mode() const
    {
        return _mode == play_mode::gun || _mode == play_mode::skill;
    }
    inline bool is_exploded() const
    {
        return _exploded;
    }
    inline bool is_falling() const
    {
        return _falling;
    }
    inline bool is_hooked() const
    {
        return _hooked;
    }
    inline bool is_jet() const
    {
        return _jet;
    }
    inline bool is_landed()
    {
        return _landed;
    }
    inline bool is_target_block() const
    {
        return _target.get_id() == target_id::BLOCK;
    }
    inline bool is_target_body() const
    {
        return _target.get_id() == target_id::BODY;
    }
    inline bool is_target_update() const
    {
        return _target_update;
    }
    inline void dash()
    {
        // If not hooked
        if (!_hooked && _stats.can_consume_dynamics())
        {
            // Consume energy
            _stats.consume_dynamics();

            // Get the current position and set y movement to zero
            const min::vec3<float> xz(_forward.x(), 0.0, _forward.z());
            const min::vec3<float> zero;
            const min::vec3<float> dxz = min::vec3<float>(xz).normalize_safe(zero);

            // Add force to the player body
            force(dxz * 3000.0);

            // Play the thrust sound
            _sound->play_thrust();
        }
    }
    inline void jump()
    {
        // If not hooked
        if (!_hooked)
        {
            // Allow user to jump and user boosters
            if (_jump_count == 0 && !_airborn)
            {
                // Reset land info
                reset_land_info();

                // Increment jump count
                _jump_count++;

                // Add force to the player body
                force(min::vec3<float>(0.0, 900.0, 0.0));
            }
            else if (_jump_count == 1 && _stats.can_consume_dynamics())
            {
                // Reset land info
                reset_land_info();

                // Increment jump count
                _jump_count++;

                // Consume energy
                _stats.consume_dynamics();

                // Add force to the player body
                force(min::vec3<float>(0.0, 900.0, 0.0));

                // Play the thrust sound
                _sound->play_thrust();
            }
        }
    }
    inline bool is_damageable() const
    {
        return (_damage_cd == 0) && !is_dead();
    }
    inline bool is_dead() const
    {
        return _stats.is_dead();
    }
    inline bool is_explodeable() const
    {
        return (_explode_cd == 0) && !is_dead();
    }
    inline const min::vec3<float> &land_velocity() const
    {
        return _land_vel;
    }
    inline void move(const min::vec3<float> &vel)
    {
        // If not hooked
        if (!_hooked && !_jet)
        {
            // Get the current position and set y movement to zero
            const min::vec3<float> xz(vel.x(), 0.0, vel.z());
            const min::vec3<float> zero;
            const min::vec3<float> dxz = min::vec3<float>(xz).normalize_safe(zero);

            // Add force to player body
            force(dxz * 30.0);
        }
    }
    inline play_mode get_mode() const
    {
        return _mode;
    }
    inline const min::vec3<float> &forward() const
    {
        return _forward;
    }
    inline const min::vec3<float> &position() const
    {
        // Return the character position
        return body().get_position();
    }
    inline const min::vec3<float> &projection() const
    {
        // Return the character position
        return _project;
    }
    inline const min::ray<float, min::vec3> &ray() const
    {
        return _ray;
    }
    inline void reset_explode()
    {
        _exploded = false;
        _explode_id = block_id::EMPTY;
    }
    inline void respawn(const load_state &state)
    {
        // Reset inventory
        _inv.respawn(state.is_hardcore());

        // Reset explode settings
        reset_explode();

        // Reset landed flag
        _hooked = false;
        _landed = false;
        _jet = false;

        // Reset mode
        _mode = play_mode::none;

        // Reset stats
        _stats.respawn();
    }
    inline void set_explode_cd()
    {
        _explode_cd = 5;
    }
    inline bool set_hook()
    {
        // Get the atlas of target block, if hit a block remove it
        if (is_target_block() && not_empty(_target.get_atlas()))
        {
            // Enable hooking
            _hooked = true;

            // Set hook point
            _hook = _target.get_position();

            // Calculate the hook length
            _hook_length = (_hook - position()).magnitude();

            // Return that we are hooked
            return true;
        }

        return false;
    }
    inline void set_jet(const bool flag)
    {
        _jet = flag;
    }
    inline void set_mode(const play_mode &mode)
    {
        _mode = mode;
    }
    inline target target_ray(const cgrid &grid, const min::ray<float, min::vec3> &r, const size_t max_dist) const
    {
        // Output target
        target out;

        // Get ray origin
        const min::vec3<float> &ray_pos = r.get_origin();

        // Trace a ray to the destination point to find placement position, return point is snapped
        const bool target_valid = grid.ray_trace_last_key(r, max_dist, out.position(), out.key(), out.atlas());

        // If ray is invalid or doesn't hit any blocks
        if (!target_valid || !not_empty(out.get_atlas()))
        {
            out.set_id(target_id::INVALID);
        }
        else
        {
            // Set target id to block
            out.set_id(target_id::BLOCK);
        }

        // Calculate distance to block
        const min::vec3<float> block_diff = out.position() - ray_pos;
        float min_dist = block_diff.dot(block_diff);

        // Check for collisions with a physics body before block
        const std::vector<uint_fast16_t> &map = _sim->get_index_map();
        const std::vector<std::pair<uint_fast16_t, min::vec3<float>>> &cols = _sim->get_collisions(r);

        // Iterate through all the hits
        const size_t hits = cols.size();
        for (size_t i = 0; i < hits; i++)
        {
            // Get the body and body id
            const uint_fast16_t body_index = map[cols[i].first];
            const min::body<float, min::vec3> &b = _sim->get_body(body_index);
            if (!b.is_dead() && body_index != _body_id)
            {
                // Get the body position
                const min::vec3<float> &p = b.get_position();

                // Calculate distance between body and player
                const min::vec3<float> body_diff = p - ray_pos;
                const float body_dist = body_diff.dot(body_diff);

                // If this body is closer
                if (body_dist < min_dist)
                {
                    // Update the target information
                    out.set_id(target_id::BODY);
                    out.set_position(p);
                    out.set_body_index(body_index);

                    // Update the minimum distance
                    min_dist = body_dist;

                    // Take first hit and break out
                    break;
                }
            }
        }

        // Return this target
        return out;
    }
    inline const min::vec3<float> &velocity() const
    {
        // Return the character position
        return body().get_linear_velocity();
    }
    inline void velocity(const min::vec3<float> &v)
    {
        // Warp character to new position
        body().set_linear_velocity(v);
    }
    inline void set_position(const min::vec3<float> &p)
    {
        // Warp character to new position
        body().set_position(p);
    }
    template <typename E>
    inline void update_frame(const cgrid &grid, const float friction, const E &ex_call)
    {
        // Check if player is still in the grid
        const min::vec3<float> &p = position();

        // Get all cells that could collide
        grid.player_collision_cells(_col_cells, p);

        // Solve static collisions
        bool landed = false;
        for (const auto &cell : _col_cells)
        {
            // Did we collide with block?
            const bool collide = _sim->collide(_body_id, cell.first);

            // Detect if player has landed
            if (collide)
            {
                // Check if we landed
                const min::vec3<float> center = cell.first.get_center();

                // Calculate minimum gap between player
                constexpr float min_dist = cgrid::_player_dy + 0.475;

                // Compare distances
                if (p.y() - center.y() >= min_dist)
                {
                    landed = true;
                }

                // If we collided with a sodium cell and we haven't exploded yet
                if (!is_exploded() && cell.second == block_id::SODIUM)
                {
                    // Call explosion callback
                    ex_call(cell.first.get_center(), cell.second);
                }
            }
        }

        // Cast a ray to see hovering over a cell
        if (!landed)
        {
            // Cast ray below player
            target t;
            min::ray<float, min::vec3> r(p, p - min::vec3<float>::up());

            // Trace a ray to the block below player to detect if player is falling
            const bool target_valid = grid.ray_trace_last_key(r, 2.0, t.position(), t.key(), t.atlas());
            if (target_valid)
            {
                // Calculate maximum gap between player
                constexpr float max_dist = cgrid::_player_dy + 0.505;

                // Compare distances
                if (p.y() - t.get_position().y() <= max_dist)
                {
                    landed = true;

                    // If we walk over a sodium cell and we haven't exploded yet
                    if (!is_exploded() && t.get_atlas() == block_id::SODIUM)
                    {
                        // Call explosion callback
                        ex_call(t.get_position(), t.get_atlas());
                    }
                }
            }
        }

        // Proces damage and explode cooldowns
        if (_damage_cd > 0)
        {
            _damage_cd--;
        }
        if (_explode_cd > 0)
        {
            _explode_cd--;
        }

        // Update the landed state
        update_land(landed);

        // Update the player position
        update_position(friction);

        // Update the player stats
        update_stats();
    }
    inline void update(min::camera<float> &cam)
    {
        // Cache the forward vector
        _forward = cam.get_forward();

        // Calculate new point to add
        _project = cam.project_point(_project_dist);

        // Cache ray from camera to destination
        _ray = min::ray<float, min::vec3>(cam.get_position(), _project);
    }
    inline void update_target(const cgrid &grid, const bool track_target, const size_t max_dist)
    {
        // Update camera target
        _target = target_ray(grid, _ray, max_dist);

        // If we should update the target
        if (!track_target)
        {
            _track_target = _target;
            _target_update = true;
        }
        else if (_track_target.get_id() == target_id::BODY)
        {
            // Get the target body
            const uint_fast16_t body_index = _track_target.get_body_index();
            const min::body<float, min::vec3> &b = _sim->get_body(body_index);

            // If body is not dead yet
            if (!b.is_dead())
            {
                // Update target position only
                _track_target.set_position(b.get_position());
            }
            else
            {
                _track_target.set_id(target_id::INVALID);
                _target_update = true;
            }
        }
    }
};
}

#endif
