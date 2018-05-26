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
#include <game/id.h>
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
    inline void terrain(game::thread_pool &pool, std::vector<game::block_id> &write, const game::height_map<float, float> &map) const
    {
        // Parallelize on X axis
        const auto work = [this, &map, &write](std::mt19937 &gen, const size_t i) {
            const int_fast8_t grass_start = game::id_value(game::block_id::GRASS1);
            const int_fast8_t grass_end = game::id_value(game::block_id::GRASS2);
            const int_fast8_t dirt_start = game::id_value(game::block_id::DIRT1);
            const int_fast8_t dirt_end = game::id_value(game::block_id::DIRT2);
            const int_fast8_t sand_start = game::id_value(game::block_id::SAND1);
            const int_fast8_t sand_end = game::id_value(game::block_id::SAND2);
            std::uniform_int_distribution<int_fast8_t> grass(grass_start, grass_end);
            std::uniform_int_distribution<int_fast8_t> soil(dirt_start, dirt_end);
            std::uniform_int_distribution<int_fast8_t> sand(sand_start, sand_end);

            // Z axis
            for (size_t k = 0; k < _scale; k++)
            {
                // Get the height
                const size_t level = static_cast<size_t>(std::round(map.get(i, k)));
                const size_t height = (level > _stop) ? _stop : level;
                const size_t mid = _start + (height / 2);
                const size_t end = _start + (height - 1);

                // Sand section
                for (size_t j = _start; j < mid; j++)
                {
                    const size_t write_key = key(std::make_tuple(i, j, k));
                    write[write_key] = static_cast<game::block_id>(sand(gen));
                }

                // Soil section
                for (size_t j = mid; j < end; j++)
                {
                    const size_t write_key = key(std::make_tuple(i, j, k));
                    write[write_key] = static_cast<game::block_id>(soil(gen));
                }

                // Grass surface
                const size_t write_key = key(std::make_tuple(i, end, k));
                write[write_key] = static_cast<game::block_id>(grass(gen));
            }
        };

        // Run height map in parallel
        pool.run(work, 0, _scale);
    }
    inline void plants(game::thread_pool &pool, std::vector<game::block_id> &write, const game::height_map<float, float> &map, const size_t size) const
    {
        // Parallelize on X axis
        const auto work = [this, &map, &write](std::mt19937 &gen, const size_t i) {
            const int_fast8_t plant_start = game::id_value(game::block_id::TOMATO);
            const int_fast8_t plant_end = game::id_value(game::block_id::GREEN_PEPPER);
            std::uniform_int_distribution<int_fast8_t> plant(plant_start, plant_end);

            // Get random X/Z coord, Y from height map
            std::uniform_int_distribution<size_t> p(3, _scale - 4);
            const size_t x = p(gen);
            const size_t z = p(gen);
            const size_t y = _start + static_cast<size_t>(std::round(map.get(x, z)));

            // Create plants in empty cells on top of height map
            const size_t write_key = key(std::make_tuple(x, y, z));
            if (write[write_key] == game::block_id::EMPTY)
            {
                write[write_key] = static_cast<game::block_id>(plant(gen));
            }
        };

        // Run height map in parallel
        pool.run(work, 0, size);
    }
    inline void trees(game::thread_pool &pool, std::vector<game::block_id> &write, const game::height_map<float, float> &map, const size_t size) const
    {
        // Parallelize on X axis
        const auto work = [this, &map, &write](std::mt19937 &gen, const size_t i) {
            const int_fast8_t leaf_start = game::id_value(game::block_id::LEAF1);
            const int_fast8_t leaf_end = game::id_value(game::block_id::LEAF4);
            const int_fast8_t wood_start = game::id_value(game::block_id::WOOD1);
            const int_fast8_t wood_end = game::id_value(game::block_id::WOOD2);

            // Random numbers between 5 and 13, including both
            std::uniform_int_distribution<uint_fast8_t> tree_size(4, 18);
            std::uniform_int_distribution<int_fast8_t> wood(wood_start, wood_end);
            std::uniform_int_distribution<int_fast8_t> leaf(leaf_start, leaf_end);

            // Get random X/Z coord
            std::uniform_int_distribution<size_t> p(3, _scale - 4);
            const size_t x = p(gen);
            const size_t z = p(gen);

            // Get the top of trees at X/Z coord
            const size_t tree_base = _start + static_cast<size_t>(std::round(map.get(x, z)));
            const size_t tree_height = tree_base + tree_size(gen);
            const size_t tree_top = (tree_height > _stop) ? _stop : tree_height;

            // Create tree wood
            const int_fast8_t wood_type = wood(gen);
            for (size_t y = tree_base; y < tree_top; y++)
            {
                const size_t write_key = key(std::make_tuple(x, y, z));
                write[write_key] = static_cast<game::block_id>(wood_type);
            }

            // Leaf start position and leaf type
            const size_t x_start = x - 2;
            const size_t y_start = tree_top - 2;
            const size_t z_start = z - 2;
            const int_fast8_t leaf_type = leaf(gen);

            // Generate cubic leaves
            std::uniform_int_distribution<uint_fast8_t> leaf_offset(0, 1);
            const size_t dx = leaf_offset(gen);
            const size_t x_end = x_start + (5 - dx);
            for (size_t x = x_start + dx; x < x_end; x++)
            {
                const size_t y_end = y_start + 3;
                for (size_t y = y_start; y < y_end; y++)
                {
                    const size_t dz = leaf_offset(gen);
                    const size_t z_end = z_start + (5 - dz);
                    for (size_t z = z_start + dz; z < z_end; z++)
                    {
                        const size_t write_key = key(std::make_tuple(x, y, z));
                        write[write_key] = static_cast<game::block_id>(leaf_type);
                    }
                }
            }
        };

        // Run height map in parallel
        pool.run(work, 0, size);
    }

  public:
    terrain_height(const size_t scale, const size_t start, const size_t stop)
        : _scale(scale), _start(start), _stop(stop) {}

    inline void generate(game::thread_pool &pool, std::mt19937 &gen, std::vector<game::block_id> &write) const
    {
        // Generate height map
        const size_t level = std::ceil(std::log2(_scale));
        const game::height_map<float, float> map(level, 4.0, 8.0);

        // Generate terrain
        terrain(pool, write, map);

        // Generate trees
        std::uniform_int_distribution<size_t> tree_dist(250, 1000);
        trees(pool, write, map, tree_dist(gen));

        // Generate plants
        std::uniform_int_distribution<size_t> plant_dist(50, 150);
        plants(pool, write, map, plant_dist(gen));
    }
};
}

#endif
