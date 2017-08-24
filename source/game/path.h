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
    float _angle;
    float _remain;
    float _travel;
    float _travel_step;

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
        : _destination(d), _position(s),
          _angle(0.0), _remain(0.0), _travel(0.0), _travel_step(0.0)
    {
        // Update direction
        update_direction();
    }
    inline float get_angle_step() const
    {
        return _travel_step * _angle;
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
    inline float get_travel_step() const
    {
        return _travel_step;
    }
    inline float get_travel() const
    {
        return _travel;
    }
    inline min::vec3<float> step(const min::vec3<float> &dir, const float step_size)
    {
        // Calculate step
        const min::vec3<float> step = dir * step_size;

        // Calculate the travel step
        _travel_step = step.magnitude();

        // Calculate angle with travel direction
        // Normalize direction since step is NOT NORMALIZED
        if (_travel_step > 1E-3)
        {
            const min::vec3<float> n = step * (1.0 / _travel_step);
            _angle = _direction.dot(n);
        }
        else
        {
            _angle = 0;
        }

        // return new position
        return _position + step;
    }
    inline void update(const min::vec3<float> &p)
    {
        // Update distance travelled
        _travel += _travel_step;

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
    static constexpr size_t eye_size = 27;
    static constexpr float path_expire = 5.0;
    std::vector<min::vec3<float>> _eye;
    min::vec3<float> _eye_dir[eye_size];
    float _eye_mag[eye_size];
    float _eye_dot[eye_size];
    size_t _eye_dot_index[eye_size];
    std::vector<min::vec3<float>> _path;
    size_t _path_index;
    min::vec3<float> _avoid;

    void calculate_eye_rays(const cgrid &grid, const path_data &data)
    {
        const min::vec3<float> &position = data.get_position();

        // Must be eye_size in size
        grid.get_cubic_rays(_eye, position);
        if (_eye.size() != eye_size)
        {
            throw std::runtime_error("ai_path: eyes incorrect size");
        }
    }

  public:
    path() : _eye_dir{}, _eye_mag{}, _eye_dot{}, _eye_dot_index{}, _path_index(0)
    {
        // Reserve space for eye vector
        _eye.reserve(eye_size);

        // Reserve space for path
        _path.reserve(20);
    }
    min::vec3<float> avoid() const
    {
        return _avoid;
    }
    const float *const get_eye_mag() const
    {
        return _eye_mag;
    }
    min::vec3<float> dfs(const cgrid &grid, const path_data &data)
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
            if (mag <= 1.0)
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
                // Return the direction to next point
                const float inv_mag = 1.0 / mag;
                return dp * inv_mag;
            }
            else
            {
                return dp;
            }
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
    min::vec3<float> ray_index(const size_t index) const
    {
        // return the best ray
        return _eye_dir[index];
    }
    min::vec3<float> ray_sorted(const size_t index) const
    {
        // return the best ray
        return _eye_dir[_eye_dot_index[index]];
    }
    void update(const cgrid &grid, const path_data &data)
    {
        // Get data points
        const min::vec3<float> &p = data.get_position();
        const min::vec3<float> &dir = data.get_direction();
        const min::vec3<float> &dest = data.get_destination();

        // Calculate a new set of eyes
        calculate_eye_rays(grid, data);

        // Reset avoidance vector
        _avoid = min::vec3<float>();

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

        // Update eye sensor properties
        const min::vec3<float> dv = dest - p;
        for (size_t i = 0; i < eye_size; i++)
        {
            // Calculate ray vector
            _eye_dir[i] = _eye[i] - p;

            // Calculate avoidance
            _avoid += _eye_dir[i];

            // Calculate magnitude along ray
            _eye_mag[i] = _eye_dir[i].magnitude();

            // Calculate normalize direction
            if (_eye_mag[i] > 1E-3)
            {
                const float inv_mag = 1.0 / _eye_mag[i];
                _eye_dir[i] *= inv_mag;
            }

            // Dot product along direction, prevent going through a wall
            _eye_dot[i] = std::min(_eye_dir[i].dot(dv), _eye_mag[i]);
        }

        // Normalize the avoid vector
        _avoid.normalize_safe(dir);

        // Sort the index array by largest dot product
        std::iota(_eye_dot_index, _eye_dot_index + eye_size, 0);
        std::sort(_eye_dot_index, _eye_dot_index + eye_size, [this](const size_t a, const size_t b) {
            return this->_eye_dot[a] > this->_eye_dot[b];
        });
    }
};
}

#endif
