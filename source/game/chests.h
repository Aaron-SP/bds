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
#ifndef __CHESTS__
#define __CHESTS__

#include <game/static_instance.h>
#include <min/aabbox.h>
#include <min/grid.h>
#include <min/physics_nt.h>
#include <min/vec3.h>
#include <vector>

namespace game
{
class chest
{
  private:
    size_t _body_id;
    size_t _inst_id;
    min::vec3<float> _p;

  public:
    chest(const size_t body_id, const size_t inst_id, const min::vec3<float> &p)
        : _body_id(body_id), _inst_id(inst_id), _p(p) {}

    inline size_t body_id() const
    {
        return _body_id;
    }
    inline void dec_inst()
    {
        _inst_id--;
    }
    const min::vec3<float> &get_position() const
    {
        return _p;
    }
    inline size_t inst_id() const
    {
        return _inst_id;
    }
};

class chests
{
  private:
    typedef min::physics<float, uint_fast16_t, uint_fast32_t, min::vec3, min::aabbox, min::aabbox, min::grid> physics;
    physics *const _sim;
    static_instance *const _inst;
    std::vector<chest> _chests;
    const std::string _str;

    inline min::body<float, min::vec3> &body(const size_t index)
    {
        return _sim->get_body(_chests[index].body_id());
    }
    inline const min::body<float, min::vec3> &body(const size_t index) const
    {
        return _sim->get_body(_chests[index].body_id());
    }
    inline void reserve_memory()
    {
        // Reserve space for collision cells
        _chests.reserve(static_instance::max_chests());
    }
    inline void set_position(const size_t index, const min::vec3<float> &g)
    {
        // Get the chest body
        min::body<float, min::vec3> &b = body(index);

        // Apply inverse gravity force to the body
        b.add_force(g * b.get_mass());

        // Set all chest velocities to zero!
        b.set_linear_velocity(min::vec3<float>());

        // Set position to the
        b.set_position(_chests[index].get_position());
    }

  public:
    chests(physics &sim, static_instance &inst)
        : _sim(&sim), _inst(&inst), _str("Chest")
    {
        reserve_memory();
    }
    inline void reset()
    {
        // Remove all the chests backwards to preserve chest-instance id mapping
        const size_t size = _chests.size();
        for (size_t i = size; i-- != 0;)
        {
            // Get the chest
            const chest &c = _chests[i];

            // Clear instance and body
            _inst->get_chest().clear(c.inst_id());
            _sim->clear_body(c.body_id());
        }

        // Clear all the chests
        _chests.clear();
    }
    inline bool add(const min::vec3<float> &p)
    {
        // If all boxes have been allocated, cannibalize
        if (_inst->get_chest().is_full())
        {
            return false;
        }

        // Create a chest instance
        const size_t inst_id = _inst->get_chest().add(p);

        // Create a box for the chest
        const min::aabbox<float, min::vec3> box = _inst->get_chest().get_box(inst_id);

        // Store the chest index as body data
        const size_t index = _chests.size();

        // Add to physics simulation
        const size_t body_id = _sim->add_body(box, 10.0, id_value(static_id::CHEST), index);

        // Create a new chest
        _chests.emplace_back(body_id, inst_id, p);

        // Return chest added
        return true;
    }
    inline const std::string &get_string() const
    {
        return _str;
    }
    inline void remove(const size_t index)
    {
        // Clear chest at index
        _inst->get_chest().clear(_chests[index].inst_id());
        _sim->clear_body(_chests[index].body_id());
        _chests.erase(_chests.begin() + index);

        // Adjust the remaining chest indices
        const size_t size = _chests.size();
        for (size_t i = index; i < size; i++)
        {
            // Adjust the instance id
            _chests[i].dec_inst();

            // Adjust the body data index
            body(i).set_data(min::body_data(i));
        }
    }
    inline void update_frame()
    {
        // Get gravity acceleration
        const min::vec3<float> inv_g(0.0, _grav_mag, 0.0);

        // Keep chest from moving in simulation
        const size_t size = _chests.size();
        for (size_t i = 0; i < size; i++)
        {
            // Apply normal force at surface
            set_position(i, inv_g);
        }
    }
    inline void update()
    {
        // Update all chest positions
        const size_t size = _chests.size();
        for (size_t i = 0; i < size; i++)
        {
            // Update the instance matrix
            const size_t inst_id = _chests[i].inst_id();

            // Update body positions
            const min::vec3<float> &p = body(i).get_position();
            _inst->get_chest().update_position(inst_id, p);
        }
    }
};
}

#endif
