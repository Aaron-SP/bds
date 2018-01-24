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
#ifndef __PATH__
#define __PATH__

#include <algorithm>
#include <game/cgrid.h>
#include <min/cubic.h>
#include <min/vec3.h>
#include <numeric>

namespace game
{

class path_data
{
  private:
    min::vec3<float> _destination;
    min::vec3<float> _direction;
    min::vec3<float> _position;
    float _remain;

    inline void update_direction()
    {
        // Update direction vector
        _direction = _destination - _position;

        // Normalize direction if needed
        _remain = _direction.magnitude();
        if (_remain > 1E-3)
        {
            const float inv_mag = 1.0 / _remain;
            _direction *= inv_mag;
        }
    }

  public:
    path_data(const min::vec3<float> &p, const min::vec3<float> &d)
        : _destination(d), _position(p), _remain(0.0)
    {
        // Update direction
        update_direction();
    }
    inline const min::vec3<float> &get_destination() const
    {
        return _destination;
    }
    inline const min::vec3<float> &get_direction() const
    {
        return _direction;
    }
    inline const min::vec3<float> &get_position() const
    {
        return _position;
    }
    inline float get_remain() const
    {
        return _remain;
    }
    inline min::vec3<float> step(const min::vec3<float> &dir, const float step_size)
    {
        // Calculate step
        const min::vec3<float> step = dir * step_size;

        // return new position
        return _position + step;
    }
    inline void update(const min::vec3<float> &p)
    {
        // Update the position
        _position = p;

        // Update the direction
        update_direction();
    }
    inline void update_destination(const min::vec3<float> &p)
    {
        // Update destination
        _destination = p;

        // Update the direction
        update_direction();
    }
};

class path
{
  private:
    static constexpr float _path_square_expire = 25.0;
    static constexpr float _path_square_arrive = 0.25;
    std::vector<min::vec3<float>> _path;
    min::bezier_deriv<float, min::vec3> _curve;
    min::vec3<float> _target;
    min::vec3<float> _last;
    bool _bezier_interp;
    float _curve_dist;
    float _curve_interp;
    size_t _path_index;

    inline void accumulate(const min::vec3<float> &p)
    {
        // Calculate the distance from the start point
        const min::vec3<float> accum_vec = p - _last;
        const float accum_dist = accum_vec.dot(accum_vec);

        // Increment the interpolation distance constant
        _curve_interp += std::sqrt(accum_dist / _curve_dist);

        // Reset last point
        _last = p;
    }

    inline min::vec3<float> calculate_direction(const path_data &data) const
    {
        // Current position
        const min::vec3<float> &p = data.get_position();

        // Bezier interpolation instead of linear?
        min::vec3<float> target;
        if (_bezier_interp)
        {
            // Add bezier interpolation here
            target = _curve.interpolate(_curve_interp);
        }
        else
        {
            target = (_path[_path_index] - p);
        }

        // Normalize direction vector and provide a fallback
        return target.normalize_safe(data.get_direction());
    }
    inline void expire_path(const path_data &data)
    {
        // Current position
        const min::vec3<float> &p = data.get_position();

        // Calculate the expire distance
        const min::vec3<float> expire_vec = _target - p;
        const float expire = expire_vec.dot(expire_vec);

        // Reset path?
        if (expire < _path_square_expire)
        {
            // Travel down the path
            if (expire < _path_square_arrive)
            {
                // Increment current position in path
                if (_bezier_interp)
                {
                    // We test for overflow be enabling bezier interpolation
                    _path_index += 3;
                }
                else
                {
                    _path_index++;
                }

                // Check if we have arrived at the destination
                const size_t size = _path.size();
                if (_path_index == size)
                {
                    // If we have reached the destination
                    _path.clear();
                }
                else if ((size - _path_index) >= 3)
                {
                    // Use bezier interpolation
                    set_bezier_interpolation(p);
                }
                else
                {
                    // Use bezier interpolation
                    set_linear_interpolation();
                }
            }
        }
        else
        {
            // Generate a new path
            _path.clear();
        }
    }
    inline void set_bezier_interpolation(const min::vec3<float> &begin)
    {
        // Flag we are using bezier curve
        _bezier_interp = true;

        // Calculate bezier curve indices
        const size_t i1 = _path_index;
        const size_t i2 = i1 + 1;
        const size_t i3 = i2 + 1;

        // Calculate new bezier curve
        _curve = min::bezier_deriv<float, min::vec3>(begin, _path[i1], _path[i2], _path[i3]);

        // Calculate distance between start and end point on curve
        const min::vec3<float> curve_vec = _curve.end() - _curve.begin();
        _curve_dist = curve_vec.dot(curve_vec);

        // Reset the curve interpolation constant
        _curve_interp = 0.0;

        // Reset the target position
        _target = _curve.end();
    }
    inline void set_linear_interpolation()
    {
        // Flag we are not using bezier curve
        _bezier_interp = false;

        // Reset the target position
        _target = _path[_path_index];
    }

  public:
    path() : _bezier_interp(false), _curve_dist(0.0), _curve_interp(0.0), _path_index(0)
    {
        // Reserve space for path
        _path.reserve(100);
    }
    void clear()
    {
        _path.clear();
    }
    const min::vec3<float> step(cgrid &grid, const path_data &data)
    {
        // Get data points
        const min::vec3<float> &p = data.get_position();
        const min::vec3<float> &dest = data.get_destination();

        // If we need to compute a path
        if (_path.size() == 0)
        {
            // Update path vector
            grid.path(_path, p, dest);

            // Reset path index
            _path_index = 0;

            // Reset last point
            _last = p;

            // Reset the bezier curve if have enough points
            if (_path.size() >= 3)
            {
                set_bezier_interpolation(p);
            }
            else
            {
                set_linear_interpolation();
            }
        }

        // If we got a path use it
        if (_path.size() > 0)
        {
            // Accumulate interpolation
            accumulate(p);

            // Calculate direction
            const min::vec3<float> out = calculate_direction(data);

            // Check if path has expired
            expire_path(data);

            return out;
        }

        // Failure fallback
        return data.get_direction();
    }
    const std::vector<min::vec3<float>> &get_path() const
    {
        return _path;
    }
};
}

#endif
