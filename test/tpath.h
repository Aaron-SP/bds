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
    path.update(grid, p_data);

    // Test avoid
    const min::vec3<float> avoid_dir = path.avoid();
    out = out && compare(0.8756, avoid_dir.x(), 1E-4);
    out = out && compare(-0.4667, avoid_dir.y(), 1E-4);
    out = out && compare(0.1239, avoid_dir.z(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed path avoid");
    }

    // Test DFS
    const min::vec3<float> dfs_dir = path.dfs(grid, p_data);
    out = out && compare(0.0, dfs_dir.x(), 1E-4);
    out = out && compare(1.0, dfs_dir.y(), 1E-4);
    out = out && compare(0.0, dfs_dir.z(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed path dfs");
    }

    // Test Ray max dot product
    min::vec3<float> ray_dir = path.ray_sorted(0);
    out = out && compare(0.0, ray_dir.x(), 1E-4);
    out = out && compare(-1.0, ray_dir.y(), 1E-4);
    out = out && compare(0.0, ray_dir.z(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed path ray");
    }

    // Test path data zero travel
    out = out && compare(0.0, p_data.get_travel(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed path data get_travel");
    }

    // Test path data remain
    out = out && compare(69.7173, p_data.get_remain(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed path data get_remain");
    }

    // Test path data step
    min::vec3<float> next = p_data.step(ray_dir * 3.0, 0.5);
    out = out && compare(0.5, next.x(), 1E-4);
    out = out && compare(34.5, next.y(), 1E-4);
    out = out && compare(-0.5, next.z(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed path data step");
    }

    // Update the path data
    p_data.update(next);

    // Test path data remain ~-1.5 from previous
    out = out && compare(68.4306, p_data.get_remain(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed path data update");
    }

    // Test path data direction
    const min::vec3<float> dir = p_data.get_direction();
    out = out && compare(-0.0073, dir.x(), 1E-4);
    out = out && compare(-0.8548, dir.y(), 1E-4);
    out = out && compare(0.5187, dir.z(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed path data direction");
    }

    // Test path update destination
    p_data.update_destination(start);

    // Update the path
    path.update(grid, p_data);

    // Test Ray max dot product
    ray_dir = path.ray_sorted(0);
    out = out && compare(0.0, ray_dir.x(), 1E-4);
    out = out && compare(1.0, ray_dir.y(), 1E-4);
    out = out && compare(0.0, ray_dir.z(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed path update");
    }

    // Test step backwards
    next = p_data.step(ray_dir * 3.0, 0.5);
    out = out && compare(0.5, next.x(), 1E-4);
    out = out && compare(36.0, next.y(), 1E-4);
    out = out && compare(-0.5, next.z(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed path data step back");
    }

    // Update path data
    p_data.update(next);

    // Test travel step
    const float t_step = p_data.get_travel_step();
    out = out && compare(1.5, t_step, 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed path data travel step");
    }

    // Test angle step
    const float angle = p_data.get_angle_step();
    out = out && compare(1.5, angle, 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed path data angle step");
    }

    // Test ray unsorted direction
    ray_dir = path.ray_index(10);
    out = out && compare(0.0, ray_dir.x(), 1E-4);
    out = out && compare(-1.0, ray_dir.y(), 1E-4);
    out = out && compare(0.0, ray_dir.z(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed path update");
    }

    // Test eye magnitudes
    const float *const eye_mag = path.get_eye_mag();
    out = out && compare(98.0, eye_mag[10], 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed path eye down magnitude");
    }

    // Test up eye magnitudes
    out = out && compare(29.0, eye_mag[16], 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed path eye up magnitude");
    }

    // return status
    return out;
}

#endif
