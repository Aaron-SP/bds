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
#ifndef __TERRAIN_BASE__
#define __TERRAIN_BASE__

#include <game/perlin.h>
#include <game/thread_pool.h>
#include <min/vec3.h>

namespace kernel
{

class terrain_base
{
  private:
    const size_t _scale;
    const size_t _chunk_size;
    const size_t _start;
    const size_t _stop;
    perlin_noise _noise;

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
    inline float do_perlin(const size_t x, const size_t y, const size_t z) const
    {
        if (on_edge(x) || on_edge(y) || on_edge(z))
        {
            // Abort this iteration
            return 0.0;
        }

        // Relative grid components in chunk
        const float inv_cs = 1.0 / _chunk_size;
        const float rx = x * inv_cs;
        const float ry = y * inv_cs;
        const float rz = z * inv_cs;

        // Calculate noise for this grid cell
        return _noise.perlin(rx, ry, rz);
    }

  public:
    terrain_base(const size_t scale, const size_t chunk_size, const size_t start, const size_t stop)
        : _scale(scale), _chunk_size(chunk_size), _start(start), _stop(stop) {}

    inline void generate(game::thread_pool &pool, std::vector<int8_t> &write) const
    {
        // Create working function
        const auto work = [this, &write](std::mt19937 &gen, const size_t i) {

            // Fill out this section
            for (size_t j = _start; j < _stop; j++)
            {
                for (size_t k = 0; k < _scale; k++)
                {
                    // Calculate 3d perlin
                    const float value = do_perlin(i, j, k);

                    // Calculate key index
                    const size_t index = key(std::make_tuple(i, j, k));
                    if (value >= 0.0 && value < 0.05)
                    {
                        write[index] = 2;
                    }
                    else if (value >= 0.05 && value < 0.10)
                    {
                        write[index] = 3;
                    }
                    else if (value >= 0.20 && value < 0.25)
                    {
                        write[index] = 16;
                    }
                    else if (value >= 0.25 && value < 0.30)
                    {
                        write[index] = 17;
                    }
                    else if (value >= 0.40 && value < 0.45)
                    {
                        write[index] = 18;
                    }
                    else if (value >= 0.45 && value < 0.50)
                    {
                        write[index] = 19;
                    }
                }
            }
        };

        // Parallelize on X axis
        pool.run(work, 0, _scale);
    }
};
}

#endif
