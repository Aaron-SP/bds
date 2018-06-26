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
#ifndef __BLOCK_ADDER__
#define __BLOCK_ADDER__

#include <game/cgrid.h>
#include <game/id.h>
#include <min/vec3.h>

namespace game
{

class block_adder
{
  private:
    const size_t _edge;

    inline void set_above(cgrid &grid, const std::tuple<size_t, size_t, size_t> &t, const block_id atlas) const
    {
        const bool on_edge_py = (std::get<1>(t) == _edge);
        if (!on_edge_py)
        {
            const auto above = std::make_tuple(std::get<0>(t), std::get<1>(t) + 1, std::get<2>(t));
            const size_t above_key = grid.get_block_key(above);

            // Only set if empty
            if (grid.get_block_id(above_key) == block_id::EMPTY)
            {
                grid.set_block_id(above_key, atlas);
            }
        }
    }
    inline void interaction(
        cgrid &grid,
        const std::tuple<size_t, size_t, size_t> &placed, const block_id placed_atlas,
        const std::tuple<size_t, size_t, size_t> &old) const
    {
        // Get the neighbor block ID
        const size_t old_key = grid.get_block_key(old);
        const block_id old_atlas = grid.get_block_id(old_key);

        // Lookup interaction
        switch (placed_atlas)
        {
            // Placing grass
        case block_id::GRASS1:
        case block_id::GRASS2:
        {
            switch (old_atlas)
            {
            case block_id::DIRT1:
            {
                const size_t placed_key = grid.get_block_key(placed);
                grid.set_block_id(placed_key, old_atlas);
                set_above(grid, old, placed_atlas);
                break;
            }
            case block_id::DIRT2:
            {
                const size_t placed_key = grid.get_block_key(placed);
                grid.set_block_id(placed_key, old_atlas);
                set_above(grid, old, placed_atlas);
                break;
            }
            default:
                break;
            }
            break;
        }
        // Placing leaf
        case block_id::LEAF1:
        case block_id::LEAF2:
        case block_id::LEAF3:
        case block_id::LEAF4:
        {
            switch (old_atlas)
            {
            case block_id::DIRT1:
            case block_id::WOOD1:
            {
                const size_t placed_key = grid.get_block_key(placed);
                grid.set_block_id(placed_key, block_id::WOOD1);
                set_above(grid, old, placed_atlas);
                set_above(grid, placed, block_id::WOOD1);
                break;
            }
            case block_id::DIRT2:
            case block_id::WOOD2:
            {
                const size_t placed_key = grid.get_block_key(placed);
                grid.set_block_id(placed_key, block_id::WOOD2);
                set_above(grid, old, placed_atlas);
                set_above(grid, placed, block_id::WOOD2);
                break;
            }
            case block_id::CLAY1:
            {
                grid.set_block_id(old_key, block_id::DIRT1);
                break;
            }
            case block_id::CLAY2:
            {
                grid.set_block_id(old_key, block_id::DIRT2);
                break;
            }
            case block_id::STONE1:
            {
                grid.set_block_id(old_key, block_id::STONE3);
                break;
            }
            case block_id::STONE2:
            {
                grid.set_block_id(old_key, block_id::STONE3);
                break;
            }
            default:
                break;
            }
            break;
        }

            // Placing minerals
        case block_id::CALCIUM:
        case block_id::MAGNESIUM:
        {
            switch (old_atlas)
            {
            case block_id::SAND1:
            {
                grid.set_block_id(old_key, block_id::CLAY1);
                break;
            }
            case block_id::SAND2:
            {
                grid.set_block_id(old_key, block_id::CLAY2);
                break;
            }
            default:
                break;
            }
            break;
        }
        case block_id::POTASSIUM:
        {
            switch (old_atlas)
            {
            case block_id::GRASS1:
            {
                grid.set_block_id(old_key, block_id::TOMATO);
                break;
            }
            case block_id::GRASS2:
            {
                grid.set_block_id(old_key, block_id::GREEN_PEPPER);
                break;
            }
            default:
                break;
            }
            break;
        }
        case block_id::IRON:
        {
            switch (old_atlas)
            {
            case block_id::GRASS1:
            {
                grid.set_block_id(old_key, block_id::RED_PEPPER);
                break;
            }
            case block_id::GRASS2:
            {
                grid.set_block_id(old_key, block_id::EGGPLANT);
                break;
            }
            default:
                break;
            }
            break;
        }
        case block_id::SODIUM:
        {
            switch (old_atlas)
            {
            case block_id::CLAY1:
            {
                grid.set_block_id(old_key, block_id::STONE1);
                break;
            }
            case block_id::CLAY2:
            {
                grid.set_block_id(old_key, block_id::STONE2);
                break;
            }
            default:
                break;
            }
            break;
        }
        default:
            break;
        }
    }
    inline void neighbor_interaction(cgrid &grid, const std::tuple<size_t, size_t, size_t> &placed, const block_id placed_atlas) const
    {
        // Extract the placed point index
        const size_t x = std::get<0>(placed);
        const size_t y = std::get<1>(placed);
        const size_t z = std::get<2>(placed);

        // Look at X neighbors
        const bool on_edge_nx = (x == 0);
        const bool on_edge_px = (x == _edge);
        if (!on_edge_nx)
        {
            interaction(grid, placed, placed_atlas, std::make_tuple(x - 1, y, z));
        }
        if (!on_edge_px)
        {
            interaction(grid, placed, placed_atlas, std::make_tuple(x + 1, y, z));
        }

        // Look at Y neighbors
        const bool on_edge_ny = (y == 0);
        const bool on_edge_py = (y == _edge);
        if (!on_edge_ny)
        {
            interaction(grid, placed, placed_atlas, std::make_tuple(x, y - 1, z));
        }
        if (!on_edge_py)
        {
            interaction(grid, placed, placed_atlas, std::make_tuple(x, y + 1, z));
        }

        // Look at Z neighbors
        const bool on_edge_nz = (z == 0);
        const bool on_edge_pz = (z == _edge);
        if (!on_edge_nz)
        {
            interaction(grid, placed, placed_atlas, std::make_tuple(x, y, z - 1));
        }
        if (!on_edge_pz)
        {
            interaction(grid, placed, placed_atlas, std::make_tuple(x, y, z + 1));
        }
    }

  public:
    block_adder(const size_t scale) : _edge(scale - 1) {}

    // Bounded must be in grid or will crash!
    inline void add_block(cgrid &grid, const min::vec3<float> &bounded, const block_id placed_atlas) const
    {
        // Dummy callback
        const auto f = [](const min::vec3<float> &, const block_id) -> void {
        };

        // Place the block
        const min::vec3<unsigned> scale(1, 1, 1);
        const min::vec3<int> preview_offset(1, 1, 1);
        grid.set_geometry(bounded, scale, preview_offset, placed_atlas, f);

        // Assumes bounded is in the grid
        const std::tuple<size_t, size_t, size_t> t = grid.get_grid_index_unsafe(bounded);

        // Do block interaction
        neighbor_interaction(grid, t, placed_atlas);
    }
};
}

#endif
