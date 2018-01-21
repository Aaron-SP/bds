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
#ifndef __CGRID_GENERATOR__
#define __CGRID_GENERATOR__

#include <cmath>
#include <fstream>
#include <game/height_map.h>
#include <game/work_queue.h>
#include <kernel/brownian_grow.h>
#include <kernel/mandelbulb.h>
#include <min/vec3.h>
#include <random>
#include <sstream>
#include <string>

namespace game
{

class cgrid_generator
{
  private:
    std::vector<int8_t> _copy;
    int _density;
    int _seed;
    int _length;

    inline void load_config()
    {
        std::string line;
        std::ifstream file("data/config/gen.config");
        if (file.is_open())
        {
            // Read density
            getline(file, line);
            std::stringstream ss(line);
            ss >> _density;
            if (ss.fail())
            {
                _density = 30000;
            }

            // Read seed
            getline(file, line);
            ss.clear();
            ss.str(std::string());
            ss >> _seed;
            if (ss.fail())
            {
                _seed = 100;
            }

            // Read length
            getline(file, line);
            ss.clear();
            ss.str(std::string());
            ss >> _length;
            if (ss.fail())
            {
                _length = 20;
            }

            // Close the file
            file.close();
        }
    }

  public:
    cgrid_generator(const std::vector<int8_t> &grid)
        : _copy(grid.size(), -1), _density(30000), _seed(100), _length(20)
    {
        // Load the terrain config file
        load_config();

        std::cout << _density << std::endl;
        std::cout << _seed << std::endl;
        std::cout << _length << std::endl;
    }
    inline void copy(std::vector<int8_t> &grid) const
    {
        // Parallelize on copying buffers
        const auto work = [this, &grid](const size_t i) {
            grid[i] = _copy[i];
        };

        // Convert cells to mesh in parallel
        work_queue::worker.run(work, 0, grid.size());
    }
    void generate(std::vector<int8_t> &grid, const size_t scale,
                  const std::function<size_t(const std::tuple<size_t, size_t, size_t> &)> &grid_key_unpack,
                  const std::function<min::vec3<float>(const size_t)> &grid_cell_center)
    {
        // Wake up the threads for processing
        work_queue::worker.wake();

        // 1 / 4 of the bottom half of the grid is filled
        const size_t floor_height = std::ceil(scale * 0.0625);

        // Random numbers between 0 and 5, including both
        std::uniform_int_distribution<int8_t> dist(0, 5);
        std::mt19937 gen(std::chrono::high_resolution_clock::now().time_since_epoch().count());

        // Generate height map
        const size_t level = std::ceil(std::log2(scale));
        const height_map<float, float> map(level, -1.0, 4.0);

        // Parallelize on x-axis
        const auto work = [this, &dist, &gen,
                           &map, scale, floor_height,
                           grid_key_unpack](const size_t i) {

            // y axis
            for (size_t j = 0; j < floor_height; j++)
            {
                // z axis
                for (size_t k = 0; k < scale; k++)
                {
                    // Get the height
                    const uint8_t height = static_cast<uint8_t>(std::round(map.get(i, k)));
                    if (j < height)
                    {
                        // Set ground textures
                        _copy[grid_key_unpack(std::make_tuple(i, j, k))] = dist(gen);
                    }
                }
            }
        };

        // Run height map in parallel
        work_queue::worker.run(work, 0, scale);

        // Copy data from back to front
        copy(grid);

        // Simulates growing
        kernel::brownian_grow grow(scale);

        // Highest point is height * 2
        const size_t height = std::ceil(scale * 0.85);
        for (size_t i = floor_height; i < height; i++)
        {
            grow.set_height(i);
            grow.generate(work_queue::worker, grid, _copy, _density, _seed, _length);

            // Copy data from front to back
            copy(grid);
        }

        // generate mandelbulb world using mandelbulb generator
        kernel::mandelbulb().generate(work_queue::worker, grid, scale, [grid_cell_center](const size_t i) {
            return grid_cell_center(i);
        });

        // Put the threads back to sleep
        work_queue::worker.sleep();
    }
};
}

#endif
