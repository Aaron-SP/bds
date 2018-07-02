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
#ifndef __TERRAIN_CREATIVE__
#define __TERRAIN_CREATIVE__

#include <game/id.h>
#include <min/thread_pool.h>
#include <min/vec3.h>

namespace kernel
{

class terrain_creative
{
  private:
    const size_t _scale;

    inline size_t key(const min::tri<size_t> &index) const
    {
        return min::vec3<float>::grid_key(index, _scale);
    }

  public:
    terrain_creative(const size_t scale)
        : _scale(scale) {}

    inline void generate(min::thread_pool &pool, std::vector<game::block_id> &write) const
    {
        // Create working function
        const auto work = [this, &write](std::mt19937 &gen, const size_t i) {
            // Dope minerals in base
            std::uniform_int_distribution<uint_fast8_t> group(0, 2);
            std::uniform_int_distribution<uint_fast8_t> group1(0, 20);
            std::uniform_int_distribution<uint_fast8_t> group2(24, 30);
            std::uniform_int_distribution<uint_fast8_t> group3(32, 37);

            // Fill out this section
            for (size_t j = 0; j < _scale; j++)
            {
                for (size_t k = 0; k < _scale; k++)
                {
                    // Place block every 4 blocks away
                    const bool x = (i & 3) == 0;
                    const bool y = (j & 3) == 0;
                    const bool z = (k & 3) == 0;
                    if (x && y && z)
                    {
                        // Calculate key index
                        const size_t index = key(min::tri<size_t>(i, j, k));

                        // Roll a random block
                        switch (group(gen))
                        {
                        case 0:
                            write[index] = static_cast<game::block_id>(group1(gen));
                            break;
                        case 1:
                            write[index] = static_cast<game::block_id>(group2(gen));
                            break;
                        case 2:
                            write[index] = static_cast<game::block_id>(group3(gen));
                            break;
                        }
                    }
                }
            }
        };

        // Parallelize on X axis
        pool.run(std::cref(work), 0, _scale);
    }
};
}

#endif
