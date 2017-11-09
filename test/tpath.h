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
#ifndef __TEST_PATH__
#define __TEST_PATH__

#include <game/cgrid.h>
#include <game/path.h>
#include <stdexcept>
#include <test.h>

bool test_path()
{
    bool out = true;

    // Load the graph mesh with 128 pixel tile size
    const game::cgrid grid(64, 8, 7);

    // Search points
    const min::vec3<float> start(0.5, 36.0, -0.5);
    const min::vec3<float> dest(0.0, -24.0, 35.0);

    // Get a path data between points
    game::path_data p_data(start, dest);

    // Load a path
    game::path path;

    // Test path next direction
    const min::vec3<float> step_dir = path.step(grid, p_data);
    out = out && compare(0.0, step_dir.x(), 1E-4);
    out = out && compare(1.0, step_dir.y(), 1E-4);
    out = out && compare(0.0, step_dir.z(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed path step");
    }

    // Test path data remain
    out = out && compare(69.7173, p_data.get_remain(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed path data get_remain");
    }

    // Test path data step
    min::vec3<float> next = p_data.step(min::vec3<float>(0.0, 1.0, 0.0) * 3.0, 0.5);
    out = out && compare(0.5, next.x(), 1E-4);
    out = out && compare(37.5, next.y(), 1E-4);
    out = out && compare(-0.5, next.z(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed path data step");
    }

    // Update the path data
    p_data.update(next);

    // Test path data remain increases in path direction
    out = out && compare(71.0123, p_data.get_remain(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed path data update");
    }

    // Test path data direction to destination
    const min::vec3<float> dir = p_data.get_direction();
    out = out && compare(-0.0070, dir.x(), 1E-4);
    out = out && compare(-0.8660, dir.y(), 1E-4);
    out = out && compare(0.4999, dir.z(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed path data direction");
    }

    // Test new path direction
    p_data = game::path_data(min::vec3<float>(0.5, 36.0, -0.5), min::vec3<float>(0.0, 24.0, 22.0));

    // Clear the current path
    path.clear();

    // Step once on new path
    path.step(grid, p_data);

    // Test path size
    const std::vector<min::vec3<float>> &p = path.get_path();
    out = out && compare(36, p.size());
    if (!out)
    {
        throw std::runtime_error("Failed step path size");
    }

    // Test last point
    const min::vec3<float> &last = p.back();
    out = out && compare(0.5, last.x(), 1E-4);
    out = out && compare(24.5, last.y(), 1E-4);
    out = out && compare(22.5, last.z(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed step path");
    }

    // return status
    return out;
}

#endif
