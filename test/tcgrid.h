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
#ifndef __TEST_CGRID__
#define __TEST_CGRID__

#include <game/cgrid.h>
#include <stdexcept>
#include <test.h>

bool test_cgrid()
{
    bool out = true;

    // Load the graph mesh with 128 pixel tile size
    game::cgrid grid(64, 8, 7);

    // Test crashing cgrid
    min::vec3<float> p(63.999999, 63.999999, 63.999999);
    int atlas_id = grid.grid_value(p);
    out = out && compare(-2, atlas_id);
    if (!out)
    {
        throw std::runtime_error("Failed cgrid grid_value 1");
    }

    // Test crashing cgrid
    p = min::vec3<float>(63.9999962, -2.25057721, -18.796402);
    atlas_id = grid.grid_value(p);
    out = out && compare(-1, atlas_id);
    if (!out)
    {
        throw std::runtime_error("Failed cgrid grid_value 2");
    }

    // Check grid key for middle of grid
    p = min::vec3<float>(0.0, 0.0, 0.0);
    bool valid = true;
    size_t key = grid.grid_key(p, valid);
    out = out && valid;
    out = out && compare(1056832, key);
    if (!out)
    {
        throw std::runtime_error("Failed cgrid grid_key 1");
    }

    // Check grid key for bottom left corner
    p = min::vec3<float>(-63.99999, -63.99999, -63.99999);
    valid = true;
    key = grid.grid_key(p, valid);
    out = out && valid;
    out = out && compare(0, key);
    if (!out)
    {
        throw std::runtime_error("Failed cgrid grid_key 2");
    }

    // Check grid key for bottom left corner
    p = min::vec3<float>(-64.0, -64.0, -64.0);
    valid = true;
    key = grid.grid_key(p, valid);
    out = out && !valid;
    out = out && compare(0, key);
    if (!out)
    {
        throw std::runtime_error("Failed cgrid grid_key 3");
    }

    // Test cgrid search
    min::vec3<float> start(0.5, 36.0, -0.5);
    min::vec3<float> stop(4.5, 31.5, 0.0);
    std::vector<min::vec3<float>> path;
    grid.path(path, start, stop);
    out = out && compare(11, path.size());
    if (!out)
    {
        throw std::runtime_error("Failed cgrid path size");
    }

    // Check last point
    out = out && compare(4.5, path[5].x(), 1E-4);
    out = out && compare(35.5, path[5].y(), 1E-4);
    out = out && compare(-0.5, path[5].z(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed cgrid path 1");
    }

    // Check next to last point
    out = out && compare(4.5, path[4].x(), 1E-4);
    out = out && compare(36.5, path[4].y(), 1E-4);
    out = out && compare(-0.5, path[4].z(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed cgrid path 2");
    }

    // Check first point
    out = out && compare(0.5, path[0].x(), 1E-4);
    out = out && compare(36.5, path[0].y(), 1E-4);
    out = out && compare(-0.5, path[0].z(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed cgrid path 3");
    }

    // Test cgrid hard search
    start = min::vec3<float>(0.5, 36.0, -0.5);
    stop = min::vec3<float>(0.0, 24.0, 22.0);
    grid.path(path, start, stop);
    out = out && compare(21, path.size());
    if (!out)
    {
        throw std::runtime_error("Failed cgrid hard path size");
    }

    // Test farthest point on incomplete path
    out = out && compare(0.5, path[20].x(), 1E-4);
    out = out && compare(31.5, path[20].y(), 1E-4);
    out = out && compare(10.5, path[20].z(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed cgrid hard path 1");
    }

    // Test searching outside the world
    start = min::vec3<float>(66.0, 66.0, 66.0);
    stop = min::vec3<float>(65.0, 65.0, 65.0);
    grid.path(path, start, stop);
    out = out && compare(0, path.size());
    if (!out)
    {
        throw std::runtime_error("Failed cgrid path outside world");
    }

    // return status
    return out;
}

#endif
