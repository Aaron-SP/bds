/* Copyright [2013-2016] [Aaron Springstroh, Minimal Graphics Library]

This file is part of the MGLCraft.

MGLCraft is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MGLCraft is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MGLCraft.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __PATH__
#define __PATH__

#include <algorithm>
#include <game/cgrid.h>
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
    path_data(const min::vec3<float> &s, const min::vec3<float> &d)
        : _destination(d), _position(s), _remain(0.0)
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
    static constexpr float path_expire = 5.0;
    std::vector<min::vec3<float>> _path;
    size_t _path_index;

  public:
    path() : _path_index(0)
    {
        // Reserve space for path
        _path.reserve(20);
    }
    const min::vec3<float> step(const cgrid &grid, const path_data &data)
    {
        // Get data points
        const min::vec3<float> &p = data.get_position();
        const min::vec3<float> &dest = data.get_destination();

        // If we already computed a path
        if (_path.size() > 0)
        {
            // Get the current point on this path
            const min::vec3<float> point = _path[_path_index];

            // Check if we have arrived at this point
            const min::vec3<float> dp = point - p;
            const float mag = dp.magnitude();
            if (mag <= 0.5)
            {
                // Increment the index
                _path_index++;

                // If we have reached the destination
                if (_path_index == _path.size())
                {
                    _path.clear();
                }
            }

            // Normalize direction?
            if (mag > 1E-3)
            {
                // Normalize dfs direction and cache it
                const float inv_mag = 1.0 / mag;
                return dp * inv_mag;
            }

            // Return the direction to next point
            return dp;
        }

        // Get a path between the two points
        grid.path(_path, p, dest);

        // If we found a path
        if (_path.size() > 0)
        {
            // Reset the path index, in reverse
            _path_index = 0;

            // Return the direction to next point, defaulting to data direction
            min::vec3<float> dp = _path[_path_index] - p;
            return dp.normalize_safe(data.get_direction());
        }

        // Couldn't find a path so return default
        return data.get_direction();
    }
    const std::vector<min::vec3<float>> &get_path() const
    {
        return _path;
    }
    void update(const cgrid &grid, const path_data &data)
    {
        // Get data points
        const min::vec3<float> &p = data.get_position();

        // If we have a path, test if it expired
        if (_path.size() > 0)
        {
            const min::vec3<float> &point = _path[_path_index];
            const float mag = (point - p).magnitude();

            // If nearest point in path is 10 units away, it expired
            if (mag > path_expire)
            {
                // Empty the current path
                _path.clear();
            }
        }
    }
};
}

#endif
