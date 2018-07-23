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
#ifndef _BDS_PATH_BDS_
#define _BDS_PATH_BDS_

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
    min::vec3<float> _p;
    min::vec3<float> _dest;
    min::vec3<float> _dir;
    float _remain;

    inline void update_direction()
    {
        // Update direction vector
        _dir = _dest - _p;

        // Normalize direction if needed
        _remain = _dir.magnitude();
        if (_remain > 1E-3)
        {
            const float inv_mag = 1.0 / _remain;
            _dir *= inv_mag;
        }
    }

  public:
    path_data() {}
    path_data(const min::vec3<float> &p, const min::vec3<float> &dest)
        : _p(p), _dest(dest), _remain(0.0)
    {
        // Update direction
        update_direction();
    }
    inline const min::vec3<float> &destination() const
    {
        return _dest;
    }
    inline const min::vec3<float> &direction() const
    {
        return _dir;
    }
    inline const min::vec3<float> &position() const
    {
        return _p;
    }
    inline float get_remain() const
    {
        return _remain;
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
    path_data _data;
    bool _bezier_interp;
    float _curve_dist;
    float _curve_interp;
    size_t _path_index;
    bool _is_dead;
    bool _is_stuck;

    inline min::vec3<float> calculate_direction() const
    {
        // Current position
        const min::vec3<float> &p = _data.position();

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
        return target.normalize_safe(_data.direction());
    }
    inline void expire_path()
    {
        // Current position
        const min::vec3<float> &p = _data.position();

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
    path()
        : _bezier_interp(false),
          _curve_dist(0.0), _curve_interp(0.0),
          _path_index(0),
          _is_dead(true), _is_stuck(false)
    {
        // Reserve space for path
        _path.reserve(100);
    }
    inline void clear()
    {
        _path.clear();
    }
    inline void clear_stuck()
    {
        _is_stuck = false;
    }
    inline float get_remain() const
    {
        return _data.get_remain();
    }
    inline bool is_dead() const
    {
        return _is_dead;
    }
    inline bool is_stuck() const
    {
        return _is_stuck;
    }
    inline void set_dead(const bool flag)
    {
        _is_dead = flag;
    }
    inline const min::vec3<float> step(cgrid &grid)
    {
        // Get data points
        const min::vec3<float> &p = _data.position();
        const min::vec3<float> &dest = _data.destination();

        // If we need to compute a path
        if (_path.size() == 0)
        {
            // Update path vector
            grid.path(_path, p, dest);

            // If we got a path from grid
            if (_path.size() > 0)
            {
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

                // Calculate direction
                return calculate_direction();
            }
            else
            {
                // Flag that we are stuck
                _is_stuck = true;
            }
        }
        else
        {
            // Calculate the distance from the last point
            const min::vec3<float> accum_vec = p - _last;
            const float accum_dist = accum_vec.dot(accum_vec);

            // Increment the interpolation distance constant
            _curve_interp += std::sqrt(accum_dist / _curve_dist);

            // Reset last point
            _last = p;

            // Calculate direction
            const min::vec3<float> out = calculate_direction();

            // Check if path has expired
            expire_path();

            return out;
        }

        // Failure fallback
        return _data.direction();
    }
    inline void update(const min::vec3<float> &p, const min::vec3<float> &dest)
    {
        // Assign new data
        _data = path_data(p, dest);
    }
};
}

#endif
