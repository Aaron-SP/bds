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
#ifndef __LOAD_STATE__
#define __LOAD_STATE__

#include <min/vec3.h>

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

  public:
    load_state(const min::vec3<float> &look, const min::vec3<float> &spawn)
        : _default_look(1.0, -50.0, 0.0), _default_spawn(0.0, -50.0, 0.0),
          _look(look), _spawn(spawn), _loaded(true) {}
    load_state()
        : _default_look(1.0, -50.0, 0.0), _default_spawn(0.0, -50.0, 0.0),
          _look(_default_look), _spawn(_default_spawn), _loaded(false) {}

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
