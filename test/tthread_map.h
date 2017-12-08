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
#ifndef __TEST_THREAD_MAP__
#define __TEST_THREAD_MAP__

#include <game/thread_map.h>
#include <stdexcept>
#include <test.h>

bool test_thread_map()
{
    bool out = true;

    // Create a threadpool for doing work in parallel
    game::thread_map map;

    // Work
    std::vector<int> items = {0, 1, 2, 3, 4, 5, 6, 7};

    // Create working function
    const auto work = [&items](const size_t i) {
        items[i]++;
    };

    // Run the job in parallel
    map.run(work, 0, 8);

    // Run test
    bool passed = true;
    for (int i = 0; i < 8; i++)
    {
        if (items[i] != (i + 1))
        {
            passed = false;
        }
    }

    // Test thread map
    out = out && passed;
    if (!out)
    {
        throw std::runtime_error("Failed thread map test");
    }

    // return status
    return out;
}

#endif
