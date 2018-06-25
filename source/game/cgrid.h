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
#ifndef __CHUNK_GRID__
#define __CHUNK_GRID__

#include <chrono>
#include <game/callback.h>
#include <game/cgrid_generator.h>
#include <game/file.h>
#include <game/id.h>
#include <game/swatch.h>
#include <game/terrain_mesher.h>
#include <min/aabbox.h>
#include <min/camera.h>
#include <min/intersect.h>
#include <min/mesh.h>
#include <min/ray.h>
#include <min/serial.h>
#include <min/utility.h>
#include <stdexcept>

namespace game
{

class view_chunk
{
  private:
    size_t _index;
    size_t _key;
    min::aabbox<float, min::vec3> _box;
    float _dist;

  public:
    view_chunk(const size_t index, const size_t key, const min::aabbox<float, min::vec3> &box, const float dist)
        : _index(index), _key(key), _box(box), _dist(dist) {}

    const min::aabbox<float, min::vec3> &get_box() const
    {
        return _box;
    }
    float get_dist() const
    {
        return _dist;
    }
    size_t get_key() const
    {
        return _key;
    }
    size_t get_index() const
    {
        return _index;
    }
};

class cgrid
{
  private:
    constexpr static size_t _search_limit = 20;
    const size_t _grid_scale;
    std::vector<block_id> _grid;
    std::vector<int_fast8_t> _visit;
    std::vector<std::pair<size_t, float>> _neighbors;
    std::vector<size_t> _path;
    std::vector<size_t> _stack;
    const size_t _chunk_cells;
    const size_t _chunk_size;
    const size_t _chunk_scale;
    std::vector<min::mesh<float, uint32_t>> _chunks;
    std::vector<bool> _chunk_update;
    std::vector<size_t> _chunk_update_keys;
    std::vector<size_t> _sort_chunk;
    std::vector<view_chunk> _view_chunks;
    mutable std::vector<size_t> _overlap;
    size_t _recent_chunk;
    min::vec3<float> _recent_p;
    const size_t _view_chunk_size;
    const size_t _view_half_width;
    const float _view_dist;
    const min::aabbox<float, min::vec3> _world;
    const min::vec3<float> _cell_extent;
    cgrid_generator _generator;
    terrain_mesher _mesher;

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
    inline float calculate_view_distance() const
    {
        // Calculate view half width chunk size
        const float half_width = _chunk_size * _view_half_width;

        // Calculate distance of largest distance vector
        const min::vec3<float> v(half_width, half_width, half_width);

        // Return view distance
        return v.magnitude();
    }
    inline min::aabbox<float, min::vec3> calculate_world_size(const size_t grid_scale)
    {
        // Create world AABB with a little border for protection
        const float max = static_cast<float>(grid_scale);
        const float min = (max * -1.0);

        // Get box edges
        const min::vec3<float> minv(min, min, min);
        const min::vec3<float> maxv(max, max, max);

        // Return world size
        return min::aabbox<float, min::vec3>(minv, maxv);
    }
    inline void collision_cells(std::vector<std::pair<min::aabbox<float, min::vec3>, block_id>> &out,
                                const min::aabbox<float, min::vec3> &box, const min::vec3<float> &center) const
    {
        // Get all overlapping cells
        min::vec3<float>::grid_overlap(_overlap, _world.get_min(), _cell_extent, _grid_scale, box.get_min(), box.get_max());

        // Create boxes of all overlapping cells
        const size_t size = _overlap.size();
        for (size_t i = 0; i < size; i++)
        {
            // Get the cell key
            const size_t key = _overlap[i];

            // Check if valid and if the cell is not empty
            if (_grid[key] != block_id::EMPTY)
            {
                // Create box at this point
                const min::aabbox<float, min::vec3> grid = grid_box(grid_cell_center(key));

                // Add box and grid value to
                out.emplace_back(grid, _grid[key]);
            }
        }
    }
    template <typename F>
    inline void cubic(const min::vec3<float> &start, const min::vec3<unsigned> &length, const min::vec3<int> &offset, const F &f) const
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
    template <typename F>
    inline void cubic_grid(const min::vec3<float> &start, const min::vec3<unsigned> &length, const min::vec3<int> &offset, const F &f) const
    {
        // Get world extents
        const min::vec3<float> min = _world.get_min();
        const min::vec3<float> max = _world.get_max();

        // Begin at start position, clamp out of bound to world boundary
        const min::vec3<float> bounded = min::vec3<float>(start).clamp(min, max);

        // Get the grid axis components
        const size_t end = _grid_scale;
        const auto t = min::vec3<float>::grid_index(_world.get_min(), _cell_extent, bounded);

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
                    f(i, j, k, grid_key_pack(std::make_tuple(tx, ty, tz)));
                }
            }
        }
    }
    inline min::aabbox<float, min::vec3> create_chunk_box(const min::vec3<float> &chunk_start) const
    {
        // Create box at chunk_start
        const min::vec3<float> min = chunk_start - min::vec3<float>(0.5, 0.5, 0.5);
        const min::vec3<float> max = min + min::vec3<float>(_chunk_size, _chunk_size, _chunk_size);

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
    inline min::vec3<float> chunk_center(const size_t key) const
    {
        const std::tuple<size_t, size_t, size_t> comp = chunk_key_unpack(key);

        // Adjust to middle of chunk
        const float col = std::get<0>(comp) + 0.5;
        const float row = std::get<1>(comp) + 0.5;
        const float hei = std::get<2>(comp) + 0.5;

        // Calculate the center of the chunk in world space
        const float x = col * _chunk_size + _world.get_min().x();
        const float y = row * _chunk_size + _world.get_min().y();
        const float z = hei * _chunk_size + _world.get_min().z();

        // Center not guaranteed to be in middle of box!
        return min::vec3<float>(x, y, z);
    }
    inline min::vec3<float> chunk_start(const size_t key) const
    {
        const std::tuple<size_t, size_t, size_t> comp = chunk_key_unpack(key);

        // Unpack tuple
        const size_t col = std::get<0>(comp);
        const size_t row = std::get<1>(comp);
        const size_t hei = std::get<2>(comp);

        // Calculate the bottom left corner of chunk in world space
        const float x = col * _chunk_size + _world.get_min().x() + 0.5;
        const float y = row * _chunk_size + _world.get_min().y() + 0.5;
        const float z = hei * _chunk_size + _world.get_min().z() + 0.5;

        return min::vec3<float>(x, y, z);
    }
    inline size_t grid_key_pack(const std::tuple<size_t, size_t, size_t> &t) const
    {
        return min::vec3<float>::grid_key(t, _grid_scale);
    }
    inline std::tuple<size_t, size_t, size_t> grid_key_unpack(const size_t key) const
    {
        return min::vec3<float>::grid_index(key, _grid_scale);
    }
    inline size_t grid_key_unsafe(const min::vec3<float> &point) const
    {
        // Protect against floating point round off
        const min::vec3<float> p = snap(point);

        // Compute the grid key from point
        return min::vec3<float>::grid_key(_world.get_min(), _cell_extent, _grid_scale, p);
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
    inline min::vec3<float> grid_cell(const std::tuple<size_t, size_t, size_t> &comp) const
    {
        // Unpack tuple
        const size_t col = std::get<0>(comp);
        const size_t row = std::get<1>(comp);
        const size_t hei = std::get<2>(comp);

        // Calculate the bottom left corner of the box cell
        const float x = col + _world.get_min().x();
        const float y = row + _world.get_min().y();
        const float z = hei + _world.get_min().z();

        return min::vec3<float>(x, y, z);
    }
    inline min::vec3<float> grid_cell(const size_t key) const
    {
        return grid_cell(grid_key_unpack(key));
    }
    inline min::vec3<float> grid_cell_center(const size_t key) const
    {
        return grid_cell(key) + 0.5;
    }
    inline min::vec3<float> grid_cell_center(const std::tuple<size_t, size_t, size_t> &comp) const
    {
        return grid_cell(comp) + 0.5;
    }
    inline void chunk_update(const size_t chunk_key)
    {
        // Clear the mesher
        _mesher.clear();

        // Get the last valid cell on each grid dimension
        const size_t edge = _grid_scale - 1;
        const auto edges = std::make_tuple(edge, edge, edge);

        // Begin at start position, clamp out of bound to world boundary
        const min::vec3<float> start = min::vec3<float>(chunk_start(chunk_key)).clamp(_world.get_min(), _world.get_max());

        // Get the grid axis components
        const auto t = min::vec3<float>::grid_index(_world.get_min(), _cell_extent, start);
        const size_t xend = std::min(std::get<0>(t) + _chunk_size, _grid_scale);
        const size_t yend = std::min(std::get<1>(t) + _chunk_size, _grid_scale);
        const size_t zend = std::min(std::get<2>(t) + _chunk_size, _grid_scale);

        // Function to retrieve block value
        const auto get_block = [this](const std::tuple<size_t, size_t, size_t> &t) -> block_id {
            return _grid[this->grid_key_pack(t)];
        };

        // Iterate through the chunk
        for (size_t tx = std::get<0>(t); tx < xend; tx++)
        {
            for (size_t ty = std::get<1>(t); ty < yend; ty++)
            {
                for (size_t tz = std::get<2>(t); tz < zend; tz++)
                {
                    // Get the cell index
                    const auto index = std::make_tuple(tx, ty, tz);

                    // Get the current cell index value
                    const block_id atlas = get_block(index);
                    if (atlas != block_id::EMPTY)
                    {
                        // Get the cell center point
                        const min::vec3<float> p = grid_cell_center(index);

                        // Generate cell faces
                        _mesher.generate_chunk_faces(p, index, edges, get_block, static_cast<float>(atlas));
                    }
                }
            }
        }

        // Generate mesh
        _mesher.generate_chunk(_chunks[chunk_key]);

        // Flag that the chunk needs to be updated
        _chunk_update[chunk_key] = true;
    }
    inline void chunk_warm(const size_t key)
    {
#ifdef MGL_GS_RENDER
        _chunks[key].vertex.reserve(_chunk_cells);
#else
        const size_t size6 = _chunk_cells * 6;
        _chunks[key].vertex.reserve(size6);
        _chunks[key].uv.reserve(size6);
        _chunks[key].normal.reserve(size6);
#endif
    }
    inline unsigned geometry_add(const min::vec3<float> &start, const min::vec3<unsigned> &length,
                                 const min::vec3<int> &offset, const block_id atlas_id)
    {
        unsigned out = 0;

        // Create cubic function, for each cell in cubic space
        const auto f = [this, &out, atlas_id](const size_t i, const size_t j, const size_t k, const size_t key) {
            // Count changed blocks
            if (_grid[key] != atlas_id)
            {
                // Increment the out counter
                out++;

                // Set the geometry cell with value
                geometry_set_cell(key, atlas_id);
            }
        };

        // Run the function
        cubic_grid(start, length, offset, f);

        // Return count
        return out;
    }
    inline unsigned geometry_copy_swatch(const swatch &sw, const min::vec3<float> &start, const min::vec3<unsigned> &length, const min::vec3<int> &offset)
    {
        unsigned out = 0;

        // Create cubic function, for each cell in cubic space
        const auto f = [this, &out, &sw](const size_t i, const size_t j, const size_t k, const size_t key) {
            // Count changed blocks
            const block_id value = sw.get(i, j, k);
            if (_grid[key] != value)
            {
                // Increment the out counter
                out++;

                // Set the geometry cell with value
                geometry_set_cell(key, value);

                // If we are removing blocks, add boundary cells for update
                if (value == block_id::EMPTY)
                {
                    set_boundary_chunk(key);
                }
            }
        };

        // Run the function
        cubic_grid(start, length, offset, f);

        // Return count
        return out;
    }
    template <typename SB>
    inline unsigned geometry_remove(const min::vec3<float> &start, const min::vec3<unsigned> &length, const min::vec3<int> &offset,
                                    const block_id atlas_id, const SB &set_block_call)
    {
        unsigned out = 0;

        // Create cubic function, for each cell in cubic space
        const auto f = [this, &out, atlas_id, &set_block_call](const size_t i, const size_t j, const size_t k, const size_t key) {
            // Get the old value
            const block_id old_value = _grid[key];

            // Count changed blocks
            if (_grid[key] != atlas_id)
            {
                // Increment the out counter
                out++;

                // Set the geometry cell with value
                const min::vec3<float> p = geometry_set_cell(key, atlas_id);

                // If we are removing blocks, add boundary cells for update
                set_boundary_chunk(key);

                // Callback on cell
                set_block_call(p, old_value);
            }
        };

        // Run the function
        cubic_grid(start, length, offset, f);

        // Return count
        return out;
    }
    inline min::vec3<float> geometry_set_cell(const size_t key, const block_id value)
    {
        // Get the chunk key for updating
        const min::vec3<float> p = grid_cell_center(key);
        const size_t ckey = chunk_key_unsafe(p);
        _chunk_update_keys.push_back(ckey);

        // Set the cell with value
        _grid[key] = value;

        // Return position
        return p;
    }
    inline void generate_portal()
    {
        // Function for finding grid key index
        const auto f = [this](const std::tuple<size_t, size_t, size_t> &t) -> size_t {
            return grid_key_pack(t);
        };

        // Function for finding grid center
        const auto g = [this](const size_t key) -> min::vec3<float> {
            return grid_cell_center(key);
        };

        // Generate the cgrid data
        _generator.generate_portal(_grid, _grid_scale, _chunk_size, f, g);
    }
    inline void generate_world()
    {
        // Generate the cgrid data
        _generator.generate_world(_grid, _grid_scale, _chunk_size);
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
    inline bool ray_trace(const min::ray<float, min::vec3> &r, const size_t length, size_t &prev_key, size_t &key, block_id &value) const
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
            while (_grid[key] == block_id::EMPTY && !bad_flag && count < length)
            {
                // Update the previous key
                prev_key = key;

                // Increment the current key
                key = min::vec3<float>::grid_ray_next(index, grid_ray, bad_flag, _grid_scale);
                count++;
            }

            // return the stopping cell value
            value = _grid[key];
        }

        return is_valid;
    }
    inline void reserve_memory()
    {
        _path.reserve(20);
        _neighbors.reserve(6);
        _stack.reserve(100);
        _sort_chunk.reserve(27);
        _view_chunks.reserve(27);
    }
    inline void reset()
    {
        // Clear out all vectors
        _neighbors.clear();
        _path.clear();
        _stack.clear();
        _chunk_update_keys.clear();
        _sort_chunk.clear();
        _view_chunks.clear();
    }
    inline void search(const min::vec3<float> &start, const min::vec3<float> &stop)
    {
        // Get grid keys
        bool is_valid = true;
        const size_t start_key = grid_key_safe(start, is_valid);
        const size_t stop_key = grid_key_safe(stop, is_valid);

        // If points are not in grid
        if (!is_valid)
        {
            return;
        }

        // Clear the old neighbors
        _neighbors.clear();

        // Clear the old path
        _path.clear();

        // Clear the old stack
        _stack.clear();

        // If the start key is inside terrain
        if (_grid[start_key] != block_id::EMPTY)
        {
            return;
        }

        // Reset the visited flag
        std::fill(_visit.begin(), _visit.end(), -1);

        // If we need to search
        if (start_key != stop_key)
        {
            // Push the start_key on the stack
            _stack.push_back(start_key);

            // Flag that we pushed this key
            _visit[start_key] = 1;

            // Iteratively Search for a path
            while (!_stack.empty())
            {
                const bool found = search_next(stop, stop_key);
                if (found)
                {
                    break;
                }
            }
        }
    }
    inline void search_neighbors(const std::tuple<size_t, size_t, size_t> &comp, const min::vec3<float> &stop)
    {
        // Clear neighbors buffer
        _neighbors.clear();

        // Unpack start_key to components
        const size_t x = std::get<0>(comp);
        const size_t y = std::get<1>(comp);
        const size_t z = std::get<2>(comp);

        // Check against lower x grid dimensions
        const size_t edge = _grid_scale - 1;
        if (x != 0)
        {
            const size_t nxk = min::vec3<float>::grid_key(std::make_tuple(x - 1, y, z), _grid_scale);
            _neighbors.push_back({nxk, grid_center_square_dist(nxk, stop)});
        }

        // Check against upper x grid dimensions
        if (x != edge)
        {
            const size_t pxk = min::vec3<float>::grid_key(std::make_tuple(x + 1, y, z), _grid_scale);
            _neighbors.push_back({pxk, grid_center_square_dist(pxk, stop)});
        }

        // Check against lower y grid dimensions
        if (y != 0)
        {
            const size_t nyk = min::vec3<float>::grid_key(std::make_tuple(x, y - 1, z), _grid_scale);
            _neighbors.push_back({nyk, grid_center_square_dist(nyk, stop)});
        }

        // Check against upper y grid dimensions
        if (y != edge)
        {
            const size_t pyk = min::vec3<float>::grid_key(std::make_tuple(x, y + 1, z), _grid_scale);
            _neighbors.push_back({pyk, grid_center_square_dist(pyk, stop)});
        }

        // Check against lower z grid dimensions
        if (z != 0)
        {
            const size_t nzk = min::vec3<float>::grid_key(std::make_tuple(x, y, z - 1), _grid_scale);
            _neighbors.push_back({nzk, grid_center_square_dist(nzk, stop)});
        }

        // Check against upper z grid dimensions
        if (z != edge)
        {
            const size_t pzk = min::vec3<float>::grid_key(std::make_tuple(x, y, z + 1), _grid_scale);
            _neighbors.push_back({pzk, grid_center_square_dist(pzk, stop)});
        }

        // lambda function to create sorted array indices based on distance
        std::sort(_neighbors.begin(), _neighbors.end(), [](const std::pair<size_t, float> &a, const std::pair<size_t, float> &b) {
            return a.second > b.second;
        });
    }
    inline bool search_next(const min::vec3<float> &stop, const size_t stop_key)
    {
        // Check if stack is empty to avoid stomping the stack
        if (_stack.size() == 0)
        {
            return true;
        }
        else if (_path.size() > _search_limit)
        {
            return true;
        }

        // Get element on top of stack
        const size_t key = _stack.back();

        // Check if we made it to the mother lands!
        if (key == stop_key)
        {
            // Store end point of path
            _path.push_back(key);

            // Stop searching
            return true;
        }
        else if (_visit[key] == 1)
        {
            // If we haven't seen this node yet, we are traversing
            _path.push_back(key);

            // Visit this node
            _visit[key] = 0;

            // Search along the gradient until we hit a wall
            const std::tuple<size_t, size_t, size_t> comp = grid_key_unpack(key);

            // Search all neighboring cells
            search_neighbors(comp, stop);

            for (const auto &n : _neighbors)
            {
                // If we haven't visited the neighbor cell, and it isn't a wall
                if (_visit[n.first] == -1 && _grid[n.first] == block_id::EMPTY)
                {
                    // Flag that we pushed this key to prevent duplicates on stack
                    _visit[n.first] = 1;

                    // Calculate distance to destination
                    _stack.push_back(n.first);
                }
            }
        }
        else if (_visit[key] == 0)
        {
            // If we already visited this node, we must be unwinding so pop stack
            _stack.pop_back();

            // Every time we visit we push so we must pop here
            _path.pop_back();
        }

        // Keepp looking for a path
        return false;
    }
    inline void world_create()
    {
        // Else generate world
        generate_world();

        // Reserve and update all chunks
        const size_t chunks = _chunks.size();
        for (size_t i = 0; i < chunks; i++)
        {
            chunk_warm(i);
            chunk_update(i);
        }
    }
    inline void world_load(const size_t index)
    {
        // Create output stream for loading world
        std::vector<uint8_t> stream;

        // Load data into stream from file
        load_file("save/world." + std::to_string(index), stream);

        // If load failed dont try to parse stream data
        if (stream.size() != 0)
        {
            // Load grid with file
            size_t next = 0;
            const std::vector<block_id> grid = min::read_le_vector<block_id>(stream, next);

            // Check that grid load correctly
            const size_t cubic_size = _grid_scale * _grid_scale * _grid_scale;
            if (grid.size() == cubic_size)
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
            // No file found
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

  public:
    constexpr static float _player_dx = 0.45;
    constexpr static float _player_dy = 0.95;
    constexpr static float _player_dz = 0.45;
    cgrid(const size_t chunk_size, const size_t grid_scale, const size_t view_chunk_size)
        : _grid_scale(grid_scale * 2),
          _grid(_grid_scale * _grid_scale * _grid_scale, block_id::EMPTY),
          _visit(_grid.size(), -1),
          _chunk_cells(chunk_size * chunk_size * chunk_size),
          _chunk_size(chunk_size),
          _chunk_scale(_grid_scale / _chunk_size),
          _chunks(_chunk_scale * _chunk_scale * _chunk_scale, min::mesh<float, uint32_t>("chunk")),
          _chunk_update(_chunks.size(), true),
          _recent_chunk(0),
          _view_chunk_size(view_chunk_size),
          _view_half_width(_view_chunk_size / 2),
          _view_dist(calculate_view_distance()),
          _world(calculate_world_size(grid_scale)),
          _cell_extent(1.0, 1.0, 1.0),
          _generator(_grid), _mesher(chunk_size)
    {
        // Check chunk size
        if (grid_scale % chunk_size != 0)
        {
            throw std::runtime_error("cgrid: chunk_size must evenly divide grid_scale");
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

        // Reserve memory
        reserve_memory();
    }
    inline void load(const options &opt)
    {
        // Reset the grid
        reset();

        // Load the world
        world_load(opt.get_save_slot());
    }
    inline void new_game()
    {
        // Reset the grid
        reset();

        // Load the world
        world_create();
    }
    inline void save(const options &opt)
    {
        // Create output stream for saving world
        std::vector<uint8_t> stream;

        // Reserve space for grid
        stream.reserve(_grid.size() * sizeof(block_id));

        // Write data into stream
        min::write_le_vector<block_id>(stream, _grid);

        // Write data to file
        save_file("save/world." + std::to_string(opt.get_save_slot()), stream);
    }
    static inline min::aabbox<float, min::vec3> grid_box(const min::vec3<float> &p)
    {
        // Create box at center
        const min::vec3<float> half_extent(0.5, 0.5, 0.5);

        // Return the box
        return min::aabbox<float, min::vec3>(p - half_extent, p + half_extent);
    }
    static inline min::aabbox<float, min::vec3> drone_box(const min::vec3<float> &p)
    {
        // Create box at center
        const min::vec3<float> half_extent(0.45, 0.45, 0.45);

        // Return the box
        return min::aabbox<float, min::vec3>(p - half_extent, p + half_extent);
    }
    static inline min::aabbox<float, min::vec3> drop_box(const min::vec3<float> &p)
    {
        // Create box at center
        const min::vec3<float> half_extent(0.25, 0.25, 0.25);

        // Return the box
        return min::aabbox<float, min::vec3>(p - half_extent, p + half_extent);
    }
    static inline min::aabbox<float, min::vec3> explode_box(const min::vec3<float> &p)
    {
        // Create box at center
        const min::vec3<float> half_extent(0.25, 0.25, 0.25);

        // Return the box
        return min::aabbox<float, min::vec3>(p - half_extent, p + half_extent);
    }
    static inline min::aabbox<float, min::vec3> missile_box(const min::vec3<float> &p)
    {
        // Create box at center
        const min::vec3<float> half_extent(0.25, 0.25, 0.25);

        // Return the box
        return min::aabbox<float, min::vec3>(p - half_extent, p + half_extent);
    }
    static inline min::aabbox<float, min::vec3> player_box(const min::vec3<float> &p)
    {
        // Create box at center
        const min::vec3<float> half_extent(_player_dx, _player_dy, _player_dz);

        // Return the box
        return min::aabbox<float, min::vec3>(p - half_extent, p + half_extent);
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
    inline void drone_collision_cells(std::vector<std::pair<min::aabbox<float, min::vec3>, block_id>> &out, const min::vec3<float> &center) const
    {
        // Surrounding cells
        out.clear();

        // Check if position is valid
        const bool valid = inside(center);
        if (valid)
        {
            // Create box at this point
            const min::aabbox<float, min::vec3> box = drone_box(center);

            // Calculate collision cells around this box
            collision_cells(out, box, center);
        }
    }
    inline void drop_collision_cells(std::vector<std::pair<min::aabbox<float, min::vec3>, block_id>> &out, const min::vec3<float> &center) const
    {
        // Surrounding cells
        out.clear();

        // Check if position is valid
        const bool valid = inside(center);
        if (valid)
        {
            // Create box at this point
            const min::aabbox<float, min::vec3> box = drop_box(center);

            // Calculate collision cells around this box
            collision_cells(out, box, center);
        }
    }
    inline void explosive_collision_cells(std::vector<std::pair<min::aabbox<float, min::vec3>, block_id>> &out, const min::vec3<float> &center) const
    {
        // Surrounding cells
        out.clear();

        // Check if position is valid
        const bool valid = inside(center);
        if (valid)
        {
            // Create box at this point
            const min::aabbox<float, min::vec3> box = explode_box(center);

            // Calculate collision cells around this box
            collision_cells(out, box, center);
        }
    }
    inline void missile_collision_cells(std::vector<std::pair<min::aabbox<float, min::vec3>, block_id>> &out, const min::vec3<float> &center) const
    {
        // Surrounding cells
        out.clear();

        // Check if position is valid
        const bool valid = inside(center);
        if (valid)
        {
            // Create box at this point
            const min::aabbox<float, min::vec3> box = missile_box(center);

            // Calculate collision cells around this box
            collision_cells(out, box, center);
        }
    }
    inline void player_collision_cells(std::vector<std::pair<min::aabbox<float, min::vec3>, block_id>> &out, const min::vec3<float> &center) const
    {
        // Surrounding cells
        out.clear();

        // Check if position is valid
        const bool valid = inside(center);
        if (valid)
        {
            // Create box at this point
            const min::aabbox<float, min::vec3> box = player_box(center);

            // Calculate collision cells around this box
            collision_cells(out, box, center);
        }
    }
    inline void flush_chunk_updates()
    {
        // Sort chunk keys using a radix sort
        min::uint_sort<size_t>(_chunk_update_keys, _sort_chunk, [](const size_t i) {
            return i;
        });

        // Make keys unique
        const auto last = std::unique(_chunk_update_keys.begin(), _chunk_update_keys.end());

        // Erase empty spaces in vector
        _chunk_update_keys.erase(last, _chunk_update_keys.end());

        // Update all modified chunks
        for (const auto k : _chunk_update_keys)
        {
            chunk_update(k);
        }

        // Clear out chunk update keys
        _chunk_update_keys.clear();
    }
    inline min::mesh<float, uint32_t> &get_chunk(const size_t key)
    {
        return _chunks[key];
    }
    inline size_t get_chunks() const
    {
        return _chunks.size();
    }
    inline size_t get_chunk_scale() const
    {
        return _chunk_scale;
    }
    inline const std::vector<view_chunk> &get_view_chunks() const
    {
        return _view_chunks;
    }
    inline const min::aabbox<float, min::vec3> &get_world()
    {
        return _world;
    }
    inline bool is_viewable(const min::camera<float> &cam, const min::aabbox<float, min::vec3> &box) const
    {
        // Is the box inside the frustum?
        if (min::intersect<float>(cam.get_frustum(), box))
        {
            // Check box against view distance from current chunk
            const float dist = (box.get_center() - _recent_p).magnitude();
            if (dist < _view_dist)
            {
                return true;
            }
        }

        return false;
    }
    inline unsigned load_swatch(swatch &sw, const min::vec3<float> &start, const min::vec3<int> &offset, const min::vec3<unsigned> &length) const
    {
        // Swatch cost
        unsigned out = 0;

        // Load swatch offset and length
        sw.set_length(length);
        sw.set_offset(offset);

        // Create cubic function, for each cell in cubic space
        const auto f = [this, &sw, &out](const size_t i, const size_t j, const size_t k, const size_t key) {
            // Get the atlas of this grid point
            const block_id atlas = this->_grid[key];

            // Load atlas into swatch
            sw.set(i, j, k, atlas);

            // Count ether cost
            out += ether_cost(atlas);
        };

        // Run the function
        cubic_grid(start, length, offset, f);

        // Return the swatch cost
        return out;
    }
    inline void preview_atlas(min::mesh<float, uint32_t> &mesh, const min::vec3<int> &offset, const min::vec3<unsigned> &length, const block_id atlas) const
    {
        // Clear the mesher
        _mesher.clear();

        // Convert atlas to a float
        const float float_atlas = static_cast<float>(atlas);

        // Calculate max edges
        const auto edges = std::make_tuple(length.x() - 1, length.y() - 1, length.z() - 1);

        // Create cubic function, for each cell in cubic space
        const auto f = [this, &offset, &edges, float_atlas](const size_t i, const size_t j, const size_t k, const size_t key) {
            // Add data to mesh for each cell
            const min::vec3<float> p = grid_cell(key);

            // Pack the current index
            const auto index = std::make_tuple(i, j, k);

            // Generate the rotated chunk face
            _mesher.generate_place_faces_rotated(p, offset, index, edges, float_atlas);
        };

        // Store start point => (0,0,0),
        // FOR ATLAS ONLY (0, 0, 0) IS THE CENTER!
        // Different than grid because of translation matrix!
        min::vec3<float> start;

        // Run the function
        cubic_grid(start, length, offset, f);

        // Generate mesh
        _mesher.generate_preview(mesh);
    }
    inline void preview_swatch(min::mesh<float, uint32_t> &mesh, const swatch &sw) const
    {
        // Clear the mesher
        _mesher.clear();

        // Calculate max edges
        const min::vec3<unsigned> &length = sw.get_length();
        const auto edges = std::make_tuple(length.x() - 1, length.y() - 1, length.z() - 1);

        // Function to retrieve block value
        const auto get_block = [this, &sw](const std::tuple<size_t, size_t, size_t> &t) -> block_id {
            // Get the block atlas
            const block_id atlas = sw.get(std::get<0>(t), std::get<1>(t), std::get<2>(t));

            // If it's empty return purple crystals
            return (atlas == block_id::EMPTY) ? block_id::CRYSTAL_P : atlas;
        };

        // Create cubic function, for each cell in cubic space
        const auto f = [this, &sw, &edges, &get_block](const size_t i, const size_t j, const size_t k, const size_t key) {
            // Add data to mesher for each cell
            const min::vec3<float> p = this->grid_cell(key);

            // Pack the current index
            const auto index = std::make_tuple(i, j, k);

            // Convert atlas to a float
            const float float_atlas = static_cast<float>(get_block(index));

            // Mesh the cell
            this->_mesher.generate_chunk_faces_rotated(p, sw.get_offset(), index, edges, get_block, float_atlas);
        };

        // Store start point => (0,0,0),
        // FOR ATLAS ONLY (0, 0, 0) IS THE CENTER!
        // Different than grid because of translation matrix!
        min::vec3<float> start;

        // Run the function
        cubic_grid(start, sw.get_length(), sw.get_offset(), f);

        // Generate mesh
        _mesher.generate_preview(mesh);
    }
    inline bool ray_trace_last_key(const min::ray<float, min::vec3> &r, const size_t length, min::vec3<float> &point, size_t &key, block_id &value) const
    {
        // Trace a ray and return the last key
        size_t prev_key;
        const bool is_valid = ray_trace(r, length, prev_key, key, value);
        if (is_valid)
        {
            // Return the snapped point
            point = grid_cell_center(key);
        }

        // Return if valid
        return is_valid;
    }
    inline min::vec3<float> ray_trace_last(const min::ray<float, min::vec3> &r, const size_t length, block_id &value) const
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
        block_id value = block_id::INVALID;
        const bool is_valid = ray_trace(r, length, prev_key, key, value);
        if (is_valid)
        {
            // return the snapped point
            return grid_cell_center(prev_key);
        }

        // return ray start point since it is not in the grid
        return r.get_origin();
    }
    inline void path(std::vector<min::vec3<float>> &out, const min::vec3<float> &start, const min::vec3<float> &stop)
    {
        // Convert keys to points
        out.clear();

        // Try to find a path between points
        search(start, stop);

        // For all keys in path
        const size_t size = _path.size();
        for (size_t i = 0; i < size; i++)
        {
            out.push_back(grid_cell_center(_path[i]));
        }
    }
    inline void portal()
    {
        generate_portal();

        // Update all chunks
        const size_t chunks = _chunks.size();
        for (size_t i = 0; i < chunks; i++)
        {
            chunk_update(i);
        }
    }
    inline void set_boundary_chunk(const size_t key)
    {
        // Find out if we are on a chunk boundary
        const auto g = grid_key_unpack(key);
        const size_t gx = std::get<0>(g);
        const size_t gy = std::get<1>(g);
        const size_t gz = std::get<2>(g);

        // Relative grid components in chunk
        const size_t rgx = gx % _chunk_size;
        const size_t rgy = gy % _chunk_size;
        const size_t rgz = gz % _chunk_size;

        // Chunk index of grid index
        const size_t cx = gx / _chunk_size;
        const size_t cy = gy / _chunk_size;
        const size_t cz = gz / _chunk_size;

        // Convert chunk grid components to chunk index
        const auto to_chunk_key = [this](const size_t x, const size_t y, const size_t z) -> size_t {
            return (x * _chunk_scale * _chunk_scale) + (y * _chunk_scale) + (z);
        };

        // Chunk top edge
        const size_t c_edge = _chunk_size - 1;

        // World top edge
        const size_t w_edge = _grid_scale - 1;

        // Check x axis boundary
        if (rgx == 0 && gx != 0)
        {
            const size_t ckey = to_chunk_key(cx - 1, cy, cz);
            _chunk_update_keys.push_back(ckey);
        }
        else if (rgx % c_edge == 0 && gx != w_edge)
        {
            const size_t ckey = to_chunk_key(cx + 1, cy, cz);
            _chunk_update_keys.push_back(ckey);
        }

        // Check y axis boundary
        if (rgy == 0 && gy != 0)
        {
            const size_t ckey = to_chunk_key(cx, cy - 1, cz);
            _chunk_update_keys.push_back(ckey);
        }
        else if (rgy % c_edge == 0 && gy != w_edge)
        {
            const size_t ckey = to_chunk_key(cx, cy + 1, cz);
            _chunk_update_keys.push_back(ckey);
        }

        // Check z axis boundary
        if (rgz == 0 && gz != 0)
        {
            const size_t ckey = to_chunk_key(cx, cy, cz - 1);
            _chunk_update_keys.push_back(ckey);
        }
        else if (rgz % c_edge == 0 && gz != w_edge)
        {
            const size_t ckey = to_chunk_key(cx, cy, cz + 1);
            _chunk_update_keys.push_back(ckey);
        }
    }
    unsigned set_geometry(const swatch &sw, const min::vec3<float> &start)
    {
        // Modified geometry
        unsigned out = 0;

        // Get cubic function properties
        const min::vec3<unsigned> &length = sw.get_length();
        const min::vec3<int> &offset = sw.get_offset();

        // Record all modified chunks and store them for updating
        _chunk_update_keys.reserve(length.x() * length.y() * length.z());

        // If the start point is inside the grid
        const bool in = inside(start);
        if (in)
        {
            out += geometry_copy_swatch(sw, start, length, offset);
        }

        // Return the number of modified blocks
        return out;
    }
    template <typename SB>
    unsigned set_geometry(const min::vec3<float> &start, const min::vec3<unsigned> &length, const min::vec3<int> &offset,
                          const block_id atlas_id, const SB &set_block_call)
    {
        // Modified geometry
        unsigned out = 0;

        // Record all modified chunks and store them for updating
        _chunk_update_keys.reserve(length.x() * length.y() * length.z());

        // If the start point is inside the grid
        const bool in = inside(start);
        if (atlas_id == block_id::EMPTY && in)
        {
            // Remove geometry
            out += geometry_remove(start, length, offset, atlas_id, set_block_call);
        }
        else if (in)
        {
            // Add geometry
            out += geometry_add(start, length, offset, atlas_id);
        }

        // Return the number of modified blocks
        return out;
    }
    min::vec3<float> set_geometry_box_3x3(const min::vec3<float> &p, const block_id atlas)
    {
        // Record all modified chunks and store them for updating
        _chunk_update_keys.reserve(18);

        // Get random position
        const min::vec3<float> snapped = snap(p);
        const float nx = snapped.x() - 1.0;
        const float ny = snapped.y() - 1.0;
        const float nz = snapped.z() - 1.0;

        // Create enclosed cube 3x3
        min::vec3<float> start(nx, ny, nz);
        min::vec3<unsigned> length(3, 3, 3);
        const min::vec3<int> offset(1, 1, 1);

        // Dummy callback
        const auto f = [](const min::vec3<float> &, const block_id) -> void {
        };

        // Carve out inside of box
        geometry_remove(start, length, offset, block_id::EMPTY, f);

        // Create -XZ floor
        start.y(ny - 1.0);
        length.y(1);
        geometry_add(start, length, offset, atlas);

        // Create +XZ floor
        start.y(snapped.y() + 2.0);
        geometry_add(start, length, offset, atlas);

        // Reset
        start.y(ny);
        length.y(3);
        length.z(1);

        // Create the -XY wall
        start.z(nz - 1.0);
        geometry_add(start, length, offset, atlas);

        // Create the +XY wall
        start.z(snapped.z() + 2.0);
        geometry_add(start, length, offset, atlas);

        // Reset
        start.z(nz);
        length.x(1);
        length.z(3);

        // Create the -YZ wall
        start.x(nx - 1.0);
        geometry_add(start, length, offset, atlas);

        // Create the +YZ wall
        start.x(snapped.x() + 2.0);
        geometry_add(start, length, offset, atlas);

        // Return snapped point
        return snapped;
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
            _recent_p = chunk_center(_recent_chunk);
        }
    }
    inline void update_view_chunk_index(const min::camera<float> &cam, std::vector<size_t> &out)
    {
        out.clear();
        _view_chunks.clear();

        // Center of view chunks
        const min::vec3<float> center = chunk_start(_recent_chunk);

        // Calculate view half width chunk size
        const float half_width = _chunk_size * _view_half_width;

        // Get cubic chunk start point
        const min::vec3<float> start = center - min::vec3<float>(half_width, half_width, half_width);
        const min::vec3<unsigned> length(_view_chunk_size, _view_chunk_size, _view_chunk_size);
        const min::vec3<int> offset(_chunk_size, _chunk_size, _chunk_size);

        // Calculate a weighted center to favor chunks in front of viewer
        const min::vec3<float> weight_center = cam.project_point(_chunk_size / 2);

        // Count for assigning indices
        size_t count = 0;

        // Create cubic function, for each chunk in cubic space
        const auto f = [this, &cam, &count, &weight_center](const min::vec3<float> &p) {
            // Create chunk bounding box
            const min::aabbox<float, min::vec3> box = this->create_chunk_box(p);

            // If the view is within the frustum
            if (min::intersect<float>(cam.get_frustum(), box))
            {
                // Get the key for this chunk
                const size_t key = this->chunk_key_unsafe(p);

                // Calculate square distances from center of view frustum
                const min::vec3<float> diff = weight_center - box.get_center();
                const float dist = diff.dot(diff);

                // Store the index, key, box and dist for this view chunk
                this->_view_chunks.emplace_back(count++, key, box, dist);
            }
        };

        // Run the function
        cubic(start, length, offset, f);

        // Sort the view indices based on distance from camera to reduce overdraw
        std::sort(_view_chunks.begin(), _view_chunks.end(), [](const view_chunk &a, const view_chunk &b) {
            return a.get_dist() < b.get_dist();
        });

        // Sorted indices based off distance from center of view frustum, ascending order
        for (const view_chunk &vc : _view_chunks)
        {
            out.push_back(vc.get_key());
        }
    }
};
}

#endif
