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
#include <game/memory_map.h>
#include <game/work_queue.h>
#include <kernel/mandelbulb_asym.h>
#include <kernel/mandelbulb_exp.h>
#include <kernel/mandelbulb_sym.h>
#include <kernel/terrain_base.h>
#include <kernel/terrain_height.h>
#include <min/serial.h>
#include <min/strtoken.h>
#include <min/vec3.h>
#include <random>
#include <sstream>

namespace game
{

class cgrid_generator
{
  private:
    std::string _asym;
    std::vector<std::pair<size_t, size_t>> _asym_lines;
    std::string _exp;
    std::vector<std::pair<size_t, size_t>> _exp_lines;
    std::string _sym;
    std::vector<std::pair<size_t, size_t>> _sym_lines;
    std::vector<block_id> _back;
    std::istringstream _ss;
    std::string _line;
    std::mt19937 _gen;

    inline void clear_grid(std::vector<block_id> &grid)
    {
        // Parallelize on copying buffers
        const auto work = [&grid](std::mt19937 &gen, const size_t i) {
            grid[i] = block_id::EMPTY;
        };

        // Convert cells to mesh in parallel
        work_queue::worker.run(std::cref(work), 0, grid.size());
    }
    inline void clear_stream(const std::string &str)
    {
        _ss.clear();
        _ss.str(str);
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
    inline kernel::mandelbulb_asym load_mandelbulb_asym(std::mt19937 &gen)
    {
        // Load uniform distribution for sym
        std::uniform_int_distribution<unsigned> dist(0, _asym_lines.size() - 1);
        const unsigned index = dist(gen);

        // Get the line index and get the line string
        const auto &p = _asym_lines[index];
        _line = _asym.substr(p.first, p.second);

        // Clear and reset the stream with line data
        clear_stream(_line);

        // Parse the string into four ints
        int a, b, c, d, e, f, g, h, i, j, k, l;
        _ss >> a >> b >> c >> d >> e >> f >> g >> h >> i >> j >> k >> l;

        // Check for errors
        if (_ss.fail())
        {
            throw std::runtime_error("cgrid_generator: Invalid man_asym line '" + _line + "'");
        }

        // Load the asymmetrical mandelbulb
        return kernel::mandelbulb_asym(a, b, c, d, e, f, g, h, i, j, k, l);
    }
    inline kernel::mandelbulb_exp load_mandelbulb_exp(std::mt19937 &gen)
    {
        // Load uniform distribution for exp
        std::uniform_int_distribution<unsigned> dist(0, _exp_lines.size() - 1);
        const unsigned index = dist(gen);

        // Get the line index and get the line string
        const auto &p = _exp_lines[index];
        _line = _exp.substr(p.first, p.second);

        // Clear and reset the stream with line data
        clear_stream(_line);

        // Parse the string into four ints
        int a, b, c, d;
        _ss >> a >> b >> c >> d;

        // Check for errors
        if (_ss.fail())
        {
            throw std::runtime_error("cgrid_generator: Invalid man_exp line '" + _line + "'");
        }

        // Load the exponential mandelbulb
        return kernel::mandelbulb_exp(a, b, c, d);
    }
    inline kernel::mandelbulb_sym load_mandelbulb_sym(std::mt19937 &gen)
    {
        // Load uniform distribution for sym
        std::uniform_int_distribution<unsigned> dist(0, _sym_lines.size() - 1);
        const unsigned index = dist(gen);

        // Get the line index and get the line string
        const auto &p = _sym_lines[index];
        _line = _sym.substr(p.first, p.second);

        // Clear and reset the stream with line data
        clear_stream(_line);

        // Parse the string into four ints
        int a, b, c, d;
        _ss >> a >> b >> c >> d;

        // Check for errors
        if (_ss.fail())
        {
            throw std::runtime_error("cgrid_generator: Invalid man_sym line '" + _line + "'");
        }

        // Load the symmetrical mandelbulb
        return kernel::mandelbulb_sym(a, b, c, d);
    }
    inline void load_portal_strings()
    {
        // Load the portal files
        _asym = memory_map::memory.get_file("data/portals/man_asym.portal").to_string();
        _asym_lines = min::read_lines(_asym, 1001);
        _exp = memory_map::memory.get_file("data/portals/man_exp.portal").to_string();
        _exp_lines = min::read_lines(_exp, 738);
        _sym = memory_map::memory.get_file("data/portals/man_sym.portal").to_string();
        _sym_lines = min::read_lines(_sym, 1001);
    }

  public:
    cgrid_generator(const std::vector<block_id> &grid)
        : _back(grid.size(), block_id::EMPTY),
          _gen(std::chrono::high_resolution_clock::now().time_since_epoch().count())
    {
        // Load the portal strings
        load_portal_strings();
    }
    inline void copy(std::vector<block_id> &grid) const
    {
        // Parallelize on copying buffers
        const auto work = [this, &grid](std::mt19937 &gen, const size_t i) {
            grid[i] = _back[i];
        };

        // Convert cells to mesh in parallel
        work_queue::worker.run(std::cref(work), 0, grid.size());
    }
    void generate_world(std::vector<block_id> &grid, const size_t scale, const size_t chunk_size)
    {
        // Reseed the generator
        work_queue::worker.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());

        // Wake up the threads for processing
        work_queue::worker.wake();

        // Clear out the old grid
        clear_grid(_back);

        // Calculates perlin noise
        kernel::terrain_base base(scale, chunk_size, 0, scale / 2);
        base.generate(work_queue::worker, _back);

        // Calculates a height map
        kernel::terrain_height height(scale, scale / 2, scale - 1);
        height.generate(work_queue::worker, _gen, _back);

        // Copy data from back to front buffer
        copy(grid);

        // Put the threads back to sleep
        work_queue::worker.sleep();
    }
    template <typename F, typename G>
    void generate_portal(std::vector<block_id> &grid, const size_t scale, const size_t chunk_size,
                         const F &grid_key_unpack, const G &grid_cell_center)
    {
        // Reseed the generator
        work_queue::worker.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());

        // Wake up the threads for processing
        work_queue::worker.wake();

        // Clear out the old grid
        clear_grid(grid);

        // Choose between terrain generators
        std::uniform_int_distribution<int> choose(1, 3);
        const int type = choose(_gen);
        if (type == 1)
        {
            // Generate mandelbulb world using mandelbulb generator
            load_mandelbulb_sym(_gen).generate(work_queue::worker, grid, scale, [grid_cell_center](const size_t i) {
                return grid_cell_center(i);
            });
        }
        if (type == 2)
        {
            // Generate mandelbulb world using mandelbulb generator
            load_mandelbulb_asym(_gen).generate(work_queue::worker, grid, scale, [grid_cell_center](const size_t i) {
                return grid_cell_center(i);
            });
        }
        else
        {
            // Generate mandelbulb world using mandelbulb generator
            load_mandelbulb_exp(_gen).generate(work_queue::worker, grid, scale, [grid_cell_center](const size_t i) {
                return grid_cell_center(i);
            });
        }

        // Put the threads back to sleep
        work_queue::worker.sleep();
    }
};
}

#endif
