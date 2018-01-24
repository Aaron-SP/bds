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
    path_data _data;
    path _path;

  public:
    drone(const size_t body_id, const size_t inst_id, const min::vec3<float> &p, const min::vec3<float> &dest)
        : _body_id(body_id), _inst_id(inst_id), _data(p, dest) {}
    size_t body_id() const
    {
        return _body_id;
    }
    size_t inst_id() const
    {
        return _inst_id;
    }
    path_data &get_data()
    {
        return _data;
    }
    const path_data &get_data() const
    {
        return _data;
    }
    path &get_path()
    {
        return _path;
    }
    const path &get_path() const
    {
        return _path;
    }
};

class drones
{
  private:
    typedef min::physics<float, uint16_t, uint32_t, min::vec3, min::aabbox, min::aabbox, min::grid> physics;
    static constexpr size_t _drone_size = 1;
    physics *_sim;
    static_instance *_inst;
    std::vector<min::aabbox<float, min::vec3>> _col_cells;
    std::vector<drone> _drones;
    min::vec3<float> _dest;
    bool _disable;

    inline float path_speed(const float remain)
    {
        // Calculate speed slowing down as approaching goal
        return 2.75 * ((remain - 3.0) / (remain + 3.0) + 1.1);
    }
    inline void path(cgrid &grid, const size_t index)
    {
        // Get the drone
        drone &d = _drones[index];

        // Get remaining distance
        const float remain = d.get_data().get_remain();

        // Calculate the speed of the next step
        const min::vec3<float> step = d.get_path().step(grid, d.get_data()) * path_speed(remain);

        // Add velocity to the body
        body(index).set_linear_velocity(step);
    }
    inline void reserve_memory()
    {
        // Reserve space for collision cells
        _col_cells.reserve(27);
        _drones.reserve(_drone_size);
    }

  public:
    drones(physics *sim, static_instance *inst, const min::vec3<float> &dest)
        : _sim(sim), _inst(inst), _dest(dest), _disable(false)
    {
        // Reserve memory for collision cells
        reserve_memory();

        // Add some drones
        for (size_t i = 0; i < _drone_size; i++)
        {
            add(dest);
        }
    }
    inline size_t add(const min::vec3<float> &p)
    {
        // Create a drone
        const size_t inst_id = _inst->add_drone(p);

        // Add to physics simulation
        const min::aabbox<float, min::vec3> box = _inst->box_drone(inst_id);
        const size_t body_id = _sim->add_body(box, 10.0, 1);

        // Add path and path data for drone
        _drones.emplace_back(body_id, inst_id, p, _dest);

        // Return the drone index
        return _drones.size() - 1;
    }
    inline min::body<float, min::vec3> &body(const size_t index)
    {
        return _sim->get_body(_drones[index].body_id());
    }
    inline const min::body<float, min::vec3> &body(const size_t index) const
    {
        return _sim->get_body(_drones[index].body_id());
    }
    inline const min::vec3<float> &position(const size_t index) const
    {
        // Return the character position
        return body(index).get_position();
    }
    inline void set_destination(const min::vec3<float> &p)
    {
        _dest = p;
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
            _drones[i].get_data() = path_data(p, _dest);

            // Update the instance matrix
            const size_t inst_id = _drones[i].inst_id();
            _inst->update_drone_position(inst_id, p);
        }
    }
};
}

#endif
