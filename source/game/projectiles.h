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
#ifndef __PROJECTILE__
#define __PROJECTILE__

#include <functional>
#include <game/particle.h>
#include <game/sound.h>
#include <game/static_instance.h>
#include <vector>

namespace game
{

class missile
{
  private:
    min::ray<float, min::vec3> _ray;
    min::sample<float, min::vec3> _traj;
    size_t _inst_id;
    size_t _part_id;
    size_t _sound_id;
    size_t _key;
    int8_t _value;
    bool _launch;

  public:
    missile(const min::ray<float, min::vec3> &r, const min::vec3<float> &point, const float weight,
            const size_t inst, const size_t part, const size_t sound,
            const size_t key, const int8_t value)
        : _ray(r), _traj(r.get_origin(), point, weight),
          _inst_id(inst), _part_id(part), _sound_id(sound),
          _key(key), _value(value), _launch(true) {}

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
    inline const min::vec3<float> &dest() const
    {
        return _traj.get_dest();
    }
    inline bool done() const
    {
        return _traj.done();
    }
    inline bool hit() const
    {
        return _value >= 0;
    }
    inline min::vec3<float> interpolate(const float dt)
    {
        return _traj.weight_interpolate(dt);
    }
    inline size_t key() const
    {
        return _key;
    }
    inline void kill()
    {
        _launch = false;
    }
    inline void launch()
    {
        _launch = true;
    }
    inline bool launched() const
    {
        return _launch;
    }
    inline const min::ray<float, min::vec3> &ray() const
    {
        return _ray;
    }
    inline int8_t value() const
    {
        return _value;
    }
};

class projectiles
{
  private:
    static constexpr size_t _miss_size = 1;
    static constexpr size_t _miss_max_dist = 100;

    static_instance *_inst;
    particle *_part;
    sound *_sound;
    std::vector<missile> _miss;
    const min::vec3<unsigned> _scale;

    static inline min::vec3<float> center_radius(const min::vec3<float> &p, const min::vec3<unsigned> &scale)
    {
        const min::vec3<float> offset(scale.x() / 2, scale.y() / 2, scale.z() / 2);
        const min::vec3<float> center = p - offset;

        // return center position
        return center;
    }
    inline bool launch(const cgrid &grid, const min::ray<float, min::vec3> &r)
    {
        // Get instance id
        const size_t inst_id = _inst->add_missile(r.get_origin());

        // Get particle id
        const size_t part_id = _part->get_idle_miss_launch_id();

        // Get sound id
        const size_t sound_id = _sound->get_idle_miss_launch_id();

        // Trace a ray to the destination point to find placement position, return point is snapped
        int8_t value;
        size_t key;
        min::vec3<float> traced;
        const bool valid = grid.ray_trace_last_key(r, _miss_max_dist, traced, key, value);
        if (!valid)
        {
            return false;
        }

        // Calculate trajectory distance, set missile trajectory
        const float weight = 1.0 / (traced - r.get_origin()).magnitude();

        // Create new missile
        _miss.push_back(missile(r, traced, weight, inst_id, part_id, sound_id, key, value));

        // Create quaternion from Y axis to facing direction
        const min::quat<float> q(min::vec3<float>::up(), r.get_direction());

        // Update the missile rotation
        _inst->update_missile_rotation(inst_id, q);

        // Set the launch particle attributes
        set_part(part_id, _miss.back());

        // Play the launch sound
        _sound->play_miss_launch(sound_id, r.get_origin());

        // Return that we launched a missile
        return true;
    }
    inline bool relaunch(const cgrid &grid, missile &m)
    {
        // Create a new ray along old ray direction
        const min::vec3<float> &origin = m.dest();
        min::vec3<float> to = origin + m.ray().get_direction();
        min::ray<float, min::vec3> r(origin, to);

        // Get instance id
        const size_t inst_id = m.inst_id();

        // Get particle id
        const size_t part_id = m.part_id();

        // Get sound id
        const size_t sound_id = m.sound_id();

        // Trace a ray to the destination point to find placement position, return point is snapped
        int8_t value;
        size_t key;
        min::vec3<float> traced;
        const bool valid = grid.ray_trace_last_key(r, _miss_max_dist, traced, key, value);
        if (!valid)
        {
            return false;
        }

        // Calculate trajectory distance, set missile trajectory
        const float weight = 1.0 / (traced - r.get_origin()).magnitude();

        // Create missile at index
        m = missile(r, traced, weight, inst_id, part_id, sound_id, key, value);

        // Create quaternion from Y axis to facing direction
        const min::quat<float> q(min::vec3<float>::up(), r.get_direction());

        // Update the missile rotation
        _inst->update_missile_rotation(inst_id, q);

        // Return that we launched a missile
        return true;
    }
    inline void remove(const size_t index)
    {
        // Clear missiles at index
        _inst->clear_missile(_miss[index].inst_id());
        _miss.erase(_miss.begin() + index);

        // Adjust the remaining missile indices
        const size_t size = _miss.size();
        for (size_t i = index; i < size; i++)
        {
            _miss[i].dec_inst();
        }
    }
    inline void set_part(const size_t part_id, const missile &m)
    {
        // Set particle settings
        const min::ray<float, min::vec3> &ray = m.ray();
        const min::vec3<float> &p = ray.get_origin();
        const min::vec3<float> dir = ray.get_direction() * -10.0;

        // Load particles at id for 86400 seconds = 1 day :)
        _part->load_miss_launch(part_id, p, dir, 86400.0, 40.0);
    }

  public:
    projectiles(particle *const particles, static_instance *const inst, sound *const s)
        : _inst(inst), _part(particles), _sound(s), _scale(3, 3, 3) {}

    inline bool launch_missile(const cgrid &grid, const min::ray<float, min::vec3> &r)
    {
        // Are all missiles being used?
        if (_inst->missile_full())
        {
            return false;
        }

        return launch(grid, r);
    }
    inline void update(const cgrid &grid, const float speed,
                       const std::function<void(
                           const min::vec3<float> &point,
                           const min::vec3<float> &direction,
                           const min::vec3<unsigned> &scale,
                           const size_t value)> &f = nullptr)
    {
        // Update each missile
        size_t size = _miss.size();
        for (size_t i = 0; i < size; i++)
        {
            // Get the missile
            missile &m = _miss[i];

            // If missile is launched
            if (m.launched())
            {
                // Get the atlas of target block, if hit a block remove it
                if (m.done())
                {
                    // Test for race condition on grid cell
                    const bool hit = m.hit();
                    if (hit)
                    {
                        // Test if we need to relaunch missile
                        if (grid.ray_trace_key(m.key()) < 0)
                        {
                            // Relaunch the missile
                            const bool success = relaunch(grid, m);
                            if (success)
                            {
                                // Continue processing other missiles
                                continue;
                            }
                        }
                    }

                    // Missile hit target
                    m.kill();

                    // Stop playing particles
                    _part->abort_miss_launch(m.part_id());

                    // Stop playing launch sound
                    _sound->stop_miss_launch(m.sound_id());

                    // If we hit a block remove it
                    if (hit)
                    {
                        // Invoke the function callback if provided
                        if (f)
                        {
                            // Get direction for particle spray
                            const min::vec3<float> dir = m.ray().get_direction() * -1.0;

                            // Center the explosion point of missile explosion
                            const min::vec3<float> center = center_radius(m.dest(), _scale);

                            // Call function callback
                            f(center, dir, _scale, m.value());
                        }
                    }

                    // Clear this missiles
                    remove(i);

                    // Adjust the size and current index
                    size--;
                    i--;
                }
                else
                {
                    // Interpolate missile location
                    const min::vec3<float> point = m.interpolate(speed);

                    // Update missile position
                    _inst->update_missile_position(m.inst_id(), point);

                    // Set particle position slightly behind the rocket
                    const min::vec3<float> offset = point - m.ray().get_direction() * 0.25;
                    _part->set_miss_launch_position(m.part_id(), offset);

                    // Update the launch sound position
                    _sound->update_miss_launch(m.sound_id(), point);
                }
            }
        }
    }
};
}

#endif
