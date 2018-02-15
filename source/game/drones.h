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
    std::vector<path> *_paths;

    inline path &get_path()
    {
        return (*_paths)[_path_id];
    }
    inline const path &get_path() const
    {
        return (*_paths)[_path_id];
    }

  public:
    drone(const size_t body_id, const size_t inst_id,
          const size_t path_id, std::vector<path> *const vp,
          const min::vec3<float> &p, const min::vec3<float> &dest)
        : _body_id(body_id), _inst_id(inst_id),
          _path_id(path_id), _paths(vp)
    {
        // Reset path and update with new info
        get_path().set_dead(false);
        get_path().update(p, dest);
    }
    ~drone()
    {
        // Destroy path on destruction
        get_path().clear();
        get_path().set_dead(true);
    }
    inline size_t body_id() const
    {
        return _body_id;
    }
    inline void dec_inst()
    {
        _inst_id--;
    }
    inline size_t inst_id() const
    {
        return _inst_id;
    }
    inline float get_remain() const
    {
        return get_path().get_remain();
    }
    inline min::vec3<float> step(cgrid &grid, const float speed)
    {
        return get_path().step(grid) * speed;
    }
    inline void update(const min::vec3<float> &p, const min::vec3<float> &dest)
    {
        get_path().update(p, dest);
    }
};

class drones
{
  private:
    typedef min::physics<float, uint16_t, uint32_t, min::vec3, min::aabbox, min::aabbox, min::grid> physics;
    typedef std::function<void(min::body<float, min::vec3> &, min::body<float, min::vec3> &)> coll_call;
    physics *_sim;
    static_instance *_inst;
    std::vector<min::aabbox<float, min::vec3>> _col_cells;
    std::vector<drone> _drones;
    min::vec3<float> _dest;
    std::vector<path> _paths;
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
    inline static float path_speed(const float remain)
    {
        // Calculate speed slowing down as approaching goal
        return 2.75 * ((remain - 3.0) / (remain + 3.0) + 1.1);
    }
    inline void path(cgrid &grid, const size_t index)
    {
        // Get the drone
        drone &d = _drones[index];

        // Get remaining distance
        const float remain = d.get_remain();

        // Calculate the speed of the next step
        const min::vec3<float> step = d.step(grid, path_speed(remain));

        // Add velocity to the body
        body(index).set_linear_velocity(step);
    }
    inline const min::vec3<float> &position(const size_t index) const
    {
        // Return the drone position
        return body(index).get_position();
    }
    inline void reserve_memory()
    {
        // Reserve space for collision cells
        _col_cells.reserve(27);
        _drones.reserve(static_instance::max_drones());
    }

  public:
    drones(physics *sim, static_instance *inst)
        : _sim(sim), _inst(inst), _paths(static_instance::max_drones()),
          _path_old(0), _f(nullptr), _disable(false)
    {
        // Reserve memory for collision cells
        reserve_memory();
    }
    inline void remove(const size_t index)
    {
        // Clear drone at index
        _inst->clear_drone(_drones[index].inst_id());
        _sim->clear_body(_drones[index].body_id());
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
    inline void set_collision_callback(const coll_call &f)
    {
        _f = f;
    }
    inline void set_destination(const min::vec3<float> &p)
    {
        _dest = p;
    }
    inline bool spawn(const min::vec3<float> &p)
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

        // Add path and path data for drone
        _drones.emplace_back(body_id, inst_id, path_id, &_paths, p, _dest);

        // Spawned a drone
        return true;
    }
    inline void warp(const size_t index, const min::vec3<float> &p)
    {
        // Warp character to new position
        body(index).set_position(p);
    }
    inline void update_frame(const cgrid &grid, const float dt)
    {
        // Do drone collisions
        const size_t size = _drones.size();
        for (size_t i = 0; i < size; i++)
        {
            // Get all cells that could collide
            grid.drone_collision_cells(_col_cells, position(i));

            // Solve static collisions
            const size_t body = _drones[i].body_id();
            for (const auto &cell : _col_cells)
            {
                _sim->collide(body, cell);
            }
        }
    }
    inline void update(cgrid &grid)
    {
        // Get number of drones created
        const size_t size = _drones.size();

        // Update drone paths
        if (!_disable)
        {
            for (size_t i = 0; i < size; i++)
            {
                path(grid, i);
            }
        }

        // Update all drone positions
        for (size_t i = 0; i < size; i++)
        {
            // Update the path data position
            const min::vec3<float> &p = body(i).get_position();
            _drones[i].update(p, _dest);

            // Update the instance matrix
            const size_t inst_id = _drones[i].inst_id();
            _inst->update_drone_position(inst_id, p);
        }
    }
};
}

#endif
