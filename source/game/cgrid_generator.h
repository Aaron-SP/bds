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
#include <game/id.h>
#include <game/work_queue.h>
#include <kernel/mandelbulb_asym.h>
#include <kernel/mandelbulb_exp.h>
#include <kernel/mandelbulb_sym.h>
#include <kernel/terrain_base.h>
#include <kernel/terrain_height.h>
#include <min/vec3.h>
#include <random>

namespace game
{

class cgrid_generator
{
  private:
    std::vector<block_id> _back;

    inline void clear_grid(std::vector<block_id> &grid)
    {
        // Parallelize on copying buffers
        const auto work = [&grid](std::mt19937 &gen, const size_t i) {
            grid[i] = block_id::EMPTY;
        };

        // Convert cells to mesh in parallel
        work_queue::worker.run(work, 0, grid.size());
    }
    inline size_t count_grid(std::vector<block_id> &grid)
    {
        // Out variable
        size_t count = 0;

        // Count all active cells in grid
        const size_t size = grid.size();
        for (size_t i = 0; i < size; i++)
        {
            if (grid[i] != block_id::EMPTY)
            {
                count++;
            }
        }

        // Return count;
        return count;
    }

  public:
    cgrid_generator(const std::vector<block_id> &grid) : _back(grid.size(), block_id::EMPTY) {}
    inline void copy(std::vector<block_id> &grid) const
    {
        // Parallelize on copying buffers
        const auto work = [this, &grid](std::mt19937 &gen, const size_t i) {
            grid[i] = _back[i];
        };

        // Convert cells to mesh in parallel
        work_queue::worker.run(work, 0, grid.size());
    }
    void generate_world(std::vector<block_id> &grid, const size_t scale, const size_t chunk_size,
                        const std::function<size_t(const std::tuple<size_t, size_t, size_t> &)> &grid_key_unpack,
                        const std::function<min::vec3<float>(const size_t)> &grid_cell_center)
    {
        // Wake up the threads for processing
        work_queue::worker.wake();

        // Create random number generator
        std::mt19937 gen(std::chrono::high_resolution_clock::now().time_since_epoch().count());

        // Calculates perlin noise
        kernel::terrain_base base(scale, chunk_size, 0, scale / 2);
        base.generate(work_queue::worker, _back);

        // Calculates a height map
        kernel::terrain_height height(scale, scale / 2, scale - 1);
        height.generate(work_queue::worker, gen, _back);

        // Copy data from back to front buffer
        copy(grid);

        // Put the threads back to sleep
        work_queue::worker.sleep();
    }
    void generate_portal(std::vector<block_id> &grid, const size_t scale, const size_t chunk_size,
                         const std::function<size_t(const std::tuple<size_t, size_t, size_t> &)> &grid_key_unpack,
                         const std::function<min::vec3<float>(const size_t)> &grid_cell_center)
    {
        // Wake up the threads for processing
        work_queue::worker.wake();

        // Count active cells in grid
        size_t count = 0;

        // Calculate total grid volume
        const float inv_scale3 = 1.0 / (scale * scale * scale);

        // Choose between terrain generators
        std::mt19937 gen(std::chrono::high_resolution_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<int> choose(1, 3);
        const int type = choose(gen);
        if (type == 1)
        {
            // Filter all dead worlds less than 20% filled
            while (count * inv_scale3 < 0.20)
            {
                // Clear out the old grid
                clear_grid(grid);

                // generate mandelbulb world using mandelbulb generator
                kernel::mandelbulb_sym(gen).generate(work_queue::worker, grid, scale, [grid_cell_center](const size_t i) {
                    return grid_cell_center(i);
                });

                // Update count
                count = count_grid(grid);
            }
        }
        if (type == 2)
        {
            // Filter all dead worlds less than 20% filled
            while (count * inv_scale3 < 0.20)
            {
                // Clear out the old grid
                clear_grid(grid);

                // generate mandelbulb world using mandelbulb generator
                kernel::mandelbulb_asym(gen).generate(work_queue::worker, grid, scale, [grid_cell_center](const size_t i) {
                    return grid_cell_center(i);
                });

                // Update count
                count = count_grid(grid);
            }
        }
        else
        {
            // Filter all dead worlds less than 10% filled
            while (count * inv_scale3 < 0.10)
            {
                // Clear out the old grid
                clear_grid(grid);

                // generate mandelbulb world using mandelbulb generator
                kernel::mandelbulb_exp(gen).generate(work_queue::worker, grid, scale, [grid_cell_center](const size_t i) {
                    return grid_cell_center(i);
                });

                // Update count
                count = count_grid(grid);
            }
        }

        // Put the threads back to sleep
        work_queue::worker.sleep();
    }
};
}

#endif
