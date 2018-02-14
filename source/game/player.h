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

#include <game/inventory.h>
#include <game/load_state.h>
#include <game/skills.h>
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

class player
{
  private:
    static constexpr float _air_threshold = 1.0;
    static constexpr float _fall_threshold = -1.0;
    static constexpr float _grav_mag = 10.0;
    static constexpr float _jet_cost = 0.25;
    static constexpr float _project_dist = 0.5;

    min::physics<float, uint16_t, uint32_t, min::vec3, min::aabbox, min::aabbox, min::grid> *_sim;
    size_t _body_id;
    std::vector<std::pair<min::aabbox<float, min::vec3>, int8_t>> _col_cells;
    inventory _inv;
    bool _exploded;
    int8_t _explode_id;
    bool _hooked;
    min::vec3<float> _hook;
    float _hook_length;
    min::vec3<float> _forward;
    min::vec3<float> _project;
    min::vec3<float> _target;
    size_t _target_key;
    int8_t _target_value;
    bool _target_valid;
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
        _airborn = std::abs(v.y()) >= _air_threshold;
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
                    _stats.consume_health(100.0);
                }
                else if (speed > 10.0)
                {
                    _stats.consume_health(50.0);
                }
            }
        }
        else if (!_landed && _falling)
        {
            // Reset landed flags
            _land_count = 0;
            _landed = false;

            // Cache the land velocity at time of landing
            _land_vel = v;
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
            if (_stats.can_consume_energy(_jet_cost))
            {
                // Consume energy
                _stats.consume_energy(_jet_cost);

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

            // Add friction force opposing lateral motion
            force(xz * friction);
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
    }

  public:
    player(min::physics<float, uint16_t, uint32_t, min::vec3, min::aabbox, min::aabbox, min::grid> *sim, const load_state &state, const size_t body_id)
        : _sim(sim), _body_id(body_id),
          _exploded(false), _explode_id(-1),
          _hooked(false), _hook_length(0.0), _target_key(0), _target_value(-2), _target_valid(false),
          _airborn(false), _falling(false), _land_count(0), _jump_count(0), _landed(false), _jet(false),
          _mode(play_mode::none)
    {
        // Reserve space for collision cells
        reserve_memory();

        // Copy loaded inventory
        _inv.fill(state.get_inventory());
    }
    inline const min::body<float, min::vec3> &body() const
    {
        return _sim->get_body(_body_id);
    }
    inline min::body<float, min::vec3> &body()
    {
        return _sim->get_body(_body_id);
    }
    inline void drone_collide(const min::vec3<float> &p)
    {
        // Consume some of players health
        _stats.consume_health(5);

        // Calculate collision direction towards player
        //const min::vec3<float> dir = (position() - p).normalize();

        // Add a kick force
        //force(dir * 1000.0);
    }
    inline void explode(const min::vec3<float> &dir, const float sq_dist, const float size, const float power, const int8_t value)
    {
        // If we haven't been exploded take damage
        if (!_exploded)
        {
            // Record what we hit
            _explode_id = value;

            // Calculate damage
            const float inv_sq = 1.0 / sq_dist;
            const float mult = power * size * inv_sq * 0.0005;
            const float damage = (mult * 2.0) - (mult * inv_sq);
            _stats.consume_health(damage);

            // Signal explode signal
            _exploded = true;

            // Apply force to the player body
            force(dir * power);
        }
    }
    inline void force(const min::vec3<float> &f)
    {
        // Apply force to the body per mass
        body().add_force(f * mass());
    }
    inline int8_t get_explode_id() const
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
    inline const min::vec3<float> &get_target() const
    {
        return _target;
    }
    inline size_t get_target_key() const
    {
        return _target_key;
    }
    inline int8_t get_target_value() const
    {
        return _target_value;
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
    inline bool is_target_valid() const
    {
        return _target_valid;
    }
    inline void jump()
    {
        // If not hooked
        if (!_hooked)
        {
            // Allow user to jump and user boosters
            if (_jump_count == 0 && !_airborn)
            {
                // Increment jump count
                _jump_count++;

                // Add force to the player body
                force(min::vec3<float>(0.0, 900.0, 0.0));
            }
            else if (_jump_count == 1)
            {
                // Increment jump count
                _jump_count++;

                // Add force to the player body
                force(min::vec3<float>(0.0, 900.0, 0.0));
            }
        }
    }
    inline bool is_dead() const
    {
        return _stats.is_dead();
    }
    inline const min::vec3<float> &land_velocity() const
    {
        return _land_vel;
    }
    inline float mass() const
    {
        return body().get_mass();
    }
    inline void move(const min::vec3<float> &vel)
    {
        // If not hooked
        if (!_hooked && !_jet)
        {
            // Get the current position and set y movement to zero
            const min::vec3<float> dxz = min::vec3<float>(vel.x(), 0.0, vel.z()).normalize();

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
    inline const min::vec3<float> project(const float length) const
    {
        // Project point outward from player
        return position() + forward() * length;
    }
    inline const min::vec3<float> &projection() const
    {
        // Return the character position
        return _project;
    }
    inline void reset_explode()
    {
        _exploded = false;
        _explode_id = -1;
    }
    inline void reset_landed()
    {
        _landed = false;
    }
    inline void respawn()
    {
        // Reset inventory
        _inv.respawn();

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
    inline bool set_hook()
    {
        // Get the atlas of target block, if hit a block remove it
        if (_target_value >= 0)
        {
            // Enable hooking
            _hooked = true;

            // Set hook point
            _hook = _target;

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
    inline void set_target(const cgrid &grid, min::camera<float> &cam, const size_t max_dist)
    {
        // Cache the forward vector
        _forward = cam.get_forward();

        // Calculate new point to add
        _project = cam.project_point(_project_dist);

        // Create a ray from camera to destination
        const min::ray<float, min::vec3> r(cam.get_position(), _project);

        // Trace a ray to the destination point to find placement position, return point is snapped
        _target_valid = grid.ray_trace_last_key(r, max_dist, _target, _target_key, _target_value);
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
    inline void warp(const min::vec3<float> &p)
    {
        // Warp character to new position
        body().set_position(p);
    }
    inline void update_frame(const cgrid &grid, const float friction,
                             const std::function<void(const std::pair<min::aabbox<float, min::vec3>, int8_t> &)> &ex)
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
                const min::vec3<float> center = cell.first.get_center();
                if (center.y() < p.y())
                {
                    landed = true;
                }
            }

            // If we collided with cell
            if (collide && !is_exploded())
            {
                // Call explosion callback
                ex(cell);
            }
        }

        // Update the landed state
        update_land(landed);

        // Update the player position
        update_position(friction);

        // Update the player stats
        update_stats();
    }
};
}

#endif
