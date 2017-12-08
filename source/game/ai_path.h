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
#ifndef __AI_PATH__
#define __AI_PATH__

#include <algorithm>
#include <game/cgrid.h>
#include <game/path.h>
#include <min/intersect.h>
#include <min/vec3.h>

namespace game
{

class ai_path
{
  private:
    static constexpr float _step_size = 1.0;
    path _path;
    min::vec3<float> _step;

  public:
    ai_path() {}
    inline min::vec3<float> &calculate(const cgrid &grid, const path_data &data)
    {
        // Get path properties
        const min::vec3<float> step = _path.step(grid, data);

        // Calculate step direction and length
        _step = step * _step_size;

        // return step
        return _step;
    }
    inline const path &get_path() const
    {
        return _path;
    }
    inline const min::vec3<float> &step() const
    {
        return _step;
    }
};
}

#endif
