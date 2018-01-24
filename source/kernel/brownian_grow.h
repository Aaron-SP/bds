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
#ifndef __BROWNIAN_GROW__
#define __BROWNIAN_GROW__

#include <chrono>
#include <game/thread_pool.h>
#include <min/vec3.h>
#include <random>
#include <tuple>
#include <vector>

namespace kernel
{

class brownian_grow
{
  private:
    size_t _scale;
    std::uniform_int_distribution<uint8_t> _idist;
    std::uniform_int_distribution<size_t> _gdist;
    std::mt19937 _gen;
    std::vector<std::tuple<size_t, size_t, size_t>> _walkers;
    size_t _y;

    inline size_t key(const std::tuple<size_t, size_t, size_t> &index) const
    {
        return min::vec3<float>::grid_key(index, _scale);
    }
    inline uint8_t random_int()
    {
        return _idist(_gen);
    }
    inline size_t random_grid_point()
    {
        return _gdist(_gen);
    }
    inline bool on_edge(size_t &x) const
    {
        // Reflect walkers if hit a wall
        if (x == 0)
        {
            x++;
            return true;
        }
        else if (x == _scale - 1)
        {
            x--;
            return true;
        }

        return false;
    }
    inline std::tuple<size_t, size_t, size_t> random_index()
    {
        // Generate spawn point
        const size_t x = random_grid_point();
        const size_t z = random_grid_point();

        // Spawn walker
        return std::make_tuple(x, _y, z);
    }
    inline int8_t color_table(const int8_t value)
    {
        switch (value)
        {
        // Group 1
        case 0:
        case 2:
        case 3:
        case 8:
            return 9;
        case 9:
            return 10;
        case 10:
            return 11;
        case 11:
            return 12;
        case 12:
            return 13;
        case 13:
            return 8;

        // Group 2
        case 1:
        case 4:
        case 5:
        case 16:
            return 17;
        case 17:
            return 18;
        case 18:
            return 19;
        case 19:
            return 20;
        case 20:
            return 21;
        case 21:
            return 16;

        default:
            return 8;
        }
    }
    inline bool random_walk(const std::vector<int8_t> &read, const size_t i, int8_t &value)
    {
        // Move in direction
        std::tuple<size_t, size_t, size_t> next = _walkers[i];

        // Reflect walker if on edge
        bool edge = false;
        if (on_edge(std::get<0>(next)))
        {
            edge = true;
        }
        else if (on_edge(std::get<1>(next)))
        {
            edge = true;
        }
        else if (on_edge(std::get<2>(next)))
        {
            edge = true;
        }

        // Calculate random walk direction if not on edge
        if (!edge)
        {
            const uint8_t dir = random_int();
            switch (dir)
            {
            case 0:
                std::get<0>(next)--;
                break;
            case 1:
                std::get<0>(next)++;
                break;
            case 2:
                std::get<1>(next)--;
                break;
            case 3:
                std::get<1>(next)++;
                break;
            case 4:
                std::get<2>(next)--;
                break;
            case 5:
                std::get<2>(next)++;
                break;
            }
        }

        // Is the move point a wall?
        value = read[key(next)];
        if (value == -1)
        {
            // Move the walker
            _walkers[i] = next;

            // Didn't hit a wall
            return false;
        }

        // We hit a wall
        return true;
    }
    inline void do_brownian(game::thread_pool &pool, const std::vector<int8_t> &read, std::vector<int8_t> &write, const size_t seed, const size_t years)
    {
        // Calculate a random tree seed
        for (size_t i = 0; i < seed; i++)
        {
            // Seed the write buffer
            const size_t value = (i % 3) * 8;
            write[key(random_index())] = color_table(value);
        }

        // Evolve simulation for years
        for (size_t i = 0; i < years; i++)
        {
            // Create working function
            const auto work = [this, &read, &write, i](const size_t j) {

                // If walker hits something in write buffer
                int8_t value;
                if (random_walk(read, j, value))
                {
                    // Seed the write buffer
                    write[key(_walkers[j])] = color_table(value);

                    // Move walker
                    spawn_walker(j);
                }
            };

            // Run the job in parallel
            pool.run(work, 0, _walkers.size());
        }
    }
    inline void spawn_walker(const size_t i)
    {
        // Spawn walker
        _walkers[i] = random_index();
    }
    inline void spawn_walkers(const size_t size)
    {
        // Allocate walkers
        _walkers.resize(size);

        // Randomly scatter walkers
        for (size_t i = 0; i < size; i++)
        {
            spawn_walker(i);
        }
    }

  public:
    brownian_grow(const size_t scale)
        : _scale(scale), _idist(0, 5), _gdist(0, _scale - 1),
          _gen(std::chrono::high_resolution_clock::now().time_since_epoch().count()),
          _y(0) {}

    inline void set_height(const size_t height)
    {
        _y = height;
    }
    inline void generate(game::thread_pool &pool, const std::vector<int8_t> &read, std::vector<int8_t> &write, const size_t walkers, const size_t seed, const size_t years)
    {
        // Spawn the random walkers
        spawn_walkers(walkers);

        // Generate brownian tree
        do_brownian(pool, read, write, seed, years);
    }
};
}

#endif
