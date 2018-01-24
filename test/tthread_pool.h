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
#ifndef __TEST_THREAD_POOL__
#define __TEST_THREAD_POOL__

#include <game/thread_pool.h>
#include <stdexcept>
#include <test.h>

bool test_thread_pool()
{
    bool out = true;

    // Create a threadpool for doing work in parallel
    game::thread_pool pool;

    // Work items
    std::vector<int> items = {0, 1, 2, 3, 4, 5, 6, 7};

    // Create working function
    const auto work = [&items](const size_t i) {
        items[i]++;
    };

    // Run the job in parallel
    pool.run(work, 0, 8);

    // Run the job in parallel
    pool.run(work, 0, 8);

    // Run the job in parallel
    pool.run(work, 0, 8);

    // Kill the pool
    pool.kill();

    // Run test
    bool passed = true;
    for (int i = 0; i < 8; i++)
    {
        if (items[i] != (i + 3))
        {
            passed = false;
        }
    }

    // Test thread map
    out = out && passed;
    if (!out)
    {
        throw std::runtime_error("Failed thread pool test");
    }

    // return status
    return out;
}

#endif
