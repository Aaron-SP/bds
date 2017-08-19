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
#include <game/file.h>
#include <stdexcept>
#include <test.h>

bool test_cgrid()
{
    bool out = true;

    // Load the graph mesh with 128 pixel tile size
    game::cgrid grid(64, 8, 7);

    // Test no segfault
    const min::vec3<float> out_of_bounds(-64.00001, -64.00001, -64.00001);
    std::vector<int8_t> neighbors = grid.get_neighbors(out_of_bounds);

    // Check 26 borders
    const size_t size = neighbors.size();
    for (size_t i = 0; i < size - 1; i++)
    {
        out = out && compare(-2, neighbors[i]);
        if (!out)
        {
            throw std::runtime_error("Failed cgrid get_neighbors out_of_bounds");
        }
    }

    // Check one empty
    out = out && compare(-1, neighbors[size - 1]);
    if (!out)
    {
        throw std::runtime_error("Failed cgrid get_neighbors out_of_bounds");
    }

    // Check grid key for middle of grid
    min::vec3<float> p(0.0, 0.0, 0.0);
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

    // return status
    return out;
}

#endif
