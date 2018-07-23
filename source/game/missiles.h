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
#ifndef _BDS_MISSILES_BDS_
#define _BDS_MISSILES_BDS_

#include <game/def.h>
#include <game/id.h>
#include <game/particle.h>
#include <game/sound.h>
#include <game/static_instance.h>
#include <min/grid.h>
#include <min/physics_nt.h>
#include <min/tri.h>
#include <min/vec3.h>
#include <vector>

namespace game
{

class missile
{
  private:
    size_t _body_id;
    size_t _inst_id;
    size_t _part_id;
    size_t _sound_id;

  public:
    missile(const size_t body_id, const size_t inst_id, const size_t part_id, const size_t sound_id)
        : _body_id(body_id), _inst_id(inst_id), _part_id(part_id), _sound_id(sound_id) {}

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
    inline size_t part_id() const
    {
        return _part_id;
    }
    inline size_t sound_id() const
    {
        return _sound_id;
    }
};

class missiles
{
  private:
    physics *const _sim;
    static_instance *const _inst;
    particle *const _part;
    sound *const _sound;
    std::vector<std::pair<min::aabbox<float, min::vec3>, block_id>> _col_cells;
    std::vector<missile> _miss;
    const min::tri<unsigned> _scale;
    coll_call _f;
    const std::string _str;

    inline min::body<float, min::vec3> &body(const size_t index)
    {
        return _sim->get_body(_miss[index].body_id());
    }
    inline const min::body<float, min::vec3> &body(const size_t index) const
    {
        return _sim->get_body(_miss[index].body_id());
    }
    template <typename ES>
    inline void explode(const size_t index, const block_id atlas, const ES &ex_scale_call)
    {
        // Call the explosion callback function if available
        ex_scale_call(position(index), _scale, atlas);

        // Blow up the missile
        explode(index);
    }
    inline const min::vec3<float> &velocity(const size_t index) const
    {
        // Return the explosive position
        return body(index).get_linear_velocity();
    }
    inline void remove(const size_t index)
    {
        // Clear missiles at index
        _inst->get_missile().clear(_miss[index].inst_id());
        _sim->clear_body(_miss[index].body_id());
        _miss.erase(_miss.begin() + index);

        // Adjust the remaining missile indices
        const size_t size = _miss.size();
        for (size_t i = index; i < size; i++)
        {
            // Adjust the instance id
            _miss[i].dec_inst();

            // Adjust the body data index
            body(i).set_data(min::body_data(i));
        }
    }
    inline void reserve_memory()
    {
        // Reserve space for collision cells
        _col_cells.reserve(27);
        _miss.reserve(static_instance::max_missiles());
    }

  public:
    missiles(physics &sim, particle &part, static_instance &inst, sound &s)
        : _sim(&sim), _inst(&inst),
          _part(&part), _sound(&s),
          _scale(3, 7, 3), _f(nullptr), _str("Missile")
    {
        reserve_memory();
    }
    inline void reset()
    {
        // Remove all the missiles backwards to preserve ex-instance id mapping
        const size_t size = _miss.size();
        for (size_t i = size; i-- != 0;)
        {
            // Get the missile
            const missile &m = _miss[i];

            // Clear instance and body
            _inst->get_explosive().clear(m.inst_id());
            _sim->clear_body(m.body_id());
        }

        // Clear all the drops
        _miss.clear();
    }
    inline void explode(const size_t index)
    {
        // Stop playing particles
        _part->abort_miss_launch(_miss[index].part_id());

        // Stop playing launch sound
        _sound->stop_miss_launch(_miss[index].sound_id());

        // Blow up the missile
        remove(index);
    }
    inline const min::tri<unsigned> &get_scale() const
    {
        return _scale;
    }
    inline const std::string &get_string() const
    {
        return _str;
    }
    inline bool launch_missile(const min::vec3<float> &p, const min::vec3<float> &dir, const min::vec3<float> &vel)
    {
        // Are all missiles being used?
        if (_inst->get_missile().is_full())
        {
            return false;
        }

        // Get instance id
        const size_t inst_id = _inst->get_missile().add(p);

        // Get particle id
        const size_t part_id = _part->get_idle_miss_launch_id();

        // Get sound id
        const size_t sound_id = _sound->get_idle_miss_launch_id();

        // Create quaternion from Y axis to facing direction
        const min::quat<float> q(min::vec3<float>::up(), dir);

        // Update the missile rotation
        _inst->get_missile().update_rotation(inst_id, q);

        // Set the launch particle attributes
        _part->load_miss_launch(part_id, p, dir, 86400.0, 40.0);

        // Play the launch sound
        _sound->play_miss_launch(sound_id, p);

        // Create a box for the missile
        const min::aabbox<float, min::vec3> box = _inst->get_missile().get_box(inst_id);

        // Store the missile index as body data
        const size_t index = _miss.size();

        // Add to physics simulation
        const size_t body_id = _sim->add_body(box, 10.0, id_value(static_id::MISSILE), index);

        // Register player collision callback
        _sim->register_callback(body_id, _f);

        // Get the physics body for editing
        min::body<float, min::vec3> &body = _sim->get_body(body_id);

        // Set body linear velocity
        body.set_linear_velocity(vel + (dir * 30.0));

        // Create a new missile
        _miss.emplace_back(body_id, inst_id, part_id, sound_id);

        // Return launch success
        return true;
    }
    inline const min::vec3<float> &position(const size_t index) const
    {
        // Return the explosive position
        return body(index).get_position();
    }
    inline void set_collision_callback(const coll_call &f)
    {
        _f = f;
    }
    template <typename ES>
    inline void update_frame(const cgrid &grid, const ES &ex_scale_call)
    {
        // Do missile collisions
        const size_t size = _miss.size();
        for (size_t i = 0; i < size; i++)
        {
            // Get all cells that could collide
            grid.missile_collision_cells(_col_cells, position(i));

            // Solve static collisions
            const size_t body = _miss[i].body_id();
            for (const auto &cell : _col_cells)
            {
                if (_sim->collide(body, cell.first))
                {
                    // Explode the missile
                    explode(i, cell.second, ex_scale_call);

                    // Decrement current index
                    i--;

                    // Abort inner loop
                    break;
                }
            }
        }
    }
    inline void update(const cgrid &grid)
    {
        // Update all missile positions
        const size_t size = _miss.size();
        for (size_t i = 0; i < size; i++)
        {
            // Update the instance matrix
            const size_t inst_id = _miss[i].inst_id();
            const size_t part_id = _miss[i].part_id();
            const size_t sound_id = _miss[i].sound_id();

            // Update missile positions
            const min::vec3<float> &p = body(i).get_position();
            _inst->get_missile().update_position(inst_id, p);

            // Set particle position slightly behind the rocket opposing body velocity
            const min::vec3<float> dir = min::vec3<float>(velocity(i)).normalize();
            const min::vec3<float> offset = p - dir * 0.25;
            _part->set_miss_launch_position(part_id, offset);

            // Update the launch sound position
            _sound->update_miss_launch(sound_id, p);
        }
    }
};
}

#endif
