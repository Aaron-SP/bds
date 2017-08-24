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
#ifndef __TEST_AI_PATH__
#define __TEST_AI_PATH__

#include <game/ai_path.h>
#include <game/cgrid.h>
#include <stdexcept>
#include <test.h>

bool test_ai_path()
{
    bool out = true;

    // Load the graph mesh with 128 pixel tile size
    const game::cgrid grid(64, 8, 7);

    // Load a path
    const game::ai_path path;

    // Search points
    const min::vec3<float> start(0.5, 36.0, -0.5);
    const min::vec3<float> dest(0.0, -24.0, 35.0);

    // Get a path data between points
    const game::path_data p_data(start, dest);

    // Find a path between points test first point
    // const min::vec3<float> nn_dir = path.path(grid, p_data);
    // out = out && compare(-0.0213, nn_dir.x(), 1E-4);
    // out = out && compare(-1.1338, nn_dir.y(), 1E-4);
    // out = out && compare(-1.7183, nn_dir.z(), 1E-4);
    // if (!out)
    // {
    //     throw std::runtime_error("Failed ai_path neural path");
    // }

    // return status
    return out;
}

#endif
