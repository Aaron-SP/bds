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
#ifndef __CALLBACK__
#define __CALLBACK__

#include <functional>
#include <min/physics_nt.h>
#include <min/vec3.h>

namespace game
{

// Collision constants
static constexpr float _grav_mag = 10.0;
static constexpr size_t _physics_frames = 180;

// Callbacks
typedef std::function<void(min::body<float, min::vec3> &, min::body<float, min::vec3> &)> coll_call;
typedef std::function<std::pair<float, float>(const float, const float, const block_id)> dmg_call;
typedef std::function<void(const min::vec3<float> &, const block_id)> ex_call;
typedef std::function<void(const min::vec3<float> &, const min::vec3<unsigned> &, const block_id)> ex_scale_call;
typedef std::function<void(const min::vec3<float> &, const min::vec3<float> &)> miss_call;
typedef std::function<void(min::body<float, min::vec3> &, const min::vec3<float> &)> ray_call;
typedef std::function<void(const min::vec3<float> &, const block_id)> set_call;
}

#endif
