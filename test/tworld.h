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
#ifndef __TESTWORLD__
#define __TESTWORLD__

#include <min/window.h>
#include <stdexcept>
#include <test.h>
#include <world.h>

bool test_world()
{
    bool out = true;

    // Create window for rendering
    min::window window("test world mesh", 720, 480, 3, 3);

    // Load the graph mesh with 128 pixel tile size
    game::world world("data/texture/atlas.bmp", 64, 8, 7);

    world.add_block(min::vec3<float>(0.0, 0.0, 0.0));
    world.add_block(min::vec3<float>(1.0, 1.0, 1.0));
    world.add_block(min::vec3<float>(2.0, 2.0, 2.0));

    // return status
    return out;
}

#endif
