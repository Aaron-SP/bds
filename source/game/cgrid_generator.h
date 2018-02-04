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
#ifndef __CGRID_GENERATOR__
#define __CGRID_GENERATOR__

#include <cmath>
#include <fstream>
#include <game/work_queue.h>
#include <kernel/brownian_grow.h>
#include <kernel/mandelbulb.h>
#include <kernel/terrain_base.h>
#include <kernel/terrain_height.h>
#include <min/vec3.h>
#include <random>
#include <sstream>
#include <string>

namespace game
{

class cgrid_generator
{
  private:
    std::vector<int8_t> _back;
    int _radius;
    int _seed;
    int _years;

    inline void load_config()
    {
        std::string line;
        std::ifstream file("data/config/gen.config");
        if (file.is_open())
        {
            // Read seed density
            getline(file, line);
            std::stringstream ss(line);
            ss >> _radius;
            if (ss.fail())
            {
                _radius = 10;
            }

            // Read seed
            getline(file, line);
            ss.clear();
            ss.str(line);
            ss >> _seed;
            if (ss.fail())
            {
                _seed = 1000;
            }

            // Read length
            getline(file, line);
            ss.clear();
            ss.str(line);
            ss >> _years;
            if (ss.fail())
            {
                _years = 100;
            }

            // Close the file
            file.close();
        }
    }

  public:
    cgrid_generator(const std::vector<int8_t> &grid)
        : _back(grid.size(), -1), _seed(100), _years(20)
    {
        // Load the terrain config file
        load_config();

        std::cout << "RADIUS: " << _radius << std::endl;
        std::cout << "SEED: " << _seed << std::endl;
        std::cout << "YEARS: " << _years << std::endl;
    }
    inline void copy(std::vector<int8_t> &grid) const
    {
        // Parallelize on copying buffers
        const auto work = [this, &grid](std::mt19937 &gen, const size_t i) {
            grid[i] = _back[i];
        };

        // Convert cells to mesh in parallel
        work_queue::worker.run(work, 0, grid.size());
    }
    void generate(std::vector<int8_t> &grid, const size_t scale, const size_t chunk_size,
                  const std::function<size_t(const std::tuple<size_t, size_t, size_t> &)> &grid_key_unpack,
                  const std::function<min::vec3<float>(const size_t)> &grid_cell_center)
    {
        // Wake up the threads for processing
        work_queue::worker.wake();

        // Calculates a height map
        kernel::terrain_height height(scale);
        height.generate(work_queue::worker, _back);

        // Simulates growing
        kernel::brownian_grow grow(scale, _radius, _seed, _back);

        // Copy data from back to front buffer
        copy(grid);

        grow.generate(work_queue::worker, grid, _back, _years);

        // Copy data from front to back
        copy(grid);

        // Calculates perlin noise
        //kernel::terrain_base base(scale, chunk_size);
        //base.generate(work_queue::worker, grid);

        // generate mandelbulb world using mandelbulb generator
        // kernel::mandelbulb().generate(work_queue::worker, grid, scale, [grid_cell_center](const size_t i) {
        //     return grid_cell_center(i);
        // });

        // Put the threads back to sleep
        work_queue::worker.sleep();
    }
};
}

#endif
