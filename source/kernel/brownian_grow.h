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

#include <game/id.h>
#include <game/thread_pool.h>
#include <min/vec3.h>
#include <tuple>
#include <vector>

namespace kernel
{

class brownian_grow
{
  private:
    const size_t _scale;
    const size_t _seed;
    const size_t _radius;
    std::vector<min::tri<size_t>> _points;

    inline static size_t add(const size_t x, const int dx)
    {
        return static_cast<size_t>(static_cast<int>(x) + dx);
    }
    inline size_t key(const min::tri<size_t> &index) const
    {
        return min::vec3<float>::grid_key(index, _scale);
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
    inline static game::block_id color_table(const game::block_id value)
    {
        // This needs to be updated!
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
    inline bool random_walk(const std::vector<game::block_id> &read, min::tri<size_t> &walker, const uint_fast8_t dir, game::block_id &value) const
    {
        // Copy walker position
        min::tri<size_t> next = walker;

        // Reflect walker if on edge
        bool edge = false;
        if (on_edge(next.x()))
        {
            edge = true;
        }
        else if (on_edge(next.y()))
        {
            edge = true;
        }
        else if (on_edge(next.z()))
        {
            edge = true;
        }

        // Calculate random walk direction if not on edge
        if (!edge)
        {
            switch (dir)
            {
            case 0:
                next.x()--;
                break;
            case 1:
                next.x()++;
                break;
            case 2:
                next.y()--;
                break;
            case 3:
                next.y()++;
                break;
            case 4:
                next.z()--;
                break;
            case 5:
                next.z()++;
                break;
            }
        }

        // Is the move point a wall?
        value = read[key(next)];
        if (value == game::block_id::EMPTY)
        {
            // Move the walker
            walker = next;

            // Didn't hit a wall
            return false;
        }

        // We hit a wall
        return true;
    }
    inline void do_brownian(game::thread_pool &pool, const std::vector<game::block_id> &read, std::vector<game::block_id> &write, const size_t years) const
    {
        // Create working function
        const auto work = [this, &read, &write, years](std::mt19937 &gen, const size_t i) {
            // Create random number generator for this thread
            std::uniform_int_distribution<int> gdist(-_radius, _radius);
            std::uniform_int_distribution<uint_fast8_t> idist(0, 5);

            // Spawn a walker at each seed location
            for (size_t j = 0; j < _seed; j++)
            {
                // Spawn walker at random offset from seed location
                const size_t x = add(_points[j].x(), gdist(gen));
                const size_t y = add(_points[j].y(), gdist(gen));
                const size_t z = add(_points[j].z(), gdist(gen));
                auto walker = min::tri<size_t>(x, y, z);

                // Evolve simulation for years
                for (size_t k = 0; k < years; k++)
                {
                    // Calculate random direction
                    const uint_fast8_t dir = idist(gen);

                    // If walker hits something in write buffer
                    game::block_id value;
                    if (random_walk(read, walker, dir, value))
                    {
                        // Calculate grid cell
                        const size_t cell = key(walker);

                        // Seed the write buffer
                        write[cell] = color_table(value);

                        // Respawn walker
                        const size_t x1 = add(_points[j].x(), gdist(gen));
                        const size_t y1 = add(_points[j].y(), gdist(gen));
                        const size_t z1 = add(_points[j].z(), gdist(gen));
                        walker = min::tri<size_t>(x1, y1, z1);
                    }
                }
            }
        };

        // Get number of cpu cores
        const size_t size = std::thread::hardware_concurrency();
        if (size < 1)
        {
            throw std::runtime_error("brownian_grow: can't determine number of CPU cores");
        }

        // Run the job in parallel
        pool.run(std::cref(work), 0, size);
    }

  public:
    brownian_grow(std::mt19937 &gen, std::vector<game::block_id> &write, const size_t scale, const size_t radius, const size_t seed)
        : _scale(scale), _seed(seed), _radius(radius)
    {
        // Check if radius is valid
        if (_radius >= _scale / 2)
        {
            throw std::runtime_error("brownian_grow: radius larger than world size");
        }

        // Create random number generator for this thread
        std::uniform_int_distribution<size_t> gdist(_radius, _scale - _radius - 1);

        // Randomly seed buffer
        _points.resize(_seed);
        for (size_t i = 0; i < _seed; i++)
        {
            // Calculate random cell in the grid and write value
            _points[i] = min::tri<size_t>(gdist(gen), gdist(gen), gdist(gen));

            // Calculate grid cell
            const size_t cell = key(_points[i]);

            // Write pixel into grid
            write[cell] = color_table(i % 24);
        }
    }
    inline void generate(game::thread_pool &pool, const std::vector<game::block_id> &read, std::vector<game::block_id> &write, const size_t years)
    {
        // Generate brownian tree
        do_brownian(pool, read, write, years);
    }
};
}

#endif
