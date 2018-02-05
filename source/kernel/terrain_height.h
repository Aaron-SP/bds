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
#ifndef __TERRAIN_HEIGHT__
#define __TERRAIN_HEIGHT__

#include <game/height_map.h>
#include <game/thread_pool.h>
#include <min/vec3.h>

namespace kernel
{

class terrain_height
{
  private:
    const size_t _scale;
    const size_t _start;
    const size_t _stop;

    inline size_t key(const std::tuple<size_t, size_t, size_t> &index) const
    {
        return min::vec3<float>::grid_key(index, _scale);
    }
    inline bool on_edge(const size_t x) const
    {
        if (x == 0 || x == _scale - 1)
        {
            return true;
        }

        return false;
    }

  public:
    terrain_height(const size_t scale, const size_t start, const size_t stop)
        : _scale(scale), _start(start), _stop(stop) {}

    inline void generate(game::thread_pool &pool, std::vector<int8_t> &write)
    {
        // Generate height map
        const size_t level = std::ceil(std::log2(_scale));
        const game::height_map<float, float> map(level, 4.0, 8.0);

        // Parallelize on X axis
        const auto work = [this, &map, &write](std::mt19937 &gen, const size_t i) {

            // Random numbers between 0 and 5, including both
            std::uniform_int_distribution<int8_t> grass(0, 1);
            std::uniform_int_distribution<int8_t> soil(2, 3);
            std::uniform_int_distribution<int8_t> sand(4, 5);

            // Z axis
            for (size_t k = 0; k < _scale; k++)
            {
                // Get the height
                const size_t level = static_cast<size_t>(std::round(map.get(i, k)));
                const size_t height = (level > _stop) ? _stop : level;
                const size_t mid = _start + height / 2;
                const size_t stop = _start + height;
                const size_t end = stop - 1;

                // First section
                for (size_t j = _start; j < mid; j++)
                {
                    write[key(std::make_tuple(i, j, k))] = sand(gen);
                }

                // Mid section
                for (size_t j = mid; j < end; j++)
                {
                    write[key(std::make_tuple(i, j, k))] = soil(gen);
                }

                // Surface section
                write[key(std::make_tuple(i, end, k))] = grass(gen);
            }
        };

        // Run height map in parallel
        pool.run(work, 0, _scale);
    }
};
}

#endif
