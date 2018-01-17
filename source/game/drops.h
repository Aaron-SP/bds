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
#ifndef __DROPS__
#define __DROPS__

#include <min/aabbox.h>
#include <min/grid.h>
#include <min/physics.h>
#include <min/vec3.h>

namespace game
{
class drops
{
  private:
    min::physics<float, uint16_t, uint32_t, min::vec3, min::aabbox, min::aabbox, min::grid> *_simulation;

  public:
    drops(min::physics<float, uint16_t, uint32_t, min::vec3, min::aabbox, min::aabbox, min::grid> *sim)
        : _simulation(sim) {}
};
}

#endif
