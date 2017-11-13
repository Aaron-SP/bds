/* Copyright [2013-2016] [Aaron Springstroh, Minimal Graphics Library]

This file is part of the MGLCraft.

MGLCraft is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MGLCraft is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MGLCraft.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __CHUNK_GRID__
#define __CHUNK_GRID__

#include <game/file.h>
#include <game/mandelbulb.h>
#include <min/aabbox.h>
#include <min/convert.h>
#include <min/intersect.h>
#include <min/mesh.h>
#include <min/ray.h>
#include <min/serial.h>

namespace game
{

class cgrid
{
  private:
    size_t _grid_size;
    std::vector<int8_t> _grid;
    size_t _chunk_size;
    size_t _chunk_scale;
    std::vector<min::mesh<float, uint32_t>> _chunks;
    std::vector<size_t> _surrounding_chunks;
    size_t _recent_chunk;
    size_t _view_chunk_size;
    min::aabbox<float, min::vec3> _world;
    int8_t _atlas_id;

    static inline void cubic(const min::vec3<float> &start, const min::vec3<int> &offset, const min::vec3<unsigned> &length, const std::function<void(const min::vec3<float> &)> &f)
    {
        // Begin at start position
        min::vec3<float> p = start;

        // x axis
        for (size_t i = 0; i < length.x(); i++)
        {
            // y axis
            p.y(start.y());
            for (size_t j = 0; j < length.y(); j++)
            {
                // z axis
                p.z(start.z());
                for (size_t k = 0; k < length.z(); k++)
                {
                    // Do function for cubic space
                    f(p);

                    // Increment z axis
                    p.z(p.z() + offset.z());
                }

                // Increment y axis
                p.y(p.y() + offset.y());
            }

            // Increment x axis
            p.x(p.x() + offset.x());
        }
    }
    static inline min::mesh<float, uint32_t> create_box_mesh(const min::aabbox<float, min::vec3> &box, const int8_t atlas_id)
    {
        min::mesh<float, uint32_t> box_mesh = min::to_mesh<float, uint32_t>(box);

        // grass
        if (atlas_id == 5)
        {
            for (auto &uv : box_mesh.uv)
            {
                uv *= 0.248;
                uv.x(uv.x() + 0.001);
                uv.y(uv.y() + 0.751);
            }
        }
        // stone
        else if (atlas_id == 0)
        {
            for (auto &uv : box_mesh.uv)
            {
                uv *= 0.248;
                uv.x(uv.x() + 0.251);
                uv.y(uv.y() + 0.751);
            }
        }
        // sand
        else if (atlas_id == 1)
        {
            for (auto &uv : box_mesh.uv)
            {
                uv *= 0.248;
                uv.x(uv.x() + 0.501);
                uv.y(uv.y() + 0.751);
            }
        }
        // wood
        else if (atlas_id == 3)
        {
            for (auto &uv : box_mesh.uv)
            {
                uv *= 0.248;
                uv += 0.751;
            }
        }
        // dirt
        else if (atlas_id == 4)
        {
            for (auto &uv : box_mesh.uv)
            {
                uv *= 0.248;
                uv.x(uv.x() + 0.001);
                uv.y(uv.y() + 0.501);
            }
        }
        // lava
        else if (atlas_id == 2)
        {
            for (auto &uv : box_mesh.uv)
            {
                uv *= 0.248;
                uv.x(uv.x() + 0.251);
                uv.y(uv.y() + 0.501);
            }
        }
        // water
        else if (atlas_id == 6)
        {
            for (auto &uv : box_mesh.uv)
            {
                uv *= 0.248;
                uv += 0.501;
            }
        }
        // sulphur
        else if (atlas_id == 7)
        {
            for (auto &uv : box_mesh.uv)
            {
                uv *= 0.248;
                uv.x(uv.x() + 0.751);
                uv.y(uv.y() + 0.501);
            }
        }

        return box_mesh;
    }
    static inline min::aabbox<float, min::vec3> create_box(const min::vec3<float> &center)
    {
        // Create box at center
        const min::vec3<float> min = center - min::vec3<float>(0.5, 0.5, 0.5);
        const min::vec3<float> max = center + min::vec3<float>(0.5, 0.5, 0.5);

        // return the box
        return min::aabbox<float, min::vec3>(min, max);
    }
    inline min::vec3<float> chunk_start(const size_t index) const
    {
        if (index >= _chunks.size())
        {
            throw std::runtime_error("cgrid: chunk index is not inside the world");
        }

        // Precalculate the square scale
        const size_t chunk_scale2 = _chunk_scale * _chunk_scale;

        // Calculate row, col and height
        const size_t row = index / chunk_scale2;
        const size_t col = (index - row * chunk_scale2) / _chunk_scale;
        const size_t hei = index - row * chunk_scale2 - col * _chunk_scale;

        // Calculate the center point of the box cell
        const float x = row * _chunk_size + _world.get_min().x() + 0.5;
        const float y = col * _chunk_size + _world.get_min().y() + 0.5;
        const float z = hei * _chunk_size + _world.get_min().z() + 0.5;

        return min::vec3<float>(x, y, z);
    }
    inline void chunk_update(const size_t key)
    {
        // Clear this chunk
        _chunks[key].clear();

        // Get the cell extent
        const min::vec3<float> cell_extent(1.0, 1.0, 1.0);

        // Get cubic function properties
        const min::vec3<float> start = chunk_start(key);
        const min::vec3<int> offset(1, 1, 1);
        const min::vec3<unsigned> length(_chunk_size, _chunk_size, _chunk_size);

        // Create cubic function, for each cell in cubic space
        const auto f = [this, &cell_extent, key](const min::vec3<float> &p) {
            // Check if p is within the grid
            bool is_valid = true;
            const size_t index = grid_key(p, is_valid);
            if (is_valid)
            {
                // If cell is not empty
                const int8_t atlas = _grid[index];
                if (atlas != -1)
                {
                    // Find out if we are on a world edge
                    const size_t edge = _grid_size - 1;
                    const auto t = min::vec3<float>::grid_index(_world.get_min(), cell_extent, p);
                    if (std::get<0>(t) % edge == 0 || std::get<1>(t) % edge == 0 || std::get<2>(t) % edge == 0)
                    {
                        const min::aabbox<float, min::vec3> box = create_box(p);

                        // Create mesh from box
                        const min::mesh<float, uint32_t> box_mesh = create_box_mesh(box, atlas);

                        // Add mesh to chunk
                        _chunks[key].merge(box_mesh);
                    }
                    else
                    {
                        // Get surrounding 6 cells
                        const size_t x1 = grid_key(min::vec3<float>(p.x() - 1.0, p.y(), p.z()), is_valid);
                        const size_t x2 = grid_key(min::vec3<float>(p.x() + 1.0, p.y(), p.z()), is_valid);
                        const size_t y1 = grid_key(min::vec3<float>(p.x(), p.y() - 1.0, p.z()), is_valid);
                        const size_t y2 = grid_key(min::vec3<float>(p.x(), p.y() + 1.0, p.z()), is_valid);
                        const size_t z1 = grid_key(min::vec3<float>(p.x(), p.y(), p.z() - 1.0), is_valid);
                        const size_t z2 = grid_key(min::vec3<float>(p.x(), p.y(), p.z() + 1.0), is_valid);

                        // Check if we got slightly off the edge
                        if (is_valid)
                        {
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
                                const min::aabbox<float, min::vec3> box = create_box(p);

                                // Create mesh from box
                                const min::mesh<float, uint32_t> box_mesh = create_box_mesh(box, atlas);

                                // Add mesh to chunk
                                _chunks[key].merge(box_mesh);
                            }
                        }
                    }
                }
            }
        };

        // Run the function
        cubic(start, offset, length, f);
    }
    inline void generate_world()
    {
        // generate mandelbulb world using mandelbulb generator
        mandelbulb().generate(_grid, _grid_size, [this](const size_t i) {
            return this->grid_center(i);
        });
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
            bool is_valid = true;
            const size_t ckey = chunk_key(p, is_valid);
            if (is_valid)
            {
                this->_surrounding_chunks.push_back(ckey);
            }
        };

        // Run the function
        cubic(start, offset, length, f);
    }
    inline std::tuple<size_t, size_t, size_t> grid_key_unpack(const size_t index) const
    {
        if (index >= _grid.size())
        {
            throw std::runtime_error("cgrid: grid index is not inside the world cell");
        }

        // Precalculate the square scale
        const size_t scale2 = _grid_size * _grid_size;

        // Calculate row, col and height
        const size_t col = index / scale2;
        const size_t row = (index - col * scale2) / _grid_size;
        const size_t hei = index - (col * scale2) - (row * _grid_size);

        // return tuple
        return std::make_tuple(col, row, hei);
    }
    inline min::vec3<float> grid_center(const size_t index) const
    {
        const std::tuple<size_t, size_t, size_t> comp = grid_key_unpack(index);

        // Unpack tuple
        const size_t col = std::get<0>(comp);
        const size_t row = std::get<1>(comp);
        const size_t hei = std::get<2>(comp);

        // Calculate the center point of the box cell
        const float x = col + _world.get_min().x() + 0.5;
        const float y = row + _world.get_min().y() + 0.5;
        const float z = hei + _world.get_min().z() + 0.5;

        return min::vec3<float>(x, y, z);
    }
    inline float grid_center_square_dist(const size_t index, const min::vec3<float> &point) const
    {
        // Calculate vector between points
        const min::vec3<float> dv = grid_center(index) - point;

        // Calculate the square distance to this point
        return dv.dot(dv);
    }
    inline bool inside(const min::vec3<float> &p) const
    {
        const min::vec3<float> &min = _world.get_min();
        const min::vec3<float> &max = _world.get_max();

        // Add error term to prevent breaking out of the world space
        const bool x_inside = (p.x() >= min.x() + 1E-6) && (p.x() <= max.x() - 1E-6);
        const bool y_inside = (p.y() >= min.y() + 1E-6) && (p.y() <= max.y() - 1E-6);
        const bool z_inside = (p.z() >= min.z() + 1E-6) && (p.z() <= max.z() - 1E-6);

        return x_inside && y_inside && z_inside;
    }
    inline bool ray_trace(const min::ray<float, min::vec3> &r, const size_t length, size_t &prev_key, size_t &key) const
    {
        const min::vec3<float> cell_extent(1.0, 1.0, 1.0);

        // Calculate the ray trajectory for tracing in grid
        auto grid_ray = min::vec3<float>::grid_ray(cell_extent, r.get_origin(), r.get_direction(), r.get_inverse());

        // Calculate start point in grid index format
        auto index = min::vec3<float>::grid_index(_world.get_min(), cell_extent, r.get_origin());

        // Trace a ray from origin and stop at first populated cell
        bool is_valid = true;
        prev_key = key = grid_key(r.get_origin(), is_valid);
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
        }

        return is_valid;
    }
    inline void ray_trace_assign(const min::ray<float, min::vec3> &r, const size_t length, std::vector<size_t> &keys)
    {
        const min::vec3<float> cell_extent(1.0, 1.0, 1.0);

        // Calculate the ray trajectory for tracing in grid
        auto grid_ray = min::vec3<float>::grid_ray(cell_extent, r.get_origin(), r.get_direction(), r.get_inverse());

        // Calculate start point in grid index format
        auto index = min::vec3<float>::grid_index(_world.get_min(), cell_extent, r.get_origin());

        // Trace a ray from origin and stop at first populated cell
        bool is_valid = true;
        size_t key = grid_key(r.get_origin(), is_valid);
        if (is_valid)
        {
            // bad flag signals that we have hit the last valid cell
            bool bad_flag = false;
            unsigned count = 0;

            // Skip over first three cells
            while (_grid[key] == -1 && !bad_flag && count < 3)
            {
                // Increment the current key
                key = min::vec3<float>::grid_ray_next(index, grid_ray, bad_flag, _grid_size);
                count++;
            }

            // Record chunks of remaining cells
            while (_grid[key] == -1 && !bad_flag && count < length)
            {
                // Calculate the center position of this cell
                const min::vec3<float> point = grid_center(key);

                // Get the chunk_key of this position for updating
                bool is_valid = true;
                const size_t ckey = chunk_key(point, is_valid);
                if (is_valid)
                {
                    keys.push_back(ckey);
                }

                // Update the grid value with current atlas
                _grid[key] = _atlas_id;

                // Increment the current key
                key = min::vec3<float>::grid_ray_next(index, grid_ray, bad_flag, _grid_size);
                count++;
            }
        }
    }
    inline std::vector<size_t> search(const min::vec3<float> &start, const min::vec3<float> &stop) const
    {
        // Create path from start to stop
        std::vector<size_t> path;

        // Reserve 20 nodes in path
        path.reserve(20);

        // Get grid keys
        bool is_valid = true;
        const size_t start_key = grid_key(start, is_valid);
        const size_t stop_key = grid_key(stop, is_valid);

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
            _grid = min::read_le_vector<int8_t>(stream, next);

            // Check that grid load correctly
            const size_t cubic_size = _grid_size * _grid_size * _grid_size;
            if (_grid.size() == cubic_size)
            {
                // Update all chunks
                const size_t chunks = _chunks.size();
                for (size_t i = 0; i < chunks; i++)
                {
                    chunk_update(i);
                }

                // return before generating world
                return;
            }
        }

        // Else generate world
        generate_world();

        // Update all chunks
        const size_t chunks = _chunks.size();
        for (size_t i = 0; i < chunks; i++)
        {
            chunk_update(i);
        }
    }
    inline void world_save()
    {
        // Create output stream for saving world
        std::vector<uint8_t> stream;

        // Reserve space for grid
        const size_t cubic_size = _grid_size * _grid_size * _grid_size;
        stream.reserve(cubic_size * sizeof(int8_t));

        // Write data into stream
        min::write_le_vector<int8_t>(stream, _grid);

        // Write data to file
        save_file("bin/world.bmesh", stream);
    }

  public:
    cgrid(const size_t grid_size, const size_t chunk_size, const size_t view_chunk_size)
        : _grid_size(2.0 * grid_size),
          _grid(_grid_size * _grid_size * _grid_size, -1),
          _chunk_size(chunk_size),
          _chunk_scale(_grid_size / _chunk_size),
          _chunks(_chunk_scale * _chunk_scale * _chunk_scale, min::mesh<float, uint32_t>("chunk")),
          _recent_chunk(0),
          _view_chunk_size(view_chunk_size),
          _atlas_id(0)
    {
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
    inline min::mesh<float, uint32_t> atlas_box(const min::vec3<float> &p)
    {
        const min::aabbox<float, min::vec3> box = create_box(p);
        return create_box_mesh(box, _atlas_id);
    }
    inline size_t chunk_key(const min::vec3<float> &point, bool &valid) const
    {
        // This function can crash so we need protection
        if (!this->inside(point))
        {
            valid = false;
            return 0;
        }

        // Protect against floating point round off
        const min::vec3<float> p = snap(point);

        // Compute the chunk index from point
        const min::vec3<float> cell_extent(_chunk_size, _chunk_size, _chunk_size);
        return min::vec3<float>::grid_key(_world.get_min(), cell_extent, _chunk_scale, p);
    }
    void create_player_collision_cells(std::vector<min::aabbox<float, min::vec3>> &out, const min::vec3<float> &center) const
    {
        // Surrounding cells
        out.clear();

        // Get cubic function properties
        // Use player snap to get correct y coordinate
        const min::vec3<float> start = snap_player(center) - min::vec3<float>(1.0, 1.5, 1.0);
        const min::vec3<int> offset(1, 1, 1);
        const min::vec3<unsigned> length(3, 4, 3);

        // Create cubic function, for each cell in cubic space
        const auto f = [this, &out](const min::vec3<float> &p) {
            // Calculate grid key index
            bool is_valid = true;
            const size_t key = grid_key(p, is_valid);

            // Check if valid and if the cell is not empty
            if (is_valid && _grid[key] != -1)
            {
                // Create box at this point
                out.push_back(create_box(p));
            }
        };

        // Run the function
        cubic(start, offset, length, f);
    }
    void create_mob_collision_cells(std::vector<min::aabbox<float, min::vec3>> &out, const min::vec3<float> &center) const
    {
        // Surrounding cells
        out.clear();

        // Get cubic function properties
        const min::vec3<float> start = snap(center) - min::vec3<float>(1.0, 1.0, 1.0);
        const min::vec3<int> offset(1, 1, 1);
        const min::vec3<unsigned> length(3, 3, 3);

        // Create cubic function, for each cell in cubic space
        const auto f = [this, &out](const min::vec3<float> &p) {
            // Calculate grid key index
            bool is_valid = true;
            const size_t key = grid_key(p, is_valid);

            // Check if valid and if the cell is not empty
            if (is_valid && _grid[key] != -1)
            {
                // Create box at this point
                out.push_back(create_box(p));
            }
        };

        // Run the function
        cubic(start, offset, length, f);
    }
    inline int8_t get_atlas() const
    {
        return _atlas_id;
    }
    inline void set_atlas(const int8_t id)
    {
        _atlas_id = id;
    }
    inline const min::mesh<float, uint32_t> &get_chunk(const size_t index) const
    {
        return _chunks[index];
    }
    inline size_t get_recent_chunk() const
    {
        return _recent_chunk;
    }
    void get_view_chunks(std::vector<size_t> &out) const
    {
        out.clear();
        out.push_back(_recent_chunk);

        // Get the chunk starting point
        const size_t half_width = (_view_chunk_size / 2) - 1;
        const min::vec3<float> start = chunk_start(_recent_chunk) - min::vec3<float>(_chunk_size, _chunk_size, _chunk_size) * half_width;
        const min::vec3<int> offset(_chunk_size, _chunk_size, _chunk_size);
        const min::vec3<unsigned> length(_view_chunk_size, _view_chunk_size, _view_chunk_size);

        // Create cubic function, for each cell in cubic space
        const auto f = [this, &out](const min::vec3<float> &p) {
            bool is_valid = true;
            const size_t ckey = chunk_key(p, is_valid);
            if (is_valid)
            {
                out.push_back(ckey);
            }
        };

        // Run the function
        cubic(start, offset, length, f);
    }
    inline const min::aabbox<float, min::vec3> &get_world()
    {
        return _world;
    }
    inline size_t grid_key(const min::vec3<float> &point, bool &valid) const
    {
        // This function can crash so we need protection
        if (!this->inside(point))
        {
            valid = false;
            return 0;
        }

        // Protect against floating point round off
        const min::vec3<float> p = snap(point);

        // Compute the grid index from point
        const min::vec3<float> cell_extent(1.0, 1.0, 1.0);
        return min::vec3<float>::grid_key(_world.get_min(), cell_extent, _grid_size, p);
    }
    inline int8_t grid_value(const min::vec3<float> &point) const
    {
        // Lookup grid index from point
        bool is_valid = true;
        const size_t key = grid_key(point, is_valid);
        if (is_valid)
        {
            // Return the atlas id at grid cell
            return _grid[key];
        }

        // Out of bounds
        return -2;
    }
    void ray_trace_atlas(const min::ray<float, min::vec3> &r, const size_t length)
    {
        // Record all modified chunks and store them for updating
        std::vector<size_t> keys;
        keys.reserve(length);

        // Set the geometry along this ray
        ray_trace_assign(r, length, keys);

        // Sort chunk_keys and make the vector unique
        std::sort(keys.begin(), keys.end());
        const auto last = std::unique(keys.begin(), keys.end());

        // Erase empty spaces in vector
        keys.erase(last, keys.end());

        // Update all modified chunks
        for (const auto k : keys)
        {
            chunk_update(k);
        }
    }
    min::vec3<float> ray_trace_last(const min::ray<float, min::vec3> &r, const size_t length) const
    {
        // Trace a ray and return the last key
        size_t prev_key, key;
        const bool is_valid = ray_trace(r, length, prev_key, key);
        if (is_valid)
        {
            // return the snapped point
            return grid_center(key);
        }

        // return ray start point since it is not in the grid
        return r.get_origin();
    }
    min::vec3<float> ray_trace_prev(const min::ray<float, min::vec3> &r, const size_t length) const
    {
        // Trace a ray and return the last key
        size_t prev_key, key;
        const bool is_valid = ray_trace(r, length, prev_key, key);
        if (is_valid)
        {
            // return the snapped point
            return grid_center(prev_key);
        }

        // return ray start point since it is not in the grid
        return r.get_origin();
    }
    void path(std::vector<min::vec3<float>> &out, const min::vec3<float> &start, const min::vec3<float> &stop) const
    {
        // Try to find a path between points
        const std::vector<size_t> path = search(start, stop);
        const size_t size = path.size();

        // Convert keys to points
        out.clear();

        // For all keys
        for (size_t i = 0; i < size; i++)
        {
            out.push_back(grid_center(path[i]));
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
        const auto f = [this, &keys, &out, atlas_id](const min::vec3<float> &p) {
            // Calculate grid key index
            bool is_valid = true;
            const size_t gkey = grid_key(p, is_valid);

            // Count changed blocks
            if (is_valid && _grid[gkey] != atlas_id)
            {
                // Increment the out counter
                out++;

                // Get the chunk_key for updating
                bool is_valid = true;
                const size_t ckey = chunk_key(p, is_valid);
                if (is_valid)
                {
                    keys.push_back(ckey);
                }

                // Set the cell with atlas id
                _grid[gkey] = atlas_id;
            }
        };

        // Run the function
        cubic(start, offset, length, f);

        // If we are removing blocks, add surrounding cells for update
        if (atlas_id == -1)
        {
            bool is_valid = true;
            const size_t ckey = chunk_key(start, is_valid);
            if (is_valid)
            {
                // Get surrounding chunks for updating
                get_surrounding_chunks(ckey);

                // Insert surrounding chunks into keys
                keys.insert(keys.end(), _surrounding_chunks.begin(), _surrounding_chunks.end());
            }
        }

        // Sort chunk_keys and make the vector unique
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
    void update(const min::vec3<float> &p)
    {
        bool is_valid = true;
        const size_t ckey = chunk_key(p, is_valid);
        if (is_valid)
        {
            _recent_chunk = ckey;
        }
    }
    void update(const size_t current_chunk)
    {
        _recent_chunk = current_chunk;
    }
};
}

#endif
