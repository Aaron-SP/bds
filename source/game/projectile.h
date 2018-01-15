/* Copyright [2013-2018] [Aaron Springstroh, Minimal Graphics Library]

This file is part of the Fractex.

Fractex is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Fractex is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Fractex.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __PROJECTILE__
#define __PROJECTILE__

#include <functional>
#include <game/particle.h>
#include <game/sound.h>
#include <game/static_instance.h>

namespace game
{

class projectile
{
  private:
    static_instance *_instance;
    particle *_particles;
    sound *_sound;
    min::ray<float, min::vec3> _ray;
    min::vec3<unsigned> _scale;
    min::sample<float, min::vec3> _traj;
    int8_t _value;
    size_t _id;
    bool _launch;
    bool _remove;

    static inline min::vec3<float> center_radius(const min::vec3<float> &p, const min::vec3<unsigned> &scale)
    {
        const min::vec3<float> offset(scale.x() / 2, scale.y() / 2, scale.z() / 2);
        const min::vec3<float> center = p - offset;

        // return center position
        return center;
    }

  public:
    projectile(particle *const particles, static_instance *const instance, sound *const s)
        : _instance(instance), _particles(particles), _sound(s), _scale(3, 3, 3),
          _launch(false), _remove(false) {}

    inline void set_launch_particles()
    {
        // Add particle effects along ray
        const min::vec3<float> dir = _ray.get_direction() * -10.0;
        _particles->load_emit_launch(_ray.get_origin(), dir, 86400.0, 20.0);
    }

    inline void launch_missile(const min::ray<float, min::vec3> &r, const cgrid &grid, const size_t max_length)
    {
        // Cache ray for later use
        _ray = r;

        // Trace a ray to the destination point to find placement position, return point is snapped
        _value = -2;
        const min::vec3<float> traced = grid.ray_trace_last(r, max_length, _value);

        // Record if we hit a block to remove
        _remove = _value >= 0;

        // Enabled missile launching
        _launch = true;

        // Calculate trajectory distance, set missile trajectory
        const float inv_d = 1.0 / (traced - r.get_origin()).magnitude();
        _traj = min::sample<float, min::vec3>(r.get_origin(), traced, inv_d);

        // Create quaternion from Y axis to facing direction
        const min::quat<float> q(min::vec3<float>::up(), r.get_direction());

        // Add a missile
        _id = _instance->add_missile(r.get_origin());

        // Update the missile rotation
        _instance->update_missile_rotation(q, _id);

        // Set the launch particle attributes
        set_launch_particles();

        // Play the launch sound
        _sound->play_miss_launch(r.get_origin());
    }
    inline void draw(game::uniforms &uniforms) const
    {
        // Draw launch particles
        _particles->draw_emit_launch(uniforms);
    }
    inline void update(const float speed,
                       const std::function<void(
                           const min::vec3<float> &point,
                           const min::vec3<float> &direction,
                           const min::vec3<unsigned> &scale,
                           const size_t value)> &f = nullptr)
    {
        if (_launch)
        {
            // Interpolate missile location
            const min::vec3<float> point = _traj.weight_interpolate(speed);

            // Get the atlas of target block, if hit a block remove it
            if (_traj.done())
            {
                // Missile hit target
                _launch = false;

                // Clear all missiles
                _instance->clear_missile();

                // Stop playing particles
                _particles->abort_launch();

                // Stop playing launch sound
                _sound->stop_miss_launch();

                // If we hit a block remove it
                if (_remove)
                {
                    // Invoke the function callback if provided
                    if (f)
                    {
                        // Get direction for particle spray
                        const min::vec3<float> dir = _ray.get_direction() * -1.0;

                        // Center the explosion point of missile explosion
                        const min::vec3<float> center = center_radius(_traj.get_dest(), _scale);

                        // Call function callback
                        f(center, dir, _scale, _value);
                    }
                }
            }
            else
            {
                // Update missile position
                _instance->update_missile_position(point, _id);

                // Set particle position slightly behind the rocket
                const min::vec3<float> offset = point - _ray.get_direction() * 0.25;
                _particles->set_launch_position(offset);

                // Update the launch sound position
                _sound->update_miss_launch(point);
            }
        }
    }
};
}

#endif
