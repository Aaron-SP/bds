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
#ifndef __CHUNK_GRID__
#define __CHUNK_GRID__

#include <chrono>
#include <game/file.h>
#include <game/height_map.h>
#include <game/mandelbulb.h>
#include <game/work_queue.h>
#include <min/aabbox.h>
#include <min/intersect.h>
#include <min/mesh.h>
#include <min/ray.h>
#include <min/serial.h>
#include <random>

namespace game
{

class cgrid
{
  private:
    static constexpr size_t _floor_height = 8;
    size_t _grid_size;
    std::vector<int8_t> _grid;
    size_t _chunk_cells;
    size_t _chunk_size;
    size_t _chunk_scale;
    std::vector<min::mesh<float, uint32_t>> _chunks;
    std::vector<bool> _chunk_update;
    std::vector<size_t> _surrounding_chunks;
    size_t _recent_chunk;
    size_t _view_chunk_size;
    size_t _view_half_width;
    min::aabbox<float, min::vec3> _world;
    min::vec3<float> _cell_extent;
    int8_t _atlas_id;

    static inline bool in_x(const min::vec3<float> &p, const min::vec3<float> &min, const min::vec3<float> &max)
    {
        return (p.x() >= min.x() + 1E-6) && (p.x() <= max.x() - 1E-6);
    }
    static inline bool in_y(const min::vec3<float> &p, const min::vec3<float> &min, const min::vec3<float> &max)
    {
        return (p.y() >= min.y() + 1E-6) && (p.y() <= max.y() - 1E-6);
    }
    static inline bool in_z(const min::vec3<float> &p, const min::vec3<float> &min, const min::vec3<float> &max)
    {
        return (p.z() >= min.z() + 1E-6) && (p.z() <= max.z() - 1E-6);
    }

    inline void cubic(const min::vec3<float> &start, const min::vec3<int> &offset, const min::vec3<unsigned> &length, const std::function<void(const min::vec3<float> &)> &f) const
    {
        // Begin at start position
        min::vec3<float> p = start;

        // Get world extents
        const min::vec3<float> &min = _world.get_min();
        const min::vec3<float> &max = _world.get_max();

        // x axis: will prune points outside grid
        for (size_t i = 0; i < length.x(); i++, p.x(p.x() + offset.x()))
        {
            if (!in_x(p, min, max))
            {
                continue;
            }

            // y axis: will prune points outside grid
            p.y(start.y());
            for (size_t j = 0; j < length.y(); j++, p.y(p.y() + offset.y()))
            {
                if (!in_y(p, min, max))
                {
                    continue;
                }

                // z axis: will prune points outside grid
                p.z(start.z());
                for (size_t k = 0; k < length.z(); k++, p.z(p.z() + offset.z()))
                {
                    if (!in_z(p, min, max))
                    {
                        continue;
                    }

                    // Do function for cubic space
                    f(p);
                }
            }
        }
    }
    inline void cubic_grid(const min::vec3<float> &start, const min::vec3<int> &offset, const min::vec3<unsigned> &length, const std::function<void(const size_t)> &f) const
    {
        // Get world extents
        const min::vec3<float> min = _world.get_min();
        const min::vec3<float> max = _world.get_max();

        // Begin at start position, clamp out of bound to world boundary
        const min::vec3<float> restart = min::vec3<float>(start).clamp(min, max);

        // Get the grid axis components
        const size_t end = _grid_size;
        const auto t = min::vec3<float>::grid_index(_world.get_min(), _cell_extent, restart);

        // x axis: will prune points outside grid
        size_t tx = std::get<0>(t);
        for (size_t i = 0; i < length.x() && tx < end; i++, tx += offset.x())
        {
            // y axis: will prune points outside grid
            size_t ty = std::get<1>(t);
            for (size_t j = 0; j < length.y() && ty < end; j++, ty += offset.y())
            {
                // z axis: will prune points outside grid
                size_t tz = std::get<2>(t);
                for (size_t k = 0; k < length.z() && tz < end; k++, tz += offset.z())
                {
                    // Do function for cubic space
                    f(grid_key_pack(std::make_tuple(tx, ty, tz)));
                }
            }
        }
    }
    static inline min::aabbox<float, min::vec3> create_box(const min::vec3<float> &center)
    {
        // Create box at center
        const min::vec3<float> min = center - min::vec3<float>(0.5, 0.5, 0.5);
        const min::vec3<float> max = center + min::vec3<float>(0.5, 0.5, 0.5);

        // return the box
        return min::aabbox<float, min::vec3>(min, max);
    }
    inline std::tuple<size_t, size_t, size_t> chunk_key_unpack(const size_t key) const
    {
        return min::vec3<float>::grid_index(key, _chunk_scale);
    }
    inline size_t chunk_key_unsafe(const min::vec3<float> &point) const
    {
        // Protect against floating point round off
        const min::vec3<float> p = snap(point);

        // Compute the chunk key from point
        const min::vec3<float> cell_extent(_chunk_size, _chunk_size, _chunk_size);
        return min::vec3<float>::grid_key(_world.get_min(), cell_extent, _chunk_scale, p);
    }
    inline size_t chunk_key_safe(const min::vec3<float> &point, bool &valid) const
    {
        // This function can crash so we need protection
        if (!this->inside(point))
        {
            valid = false;
            return 0;
        }

        return chunk_key_unsafe(point);
    }
    inline min::vec3<float> chunk_start(const size_t key) const
    {
        const std::tuple<size_t, size_t, size_t> comp = chunk_key_unpack(key);

        // Unpack tuple
        const size_t col = std::get<0>(comp);
        const size_t row = std::get<1>(comp);
        const size_t hei = std::get<2>(comp);

        // Calculate the center point of the box cell
        const float x = col * _chunk_size + _world.get_min().x() + 0.5;
        const float y = row * _chunk_size + _world.get_min().y() + 0.5;
        const float z = hei * _chunk_size + _world.get_min().z() + 0.5;

        return min::vec3<float>(x, y, z);
    }
    inline std::tuple<size_t, size_t, size_t> grid_key_unpack(const size_t key) const
    {
        return min::vec3<float>::grid_index(key, _grid_size);
    }
    inline size_t grid_key_pack(const std::tuple<size_t, size_t, size_t> &t) const
    {
        return min::vec3<float>::grid_key(t, _grid_size);
    }
    inline size_t grid_key_unsafe(const min::vec3<float> &point) const
    {
        // Protect against floating point round off
        const min::vec3<float> p = snap(point);

        // Compute the grid key from point
        return min::vec3<float>::grid_key(_world.get_min(), _cell_extent, _grid_size, p);
    }
    inline size_t grid_key_safe(const min::vec3<float> &point, bool &valid) const
    {
        // This function can crash so we need protection
        if (!this->inside(point))
        {
            valid = false;
            return 0;
        }

        return grid_key_unsafe(point);
    }
    inline min::vec3<float> grid_cell(const size_t key) const
    {
        const std::tuple<size_t, size_t, size_t> comp = grid_key_unpack(key);

        // Unpack tuple
        const size_t col = std::get<0>(comp);
        const size_t row = std::get<1>(comp);
        const size_t hei = std::get<2>(comp);

        // Calculate the center point of the box cell
        const float x = col + _world.get_min().x();
        const float y = row + _world.get_min().y();
        const float z = hei + _world.get_min().z();

        return min::vec3<float>(x, y, z);
    }
    inline min::vec3<float> grid_cell_center(const size_t key) const
    {
        return grid_cell(key) + 0.5;
    }
    inline void chunk_update(const size_t chunk_key)
    {
        // Clear this chunk
        _chunks[chunk_key].clear();

        // Create cubic function, for each cell in cubic space
        const auto f = [this, chunk_key](const size_t key) {

            // cell should always be in the chunk, if not empty
            const int8_t atlas = _grid[key];
            if (atlas != -1)
            {
                // Find out if we are on a world edge
                const auto t = grid_key_unpack(key);
                const size_t tx = std::get<0>(t);
                const size_t ty = std::get<1>(t);
                const size_t tz = std::get<2>(t);
                const size_t edge = _grid_size - 1;
                const min::vec3<float> p = grid_cell_center(key);
                if (tx % edge == 0 || ty % edge == 0 || tz % edge == 0)
                {
                    // Push back block, store atlas in w component see vertex/geometry shader
                    _chunks[chunk_key].vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), static_cast<float>(atlas)));
                }
                else
                {
                    // Get surrounding 6 cells unsafely, check if cell is within the grid
                    const size_t x1 = grid_key_pack(std::make_tuple(tx - 1, ty, tz));
                    const size_t x2 = grid_key_pack(std::make_tuple(tx + 1, ty, tz));
                    const size_t y1 = grid_key_pack(std::make_tuple(tx, ty - 1, tz));
                    const size_t y2 = grid_key_pack(std::make_tuple(tx, ty + 1, tz));
                    const size_t z1 = grid_key_pack(std::make_tuple(tx, ty, tz - 1));
                    const size_t z2 = grid_key_pack(std::make_tuple(tx, ty, tz + 1));

                    const bool bx1 = _grid[x1] != -1;
                    const bool bx2 = _grid[x2] != -1;
                    const bool by1 = _grid[y1] != -1;
                    const bool by2 = _grid[y2] != -1;
                    const bool bz1 = _grid[z1] != -1;
                    const bool bz2 = _grid[z2] != -1;

                    // Only generate if not
                    const bool skip = bx1 && bx2 && by1 && by2 && bz1 && bz2;
                    if (!skip)
                    {
                        // Push back block, store atlas in w component see vertex/geometry shader
                        _chunks[chunk_key].vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), static_cast<float>(atlas)));
                    }
                }
            }
        };

        // Flag that the chunk needs to be updated
        _chunk_update[chunk_key] = true;

        // Get cubic function properties
        const min::vec3<float> start = chunk_start(chunk_key);
        const min::vec3<int> offset(1, 1, 1);
        const min::vec3<unsigned> length(_chunk_size, _chunk_size, _chunk_size);

        // Run the function
        cubic_grid(start, offset, length, f);
    }
    inline void chunk_warm(const size_t key)
    {
        _chunks[key].vertex.reserve(_chunk_cells);
        _chunks[key].index.reserve(_chunk_cells);
    }
    inline void generate_world()
    {
        // generate mandelbulb world using mandelbulb generator
        mandelbulb().generate(work_queue::worker, _grid, _grid_size, [this](const size_t i) {
            return this->grid_cell_center(i);
        });

        // Random numbers between -1.0, and 1.0
        std::uniform_int_distribution<int8_t> dist(0, 4);
        std::mt19937 gen(std::chrono::high_resolution_clock::now().time_since_epoch().count());

        // Generate height map
        const size_t level = std::ceil(std::log2(_grid_size));
        const height_map<float, float> map(level, -1.0, 4.0);

        // Parallelize on x-axis
        const auto work = [this, &dist, &gen, &map](const size_t i) {

            // y axis
            for (size_t j = 0; j < _floor_height; j++)
            {
                // z axis
                for (size_t k = 0; k < _grid_size; k++)
                {
                    // Get the height
                    const uint8_t height = static_cast<uint8_t>(std::round(map.get(i, k)));
                    if (j < height)
                    {
                        // Select ground textures
                        switch (dist(gen))
                        {
                        case 0:
                            _grid[grid_key_pack(std::make_tuple(i, j, k))] = 1;
                            break;
                        case 1:
                            _grid[grid_key_pack(std::make_tuple(i, j, k))] = 10;
                            break;
                        case 2:
                            _grid[grid_key_pack(std::make_tuple(i, j, k))] = 13;
                            break;
                        case 3:
                            _grid[grid_key_pack(std::make_tuple(i, j, k))] = 14;
                            break;
                        case 4:
                            _grid[grid_key_pack(std::make_tuple(i, j, k))] = 15;
                            break;
                        }
                    }
                }
            }
        };

        // Run the job in parallel
        work_queue::worker.run(work, 0, _grid_size);
    }
    inline void get_surrounding_chunks(const size_t key)
    {
        _surrounding_chunks.clear();
        _surrounding_chunks.push_back(key);

        // Get cubic function properties
        const min::vec3<float> start = chunk_start(key) - min::vec3<float>(_chunk_size, _chunk_size, _chunk_size);
        const min::vec3<int> offset(_chunk_size, _chunk_size, _chunk_size);
        const min::vec3<unsigned> length(3, 3, 3);

        // Create cubic function, for each cell in cubic space
        const auto f = [this](const min::vec3<float> &p) {
            const size_t ckey = chunk_key_unsafe(p);
            this->_surrounding_chunks.push_back(ckey);
        };

        // Run the function
        cubic(start, offset, length, f);
    }
    inline float grid_center_square_dist(const size_t key, const min::vec3<float> &point) const
    {
        // Calculate vector between points
        const min::vec3<float> dv = grid_cell_center(key) - point;

        // Calculate the square distance to this point
        return dv.dot(dv);
    }
    inline bool inside(const min::vec3<float> &p) const
    {
        const min::vec3<float> &min = _world.get_min();
        const min::vec3<float> &max = _world.get_max();

        return in_x(p, min, max) && in_y(p, min, max) && in_z(p, min, max);
    }
    inline bool ray_trace(const min::ray<float, min::vec3> &r, const size_t length, size_t &prev_key, size_t &key, int8_t &value) const
    {
        // Calculate the ray trajectory for tracing in grid
        auto grid_ray = min::vec3<float>::grid_ray(_cell_extent, r.get_origin(), r.get_direction(), r.get_inverse());

        // Calculate start point in grid index format
        auto index = min::vec3<float>::grid_index(_world.get_min(), _cell_extent, r.get_origin());

        // Trace a ray from origin and stop at first populated cell
        bool is_valid = true;
        prev_key = key = grid_key_safe(r.get_origin(), is_valid);
        if (is_valid)
        {
            // bad flag signals that we have hit the last valid cell
            bool bad_flag = false;
            unsigned count = 0;
            while (_grid[key] == -1 && !bad_flag && count < length)
            {
                // Update the previous key
                prev_key = key;

                // Increment the current key
                key = min::vec3<float>::grid_ray_next(index, grid_ray, bad_flag, _grid_size);
                count++;
            }

            // Allow setting cells on edge of grid
            if (bad_flag)
            {
                prev_key = key;
            }

            // return the stopping cell value
            value = _grid[key];
        }

        return is_valid;
    }
    inline std::vector<size_t> search(const min::vec3<float> &start, const min::vec3<float> &stop) const
    {
        // Create path from start to stop
        std::vector<size_t> path;

        // Reserve 20 nodes in path
        path.reserve(20);

        // Get grid keys
        bool is_valid = true;
        const size_t start_key = grid_key_safe(start, is_valid);
        const size_t stop_key = grid_key_safe(stop, is_valid);

        // If points are not in grid
        if (!is_valid)
        {
            return path;
        }

        // Create integer visit flag grid
        std::vector<int8_t> visit(_grid.size(), -1);

        // Create a stack for storing nodes in DFS
        std::vector<size_t> stack;

        // Flood fill 6 way DFS search
        std::vector<std::pair<size_t, float>> neighbors;
        neighbors.reserve(6);

        // If we need to search
        if (start_key != stop_key)
        {
            // Get component wise keys
            const std::tuple<size_t, size_t, size_t> stop_comp = grid_key_unpack(stop_key);

            // Push the start_key on the stack
            stack.push_back(start_key);

            // Iteratively Search for a path
            while (!stack.empty())
            {
                const bool found = search_next(visit, path, stack, neighbors, stop_comp, stop_key, stop);
                if (found)
                {
                    break;
                }
            }
        }

        // Return the path
        return path;
    }
    inline void search_neighbors(std::vector<std::pair<size_t, float>> &neighbors, const std::tuple<size_t, size_t, size_t> &comp,
                                 const std::tuple<size_t, size_t, size_t> &stop_comp, const min::vec3<float> &stop) const
    {
        // Clear neighbors buffer
        neighbors.clear();

        // Unpack start_key to components
        const size_t x = std::get<0>(comp);
        const size_t y = std::get<1>(comp);
        const size_t z = std::get<2>(comp);

        // Check against lower x grid dimensions
        const size_t edge = _grid_size - 1;
        if (x != 0)
        {
            const size_t nxk = min::vec3<float>::grid_key(std::make_tuple(x - 1, y, z), _grid_size);
            neighbors.push_back({nxk, grid_center_square_dist(nxk, stop)});
        }

        // Check against upper x grid dimensions
        if (x != edge)
        {
            const size_t pxk = min::vec3<float>::grid_key(std::make_tuple(x + 1, y, z), _grid_size);
            neighbors.push_back({pxk, grid_center_square_dist(pxk, stop)});
        }

        // Check against lower y grid dimensions
        if (y != 0)
        {
            const size_t nyk = min::vec3<float>::grid_key(std::make_tuple(x, y - 1, z), _grid_size);
            neighbors.push_back({nyk, grid_center_square_dist(nyk, stop)});
        }

        // Check against upper y grid dimensions
        if (y != edge)
        {
            const size_t pyk = min::vec3<float>::grid_key(std::make_tuple(x, y + 1, z), _grid_size);
            neighbors.push_back({pyk, grid_center_square_dist(pyk, stop)});
        }

        // Check against lower z grid dimensions
        if (z != 0)
        {
            const size_t nzk = min::vec3<float>::grid_key(std::make_tuple(x, y, z - 1), _grid_size);
            neighbors.push_back({nzk, grid_center_square_dist(nzk, stop)});
        }

        // Check against upper z grid dimensions
        if (z != edge)
        {
            const size_t pzk = min::vec3<float>::grid_key(std::make_tuple(x, y, z + 1), _grid_size);
            neighbors.push_back({pzk, grid_center_square_dist(pzk, stop)});
        }

        // lambda function to create sorted array indices based on distance
        std::sort(neighbors.begin(), neighbors.end(), [](const std::pair<size_t, float> &a, const std::pair<size_t, float> &b) {
            return a.second > b.second;
        });
    }
    inline bool search_next(
        std::vector<int8_t> &visit, std::vector<size_t> &path,
        std::vector<size_t> &stack, std::vector<std::pair<size_t, float>> &neighbors,
        const std::tuple<size_t, size_t, size_t> &stop_comp, const size_t stop_key, const min::vec3<float> &stop) const
    {
        // Get element on top of stack
        const size_t key = stack.back();

        // Check if we made it to the mother lands!
        if (key == stop_key)
        {
            // Store end point of path
            path.push_back(key);
            return true;
        }

        // If we haven't seen this node yet, we are traversing
        if (visit[key] == -1)
        {
            path.push_back(key);
        }

        // If we already visited this node, we are unwinding
        if (visit[key] == 0)
        {
            // pop stack
            stack.pop_back();

            // pop path
            path.pop_back();

            // terminate
            return false;
        }

        // If we haven't visited this cell yet, and the next cell isn't a wall or visited
        if (visit[key] == -1)
        {
            // Visit this node
            visit[key] = 0;

            // Search along the gradient until we hit a wall
            const std::tuple<size_t, size_t, size_t> comp = grid_key_unpack(key);

            // Search all neighboring cells
            search_neighbors(neighbors, comp, stop_comp, stop);

            for (const auto &n : neighbors)
            {
                // If we haven't visited the neighbor cell, and it isn't a wall
                if (visit[n.first] == -1 && _grid[n.first] == -1)
                {
                    // Calculate distance to destination
                    stack.push_back(n.first);
                }
            }
        }

        // We failed to find a path :(
        return false;
    }
    inline void world_load()
    {
        // Create output stream for loading world
        std::vector<uint8_t> stream;

        // Load data into stream from file
        load_file("bin/world.bmesh", stream);

        // If load failed dont try to parse stream data
        if (stream.size() != 0)
        {
            // Load grid with file
            size_t next = 0;
            const std::vector<int8_t> grid = min::read_le_vector<int8_t>(stream, next);

            // Check that grid load correctly
            const size_t cubic_size = _grid_size * _grid_size * _grid_size;
            if (_grid.size() == cubic_size)
            {
                // Copy grid from file
                _grid = grid;
            }
            else
            {
                // Grid is wrong dimensions so regenerate world
                generate_world();
            }
        }
        else
        {
            // Else generate world
            generate_world();
        }

        // Reserve and update all chunks
        const size_t chunks = _chunks.size();
        for (size_t i = 0; i < chunks; i++)
        {
            chunk_warm(i);
            chunk_update(i);
        }
    }
    inline void world_save()
    {
        // Create output stream for saving world
        std::vector<uint8_t> stream;

        // Reserve space for grid
        stream.reserve(_grid.size() * sizeof(int8_t));

        // Write data into stream
        min::write_le_vector<int8_t>(stream, _grid);

        // Write data to file
        save_file("bin/world.bmesh", stream);
    }

  public:
    cgrid(const size_t grid_size, const size_t chunk_size, const size_t view_chunk_size)
        : _grid_size(2.0 * grid_size),
          _grid(_grid_size * _grid_size * _grid_size, -1),
          _chunk_cells(chunk_size * chunk_size * chunk_size),
          _chunk_size(chunk_size),
          _chunk_scale(_grid_size / _chunk_size),
          _chunks(_chunk_scale * _chunk_scale * _chunk_scale, min::mesh<float, uint32_t>("chunk")),
          _chunk_update(_chunks.size(), false),
          _recent_chunk(0),
          _view_chunk_size(view_chunk_size),
          _view_half_width(_view_chunk_size / 2),
          _cell_extent(1.0, 1.0, 1.0),
          _atlas_id(0)
    {

        // Check chunk size
        if (grid_size % chunk_size != 0)
        {
            throw std::runtime_error("cgrid: chunk_size must evenly divide grid_size");
        }

        // Check view size
        if (_view_chunk_size % 2 == 0 || _view_chunk_size == 1)
        {
            // View chunk_size is not symmetric or greater than one
            throw std::runtime_error("cgrid: view_chunk_size must be an odd number of cells, greater than one");
        }
        else if (_view_half_width >= _chunk_scale)
        {
            // View half width is larger than chunk grid dimension
            throw std::runtime_error("cgrid: view_chunk_size can't be greater than " + std::to_string(_chunk_scale * 2 + 1));
        }

        // Create world AABB with a little border for protection
        const float max = static_cast<float>(grid_size);
        const float min = (max * -1.0);
        min::vec3<float> minv(min, min, min);
        min::vec3<float> maxv(max, max, max);
        _world = min::aabbox<float, min::vec3>(minv, maxv);

        // Add starting blocks to simulation
        world_load();

        // Reserve space for surrounding chunks
        _surrounding_chunks.reserve(27);
    }
    ~cgrid()
    {
        // Save the world state in file
        world_save();
    }
    static inline min::vec3<float> snap(const min::vec3<float> &point)
    {
        const float x = point.x();
        const float y = point.y();
        const float z = point.z();

        return min::vec3<float>(std::floor(x) + 0.5, std::floor(y) + 0.5, std::floor(z) + 0.5);
    }
    static inline min::vec3<float> snap_player(const min::vec3<float> &position)
    {
        const float x = position.x();
        const float y = position.y();
        const float z = position.z();

        return min::vec3<float>(std::floor(x) + 0.5, std::round(y), std::floor(z) + 0.5);
    }
    inline void atlas_preview(min::mesh<float, uint32_t> &mesh, const min::vec3<int> &offset, const min::vec3<unsigned> &length)
    {
        // Clear mesh contents
        mesh.vertex.clear();
        mesh.index.clear();

        // Store start point => (0,0,0)
        min::vec3<float> start;
        const float atlas = static_cast<float>(_atlas_id);

        // Create cubic function, for each cell in cubic space
        const auto f = [this, &mesh, &atlas](const size_t key) {

            // Add data to mesh for each cell
            const min::vec3<float> p = grid_cell(key);
            mesh.vertex.push_back(min::vec4<float>(p.x(), p.y(), p.z(), atlas));
        };

        // Run the function
        cubic_grid(start, offset, length, f);
    }
    inline void create_player_collision_cells(std::vector<std::pair<min::aabbox<float, min::vec3>, int8_t>> &out, const min::vec3<float> &center) const
    {
        // Surrounding cells
        out.clear();

        // Get cubic function properties
        // Use player snap to get correct y coordinate
        const min::vec3<float> start = snap_player(center) - min::vec3<float>(1.0, 1.5, 1.0);
        const min::vec3<int> offset(1, 1, 1);
        const min::vec3<unsigned> length(3, 4, 3);

        // Create cubic function, for each cell in cubic space
        const auto f = [this, &out](const size_t key) {

            // Check if valid and if the cell is not empty
            if (_grid[key] != -1)
            {
                // Create box at this point
                const min::aabbox<float, min::vec3> box = create_box(grid_cell_center(key));

                // Add box and grid value to
                out.emplace_back(box, _grid[key]);
            }
        };

        // Run the function
        cubic_grid(start, offset, length, f);
    }
    inline void create_mob_collision_cells(std::vector<min::aabbox<float, min::vec3>> &out, const min::vec3<float> &center) const
    {
        // Surrounding cells
        out.clear();

        // Get cubic function properties
        const min::vec3<float> start = snap(center) - min::vec3<float>(1.0, 1.0, 1.0);
        const min::vec3<int> offset(1, 1, 1);
        const min::vec3<unsigned> length(3, 3, 3);

        // Create cubic function, for each cell in cubic space
        const auto f = [this, &out](const size_t key) {

            // Check if valid and if the cell is not empty
            if (_grid[key] != -1)
            {
                // Create box at this point
                out.push_back(create_box(grid_cell_center(key)));
            }
        };

        // Run the function
        cubic_grid(start, offset, length, f);
    }
    inline int8_t get_atlas() const
    {
        return _atlas_id;
    }
    inline void set_atlas(const int8_t id)
    {
        _atlas_id = id;
    }
    inline min::mesh<float, uint32_t> &get_chunk(const size_t key)
    {
        return _chunks[key];
    }
    inline size_t get_chunk_size() const
    {
        return _chunks.size();
    }
    inline void get_view_chunks(std::vector<size_t> &out) const
    {
        out.clear();

        // Get the chunk starting point
        const min::vec3<float> start = chunk_start(_recent_chunk) - min::vec3<float>(_chunk_size, _chunk_size, _chunk_size) * _view_half_width;
        const min::vec3<int> offset(_chunk_size, _chunk_size, _chunk_size);
        const min::vec3<unsigned> length(_view_chunk_size, _view_chunk_size, _view_chunk_size);

        // Create cubic function, for each cell in cubic space
        const auto f = [this, &out](const min::vec3<float> &p) {
            const size_t ckey = chunk_key_unsafe(p);
            out.push_back(ckey);
        };

        // Run the function
        cubic(start, offset, length, f);
    }
    inline const min::aabbox<float, min::vec3> &get_world()
    {
        return _world;
    }
    inline min::vec3<float> ray_trace_last(const min::ray<float, min::vec3> &r, const size_t length, int8_t &value) const
    {
        // Trace a ray and return the last key
        size_t prev_key, key;
        const bool is_valid = ray_trace(r, length, prev_key, key, value);
        if (is_valid)
        {
            // return the snapped point
            return grid_cell_center(key);
        }

        // return ray start point since it is not in the grid
        return r.get_origin();
    }
    inline min::vec3<float> ray_trace_prev(const min::ray<float, min::vec3> &r, const size_t length) const
    {
        // Trace a ray and return the last key
        size_t prev_key, key;
        int8_t value = -2;
        const bool is_valid = ray_trace(r, length, prev_key, key, value);
        if (is_valid)
        {
            // return the snapped point
            return grid_cell_center(prev_key);
        }

        // return ray start point since it is not in the grid
        return r.get_origin();
    }
    inline void path(std::vector<min::vec3<float>> &out, const min::vec3<float> &start, const min::vec3<float> &stop) const
    {
        // Try to find a path between points
        const std::vector<size_t> path = search(start, stop);
        const size_t size = path.size();

        // Convert keys to points
        out.clear();

        // For all keys
        for (size_t i = 0; i < size; i++)
        {
            out.push_back(grid_cell_center(path[i]));
        }
    }
    // Modifies the geometry in grid
    unsigned set_geometry(const min::vec3<float> &point, const min::vec3<unsigned> &scale, const min::vec3<int> &offset, const int8_t atlas_id)
    {
        // Modified geometry
        unsigned out = 0;

        // Record all modified chunks and store them for updating
        std::vector<size_t> keys;
        keys.reserve(scale.x() * scale.y() * scale.z());

        // Get cubic function properties
        const min::vec3<float> &start = point;
        const min::vec3<unsigned> &length = scale;

        // Create cubic function, for each cell in cubic space
        const auto f = [this, &keys, &out, atlas_id](const size_t key) {

            // Count changed blocks
            if (_grid[key] != atlas_id)
            {
                // Increment the out counter
                out++;

                // Get the chunk key for updating
                const min::vec3<float> p = grid_cell_center(key);
                const size_t ckey = chunk_key_unsafe(p);
                keys.push_back(ckey);

                // Set the cell with atlas id
                _grid[key] = atlas_id;
            }
        };

        // Run the function
        cubic_grid(start, offset, length, f);

        // If we are removing blocks, add surrounding cells for update
        if (atlas_id == -1)
        {
            bool is_valid = true;
            const size_t ckey = chunk_key_safe(start, is_valid);
            if (is_valid)
            {
                // Get surrounding chunks for updating
                get_surrounding_chunks(ckey);

                // Insert surrounding chunks into keys
                keys.insert(keys.end(), _surrounding_chunks.begin(), _surrounding_chunks.end());
            }
        }

        // Sort chunk keys and make the vector unique
        std::sort(keys.begin(), keys.end());
        const auto last = std::unique(keys.begin(), keys.end());

        // Erase empty spaces in vector
        keys.erase(last, keys.end());

        // Update all modified chunks
        for (const auto k : keys)
        {
            chunk_update(k);
        }

        // Return the number of modified blocks
        return out;
    }
    inline bool is_update_chunk(const size_t chunk_key) const
    {
        return _chunk_update[chunk_key];
    }
    inline void update_chunk(const size_t chunk_key)
    {
        _chunk_update[chunk_key] = false;
    }
    inline void update_current_chunk(const min::vec3<float> &p)
    {
        bool is_valid = true;
        const size_t key = chunk_key_safe(p, is_valid);

        // Maintain that new chunk is valid
        if (is_valid)
        {
            _recent_chunk = key;
        }
    }
};
}

#endif
