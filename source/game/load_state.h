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
#ifndef __LOAD_STATE__
#define __LOAD_STATE__

#include <game/file.h>
#include <iostream>
#include <min/vec3.h>
#include <stdexcept>

namespace game
{
class load_state
{
  private:
    min::vec3<float> _default_look;
    min::vec3<float> _default_spawn;
    min::vec3<float> _look;
    min::vec3<float> _spawn;
    bool _loaded;

    inline void check_inside(const float grid_size)
    {
        // Compute the min and max dimension of grid
        const float min = grid_size * -1.0;
        const float max = grid_size;

        // Compute the min and max bounds of grid
        const min::vec3<float> gmin(min, min, min);
        const min::vec3<float> gmax(max, max, max);

        // Test if point is inside the grid
        if (!_spawn.inside(gmin, gmax))
        {
            // Alert changing spawn
            std::cout << "load_state: spawn out of bounds: resetting spawn point" << std::endl;

            // Erase previous state files
            erase_file("bin/state");

            // Set a valid spawn
            _spawn = _default_spawn;
            _look = _default_look;

            // Test if the spawn is valid
            if (!_spawn.inside(gmin, gmax))
            {
                throw std::runtime_error("load_state: failed to load a valid spawn point");
            }
        }
    }

  public:
    load_state(const min::vec3<float> &look, const min::vec3<float> &spawn, const float grid_size)
        : _default_look(1.0, grid_size * -0.75, 0.0), _default_spawn(0.0, grid_size * -0.75, 0.0),
          _look(look), _spawn(spawn), _loaded(true)
    {
        // Check that we loaded a valid point
        check_inside(grid_size);
    }
    load_state(const float grid_size)
        : _default_look(1.0, grid_size * -0.75, 0.0), _default_spawn(0.0, grid_size * -0.75, 0.0),
          _look(_default_look), _spawn(_default_spawn), _loaded(false)
    {
        // Check that we loaded a valid point
        check_inside(grid_size);
    }

    const min::vec3<float> &get_default_look() const
    {
        return _default_look;
    }
    const min::vec3<float> &get_default_spawn() const
    {
        return _default_spawn;
    }
    const min::vec3<float> &get_look() const
    {
        return _look;
    }
    const min::vec3<float> &get_spawn() const
    {
        return _spawn;
    }
    bool is_loaded() const
    {
        return _loaded;
    }
};
}

#endif
