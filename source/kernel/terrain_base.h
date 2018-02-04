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
    perlin_noise _noise;

    inline std::tuple<size_t, size_t, size_t> key_unpack(const size_t key) const
    {
        return min::vec3<float>::grid_index(key, _scale);
    }
    inline bool on_edge(const size_t x) const
    {
        if (x == 0 || x == _scale - 1)
        {
            return true;
        }

        return false;
    }
    inline float do_perlin(const size_t index)
    {
        // Unpack grid cell components
        const auto comp = key_unpack(index);
        const size_t gx = std::get<0>(comp);
        const size_t gy = std::get<1>(comp);
        const size_t gz = std::get<2>(comp);

        if (on_edge(gx) || on_edge(gy) || on_edge(gz))
        {
            // Abort this iteration
            return 0.0;
        }

        // Relative grid components in chunk
        const float inv_cs = 1.0 / _chunk_size;
        const float rgx = gx * inv_cs;
        const float rgy = gy * inv_cs;
        const float rgz = gz * inv_cs;

        // Calculate noise for this grid cell
        return _noise.perlin(rgx, rgy, rgz);
    }
    inline void do_perlin_terrain(game::thread_pool &pool, std::vector<int8_t> &write)
    {
        // Create working function
        const auto work = [this, &write](std::mt19937 &gen, const size_t i) {
            const float value = do_perlin(i);
            if (value >= 0.0 && value < 0.05)
            {
                write[i] = 2;
            }
            else if (value >= 0.05 && value < 0.10)
            {
                write[i] = 3;
            }
            else if (value >= 0.20 && value < 0.25)
            {
                write[i] = 16;
            }
            else if (value >= 0.25 && value < 0.30)
            {
                write[i] = 17;
            }
            else if (value >= 0.40 && value < 0.45)
            {
                write[i] = 18;
            }
            else if (value >= 0.45 && value < 0.50)
            {
                write[i] = 19;
            }
        };

        // Run the job in parallel
        const size_t size = write.size();
        pool.run(work, 0, size);
    }

  public:
    terrain_base(const size_t scale, const size_t chunk_size)
        : _scale(scale), _chunk_size(chunk_size) {}

    inline void generate(game::thread_pool &pool, std::vector<int8_t> &write)
    {
        // Generate perlin noise
        do_perlin_terrain(pool, write);
    }
};
}

#endif
