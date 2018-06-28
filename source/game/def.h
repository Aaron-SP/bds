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
#ifndef __DEFINITIONS__
#define __DEFINITIONS__

#include <functional>
#include <game/id.h>
#include <min/aabbox.h>
#include <min/grid.h>
#include <min/physics_nt.h>
#include <min/tree.h>
#include <min/vec3.h>

namespace game
{

// Physics typedef
typedef min::physics<float, uint_fast16_t, uint_fast32_t, min::vec3, min::aabbox, min::aabbox, min::grid> physics;
typedef min::tree<float, uint_fast8_t, uint_fast8_t, min::vec2, min::aabbox, min::aabbox> ui_tree;

// Collision constants
static constexpr float _grav_mag = 10.0;
static constexpr size_t _physics_frames = 180;

// Callbacks
typedef std::function<void(min::body<float, min::vec3> &, min::body<float, min::vec3> &)> coll_call;
typedef std::function<void(void)> menu_call;
}

#endif
