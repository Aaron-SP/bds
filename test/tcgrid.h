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
#ifndef __TEST_CGRID__
#define __TEST_CGRID__

#include <game/cgrid.h>
#include <stdexcept>
#include <test.h>

bool test_cgrid()
{
    bool out = true;

    // Load the graph mesh with 128 pixel tile size
    game::cgrid grid(8, 64, 7);

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

    // Check first point
    out = out && compare(0.5, path[0].x(), 1E-4);
    out = out && compare(36.5, path[0].y(), 1E-4);
    out = out && compare(-0.5, path[0].z(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed cgrid path 3");
    }

    // Check last point
    out = out && compare(4.5, path[10].x(), 1E-4);
    out = out && compare(31.5, path[10].y(), 1E-4);
    out = out && compare(0.5, path[10].z(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed cgrid path 1");
    }

    // Check next to last point
    out = out && compare(4.5, path[9].x(), 1E-4);
    out = out && compare(31.5, path[9].y(), 1E-4);
    out = out && compare(-0.5, path[9].z(), 1E-4);
    if (!out)
    {
        throw std::runtime_error("Failed cgrid path 2");
    }

    // Test cgrid hard search
    start = min::vec3<float>(0.5, 36.0, -0.5);
    stop = min::vec3<float>(0.0, 24.0, 22.0);
    grid.path(path, start, stop);
    out = out && compare(36, path.size());
    if (!out)
    {
        throw std::runtime_error("Failed cgrid hard path size");
    }

    // Test farthest point on hard path
    out = out && compare(0.5, path[35].x(), 1E-4);
    out = out && compare(24.5, path[35].y(), 1E-4);
    out = out && compare(22.5, path[35].z(), 1E-4);
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
