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
#ifndef __DRONES__
#define __DRONES__

#include <game/callback.h>
#include <game/cgrid.h>
#include <game/id.h>
#include <game/path.h>
#include <game/static_instance.h>
#include <min/aabbox.h>
#include <min/grid.h>
#include <min/physics_nt.h>
#include <min/vec3.h>
#include <vector>

namespace game
{
class drone
{
  private:
    size_t _body_id;
    size_t _inst_id;
    size_t _path_id;
    size_t _sound_id;
    std::vector<path> *_paths;
    float _max_health;
    float _health;
    size_t _idle;
    size_t _launch;

  public:
    drone(const size_t body_id, const size_t inst_id,
          const size_t path_id, const size_t sound_id, std::vector<path> *const vp,
          const min::vec3<float> &p, const min::vec3<float> &dest, const float health)
        : _body_id(body_id), _inst_id(inst_id), _path_id(path_id),
          _sound_id(sound_id), _paths(vp), _max_health(health), _health(health), _idle(0), _launch(0)
    {
        // Reset path and update with new info
        get_path().set_dead(false);
        get_path().update(p, dest);
    }
    inline size_t body_id() const
    {
        return _body_id;
    }
    inline bool damage(const float d)
    {
        // Decriment health
        _health -= d;

        // Return if dead
        return _health <= 0.0;
    }
    inline void dec_idle()
    {
        _idle--;
    }
    inline void dec_inst()
    {
        _inst_id--;
    }
    inline void dec_launch()
    {
        _launch--;
    }
    inline float get_health() const
    {
        return _health;
    }
    inline float get_max_health() const
    {
        return _max_health;
    }
    inline float get_health_percent() const
    {
        return _health / _max_health;
    }
    inline path &get_path()
    {
        return (*_paths)[_path_id];
    }
    inline const path &get_path() const
    {
        return (*_paths)[_path_id];
    }
    inline size_t inst_id() const
    {
        return _inst_id;
    }
    inline bool is_idle() const
    {
        return _idle != 0;
    }
    inline bool is_launching() const
    {
        return _launch == 0;
    }
    inline size_t path_id() const
    {
        return _path_id;
    }
    inline void set_idle(const size_t frames)
    {
        _idle = frames;
    }
    inline size_t sound_id() const
    {
        return _sound_id;
    }
    inline void set_launch(const size_t frames)
    {
        _launch = frames;
    }
    inline min::vec3<float> step(cgrid &grid, const float speed)
    {
        return get_path().step(grid) * speed;
    }
};

class drones
{
  private:
    static constexpr size_t _drone_cooldown = _physics_frames * 10;
    static constexpr uint_fast16_t _missile_level = 5;
    static constexpr uint_fast16_t _splash_level = 10;
    static constexpr uint_fast16_t _tunnel_level = 15;
    physics *const _sim;
    static_instance *const _inst;
    sound *const _sound;
    std::vector<std::pair<min::aabbox<float, min::vec3>, block_id>> _col_cells;
    min::vec3<float> _dest;
    std::vector<path> _paths;
    std::vector<drone> _drones;
    size_t _path_old;
    coll_call _f;
    bool _disable;
    const std::string _str;

    inline min::body<float, min::vec3> &body(const size_t index)
    {
        return _sim->get_body(_drones[index].body_id());
    }
    inline const min::body<float, min::vec3> &body(const size_t index) const
    {
        return _sim->get_body(_drones[index].body_id());
    }
    inline size_t get_idle_path_id()
    {
        // Output id
        size_t id = 0;

        // Scan for unused path
        const size_t size = _paths.size();
        for (size_t i = 0; i < size; i++)
        {
            // Start at the oldest index
            const size_t index = (_path_old %= size)++;

            // If index is unused
            if (_paths[index].is_dead())
            {
                // Assign id for use
                id = index;

                // Break out since found
                break;
            }
        }

        return id;
    }
    inline void force(const size_t index, const min::vec3<float> &f)
    {
        // Get the drop body
        min::body<float, min::vec3> &b = body(index);

        // Apply force to the body per mass
        b.add_force(f * b.get_mass());
    }
    inline static float path_speed(const float remain)
    {
        // Calculate speed slowing down as approaching goal
        return 3.75 * ((remain - 3.0) / (remain + 3.0) + 1.1);
    }
    inline void remove(const size_t index)
    {
        // Get path id to set dead flag
        const size_t path_id = _drones[index].path_id();
        _paths[path_id].clear();
        _paths[path_id].set_dead(true);

        // Clear drone at index
        _inst->get_drone().clear(_drones[index].inst_id());
        _sim->clear_body(_drones[index].body_id());
        _sound->stop_drone(_drones[index].sound_id());
        _drones.erase(_drones.begin() + index);

        // Adjust the remaining drone indices
        const size_t size = _drones.size();
        for (size_t i = index; i < size; i++)
        {
            // Adjust the instance id
            _drones[i].dec_inst();

            // Adjust the body data index
            body(i).set_data(min::body_data(i));
        }
    }
    inline void reserve_memory()
    {
        // Reserve space for collision cells
        _col_cells.reserve(27);
        _drones.reserve(static_instance::max_drones());
    }

  public:
    drones(physics &sim, static_instance &inst, sound &s)
        : _sim(&sim), _inst(&inst), _sound(&s),
          _paths(static_instance::max_drones()), _path_old(0),
          _f(nullptr), _disable(false), _str("Drone")
    {
        reserve_memory();
    }
    inline void reset()
    {
        // Remove all the drones backwards to preserve drone-instance id mapping
        const size_t size = _drones.size();
        for (size_t i = size; i-- != 0;)
        {
            // Get the drone
            const drone &d = _drones[i];

            // Get path id to set dead flag
            const size_t path_id = d.path_id();
            _paths[path_id].clear();
            _paths[path_id].set_dead(true);

            // Clear drone at index
            _inst->get_drone().clear(d.inst_id());
            _sim->clear_body(d.body_id());
            _sound->stop_drone(d.sound_id());
        }

        // Clear all the drones
        _drones.clear();

        // Reset the oldest path
        _path_old = 0;

        // Reset disable flag
        _disable = false;
    }
    inline bool damage(const size_t index, const min::vec3<float> &dir, const float dam)
    {
        // Get the drone
        drone &d = _drones[index];

        // Apply a force on the drone body when hit
        force(index, dir * (dam * 100.0));

        // Knock the drone offline for physics frames == 1 sec
        d.set_idle(_physics_frames);

        // Do damage and return if dead
        if (d.damage(dam))
        {
            // Remove drone
            remove(index);

            // Return remove
            return true;
        }

        // Return no remove
        return false;
    }
    inline float get_health_percent(const size_t index) const
    {
        return _drones[index].get_health_percent();
    }
    inline const std::string &get_string() const
    {
        return _str;
    }
    inline const min::vec3<float> &position(const size_t index) const
    {
        // Return the drone position
        return body(index).get_position();
    }
    inline void set_collision_callback(const coll_call &f)
    {
        _f = f;
    }
    inline void set_destination(const min::vec3<float> &p)
    {
        _dest = p;
    }
    inline size_t size() const
    {
        return _drones.size();
    }
    inline bool spawn(const min::vec3<float> &p, const float health)
    {
        // If full fail spawn
        if (_inst->get_drone().is_full())
        {
            return false;
        }

        // Create a drone
        const size_t inst_id = _inst->get_drone().add(p);

        // Add to physics simulation
        const min::aabbox<float, min::vec3> box = _inst->get_drone().get_box(inst_id);

        // Get next index for body
        const size_t index = _drones.size();
        const size_t body_id = _sim->add_body(box, 10.0, id_value(static_id::DRONE), index);

        // Register player collision callback
        _sim->register_callback(body_id, _f);

        // Get idle path
        const size_t path_id = get_idle_path_id();

        // Get idle sound id
        const size_t sound_id = _sound->get_idle_drone_id();

        // Play the launch sound
        _sound->play_drone(sound_id, p);

        // Add path and path data for drone
        _drones.emplace_back(body_id, inst_id, path_id, sound_id, &_paths, p, _dest, health);

        // Spawned a drone
        return true;
    }
    inline void set_position(const size_t index, const min::vec3<float> &p)
    {
        // Warp character to new position
        body(index).set_position(p);
    }
    template <typename R, typename ES>
    inline void update_frame(cgrid &grid, const uint_fast16_t player_level, const R &respawn, const ES &ex_scale_call)
    {
        // Do drone collisions
        const size_t size = _drones.size();

        // Update drone paths
        if (!_disable)
        {
            for (size_t i = 0; i < size; i++)
            {
                // Get the drone
                drone &d = _drones[i];

                // Only path if not idling
                if (!d.is_idle())
                {
                    // Get remaining distance
                    const float remain = d.get_path().get_remain();

                    // Calculate the speed of the next step
                    const min::vec3<float> step = d.step(grid, path_speed(remain));

                    // Add velocity to the body
                    body(i).set_linear_velocity(step);

                    // Update the path data position
                    const min::vec3<float> &p = position(i);
                    d.get_path().update(p, _dest);
                }
                else
                {
                    // Decrement idle frame count
                    d.dec_idle();
                }

                // Decrement launch counter
                if (!d.is_launching())
                {
                    d.dec_launch();
                }
            }
        }

        // Update drone collisions
        for (size_t i = 0; i < size; i++)
        {
            // Get the drone
            drone &d = _drones[i];

            // Check if drone is stuck
            // Stuck theory
            // 1) A stuck drone will receive a zero path while searching in the grid
            // 2) A zero path triggers the stuck flag
            // 3) We warp the drone below to resolve the issue and clear the flag
            if (d.get_path().is_stuck())
            {
                // Respawn the body
                body(i).set_position(respawn());

                // Clear stuck flag
                d.get_path().clear_stuck();
            }

            // Get all cells that could collide
            const min::vec3<float> &p = position(i);
            grid.drone_collision_cells(_col_cells, p);

            // Collision flag
            bool hit = false;

            // Solve static collisions
            const size_t body = d.body_id();
            for (const auto &cell : _col_cells)
            {
                // Collide with the cell
                const bool status = _sim->collide(body, cell.first);

                // Register hit flag, BUG FIX, DONT COLLAPSE THIS LINE!
                hit = hit || status;
            }

            // If we got a hit
            if (hit)
            {
                // Tunnel through geometry looking for player
                const bool splash = d.is_idle() && (player_level >= _splash_level);
                const bool tunnel = player_level >= _tunnel_level;
                if (splash || tunnel)
                {
                    // Blow up geometry around drone
                    const min::vec3<unsigned> scale(3, 3, 3);

                    // First collision, _col_cells must have a size if hit
                    ex_scale_call(p, scale, _col_cells[0].second);
                }

                // Create new path
                d.get_path().clear();
            }
        }
    }
    template <typename M>
    inline void update(cgrid &grid, const min::vec3<float> &player_pos, const uint_fast16_t player_level, const M &miss_call)
    {
        // Update all drone positions
        const size_t size = _drones.size();
        for (size_t i = 0; i < size; i++)
        {
            // Get the drone
            drone &d = _drones[i];

            // Get instance and sound id
            const size_t inst_id = d.inst_id();

            // Update drone instance position
            const min::vec3<float> &p = body(i).get_position();
            _inst->get_drone().update_position(inst_id, p);

            // Calculate distance to player
            const min::vec3<float> diff = player_pos - p;
            const float dist = diff.magnitude();
            const min::vec3<float> dir = (dist > 0.01) ? diff * (1.0 / dist) : diff;

            // Should we launch missiles
            const bool launch = (player_level >= _missile_level) && d.is_launching() && dist < 5.0;
            if (launch)
            {
                // Launch missile
                miss_call(p, p + dir);

                // Set cooldown
                d.set_launch(_drone_cooldown);
            }

            // Update drone instance rotation
            const min::vec3<float> x(1.0, 0.0, 0.0);
            const min::quat<float> q(x, dir);
            _inst->get_drone().update_rotation(inst_id, q);

            // Update the drone sound position
            const size_t sound_id = _drones[i].sound_id();
            _sound->update_drone(sound_id, p);
        }
    }
};
}

#endif
