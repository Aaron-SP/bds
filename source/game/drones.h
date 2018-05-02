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

#include <game/cgrid.h>
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
    size_t _idle;
    float _health;

  public:
    drone(const size_t body_id, const size_t inst_id,
          const size_t path_id, const size_t sound_id, std::vector<path> *const vp,
          const min::vec3<float> &p, const min::vec3<float> &dest, const float health)
        : _body_id(body_id), _inst_id(inst_id), _path_id(path_id),
          _sound_id(sound_id), _paths(vp), _idle(0), _health(health)
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
    inline bool is_pathing() const
    {
        return _idle == 0;
    }
    inline size_t path_id() const
    {
        return _path_id;
    }
    inline void set_idle(const size_t N)
    {
        _idle = N;
    }
    inline size_t sound_id() const
    {
        return _sound_id;
    }
    inline min::vec3<float> step(cgrid &grid, const float speed)
    {
        return get_path().step(grid) * speed;
    }
};

class drones
{
  private:
    typedef min::physics<float, uint16_t, uint32_t, min::vec3, min::aabbox, min::aabbox, min::grid> physics;
    typedef std::function<void(min::body<float, min::vec3> &, min::body<float, min::vec3> &)> coll_call;
    physics *_sim;
    static_instance *_inst;
    sound *_sound;
    std::vector<min::aabbox<float, min::vec3>> _col_cells;
    min::vec3<float> _dest;
    std::vector<path> _paths;
    std::vector<drone> _drones;
    size_t _path_old;
    coll_call _f;
    bool _disable;

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
        _inst->clear_drone(_drones[index].inst_id());
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
    drones(physics *const sim, static_instance *const inst, sound *const s)
        : _sim(sim), _inst(inst), _sound(s),
          _paths(static_instance::max_drones()), _path_old(0),
          _f(nullptr), _disable(false)
    {
        reserve_memory();
    }
    inline void clear()
    {
        // Kill all the drones
        const size_t size = _drones.size();
        for (size_t i = 0; i < size; i++)
        {
            // Get path id to set dead flag
            const size_t path_id = _drones[i].path_id();
            _paths[path_id].clear();
            _paths[path_id].set_dead(true);

            // Clear drone at index
            _inst->clear_drone(_drones[i].inst_id());
            _sim->clear_body(_drones[i].body_id());
            _sound->stop_drone(_drones[i].sound_id());
        }

        // Clear all the drones
        _drones.clear();
    }
    inline bool damage(const size_t index, const min::vec3<float> &dir, const float dam)
    {
        // Get the drone
        drone &d = _drones[index];

        // Apply a force on the drone body when hit
        force(index, dir * (dam * 100.0));

        // Knock the drone offline for 180 physics frames == 1 sec
        d.set_idle(180);

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
        if (_inst->drone_full())
        {
            return false;
        }

        // Create a drone
        const size_t inst_id = _inst->add_drone(p);

        // Add to physics simulation
        const min::aabbox<float, min::vec3> box = _inst->box_drone(inst_id);

        // Get next index for body
        const size_t index = _drones.size();
        const size_t body_id = _sim->add_body(box, 10.0, 1, index);

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
    inline void warp(const size_t index, const min::vec3<float> &p)
    {
        // Warp character to new position
        body(index).set_position(p);
    }
    inline void update_frame(cgrid &grid, const std::function<min::vec3<float>(void)> &respawn)
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
                if (d.is_pathing())
                {
                    // Get remaining distance
                    const float remain = d.get_path().get_remain();

                    // Calculate the speed of the next step
                    const min::vec3<float> step = d.step(grid, path_speed(remain));

                    // Add velocity to the body
                    body(i).set_linear_velocity(step);

                    // Update the path data position
                    const min::vec3<float> &p = body(i).get_position();
                    d.get_path().update(p, _dest);
                }
                else
                {
                    // Decrement idle frame count
                    d.dec_idle();
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
            grid.drone_collision_cells(_col_cells, position(i));

            // Collision flag
            bool hit = false;

            // Solve static collisions
            const size_t body = d.body_id();
            for (const auto &cell : _col_cells)
            {
                // Collide with the cell
                const bool status = _sim->collide(body, cell);

                // Register hit flag
                hit = hit || status;
            }

            // If we got a hit
            if (hit)
            {
                d.get_path().clear();
            }
        }
    }
    inline void update(cgrid &grid, const min::vec3<float> &look_at)
    {
        // Update all drone positions
        const size_t size = _drones.size();
        for (size_t i = 0; i < size; i++)
        {
            // Get instance and sound id
            const size_t inst_id = _drones[i].inst_id();

            // Update drone instance position
            const min::vec3<float> &p = body(i).get_position();
            _inst->update_drone_position(inst_id, p);

            // Update drone instance rotation
            const min::vec3<float> x(1.0, 0.0, 0.0);
            const min::vec3<float> dir = (look_at - p).normalize();
            const min::quat<float> q(x, dir);
            _inst->update_drone_rotation(inst_id, q);

            // Update the drone sound position
            const size_t sound_id = _drones[i].sound_id();
            _sound->update_drone(sound_id, p);
        }
    }
};
}

#endif
