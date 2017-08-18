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
    static min::mesh<float, uint32_t> create_box_mesh(const min::aabbox<float, min::vec3> &box, const int8_t atlas_id)
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
    static min::aabbox<float, min::vec3> create_box(const min::vec3<float> &center)
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
            throw std::runtime_error("world_mesh: chunk index is not inside the world");
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
    void chunk_update(const size_t key)
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
    void generate_world()
    {
        // generate mandelbulb world using mandelbulb generator
        mandelbulb().generate(_grid, _grid_size, [this](const size_t i) {
            return this->grid_center(i);
        });
    }
    std::vector<size_t> get_surrounding_chunks(const size_t key) const
    {
        std::vector<size_t> out;
        out.reserve(27);
        out.push_back(key);

        // Get cubic function properties
        const min::vec3<float> start = chunk_start(key) - min::vec3<float>(_chunk_size, _chunk_size, _chunk_size);
        const min::vec3<int> offset(_chunk_size, _chunk_size, _chunk_size);
        const min::vec3<unsigned> length(3, 3, 3);

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

        return out;
    }
    inline min::vec3<float> grid_center(const size_t index) const
    {
        if (index >= _grid.size())
        {
            throw std::runtime_error("world_mesh: grid index is not inside the world cell");
        }

        // Precalculate the square scale
        const size_t scale2 = _grid_size * _grid_size;

        // Calculate row, col and height
        const size_t row = index / scale2;
        const size_t col = (index - row * scale2) / _grid_size;
        const size_t hei = index - row * scale2 - col * _grid_size;

        // Calculate the center point of the box cell
        const float x = row + _world.get_min().x() + 0.5;
        const float y = col + _world.get_min().y() + 0.5;
        const float z = hei + _world.get_min().z() + 0.5;

        return min::vec3<float>(x, y, z);
    }
    inline size_t grid_key(const min::vec3<float> &point, bool &valid) const
    {
        // This function can crash so we need protection
        if (!_world.point_inside(point))
        {
            valid = false;
        }

        // Compute the grid index from point
        const min::vec3<float> cell_extent(1.0, 1.0, 1.0);
        return min::vec3<float>::grid_key(_world.get_min(), cell_extent, _grid_size, point);
    }
    void world_load()
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
    void world_save()
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
          _world(min::vec3<float>(grid_size, grid_size, grid_size) * -1.0, min::vec3<float>(grid_size, grid_size, grid_size)),
          _atlas_id(0)
    {
        // Add starting blocks to simulation
        world_load();
    }
    ~cgrid()
    {
        // Save the world state in file
        world_save();
    }
    min::mesh<float, uint32_t> atlas_box(const min::vec3<float> &p)
    {
        const min::aabbox<float, min::vec3> box = create_box(p);
        return create_box_mesh(box, _atlas_id);
    }
    inline size_t chunk_key(const min::vec3<float> &point, bool &valid) const
    {
        // This function can crash so we need protection
        if (!_world.point_inside(point))
        {
            valid = false;
        }

        // Compute the chunk index from point
        const min::vec3<float> cell_extent(_chunk_size, _chunk_size, _chunk_size);
        return min::vec3<float>::grid_key(_world.get_min(), cell_extent, _chunk_scale, point);
    }
    std::vector<min::aabbox<float, min::vec3>> create_collision_cells(const min::vec3<float> &center) const
    {
        // Surrounding cells
        std::vector<min::aabbox<float, min::vec3>> out;
        out.reserve(36);

        // Get cubic function properties
        const min::vec3<float> start = center - min::vec3<float>(1.0, 1.5, 1.0);
        const min::vec3<int> offset(1, 1, 1);
        const min::vec3<unsigned> length(3, 4, 3);

        // Create cubic function, for each cell in cubic space
        const auto f = [this, &out](const min::vec3<float> &p) {
            // Calculate grid key index
            bool is_valid = true;
            const size_t key = grid_key(p, is_valid);

            // Check if valid and if it box is not empty
            if (is_valid && _grid[key] != -1)
            {
                // Create box at this point
                out.push_back(create_box(p));
            }
        };

        // Run the function
        cubic(start, offset, length, f);

        return out;
    }
    int8_t get_atlas() const
    {
        return _atlas_id;
    }
    void set_atlas(const int8_t id)
    {
        _atlas_id = id;
    }
    const min::mesh<float, uint32_t> &get_chunk(const size_t index) const
    {
        return _chunks[index];
    }
    std::vector<int8_t> get_neighbors(const min::vec3<float> &center) const
    {
        // Surrounding cells
        std::vector<int8_t> out;
        out.reserve(27);

        // Get cubic function properties
        const min::vec3<float> start = center - min::vec3<float>(1.0, 1.5, 1.0);
        const min::vec3<int> offset(1, 1, 1);
        const min::vec3<unsigned> length(3, 3, 3);

        // Create cubic function, for each cell in cubic space
        const auto f = [this, &out](const min::vec3<float> &p) {
            // Calculate grid key index
            bool is_valid = true;
            const size_t key = grid_key(p, is_valid);
            if (is_valid)
            {
                // Create box at this point
                out.push_back(_grid[key]);
            }
            else
            {
                // This means an invalid boundary
                out.push_back(-2);
            }
        };

        // Run the function
        cubic(start, offset, length, f);

        return out;
    }
    size_t get_recent_chunk() const
    {
        return _recent_chunk;
    }
    std::vector<size_t> get_view_chunks() const
    {
        std::vector<size_t> out;
        const size_t size = _view_chunk_size * _view_chunk_size * _view_chunk_size;
        out.reserve(size);
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

        return out;
    }
    const min::aabbox<float, min::vec3> &get_world()
    {
        return _world;
    }
    int8_t grid_value(const min::vec3<float> &point) const
    {
        // Lookup grid index from point
        bool is_valid = true;
        size_t next_key = grid_key(point, is_valid);
        if (is_valid)
        {
            // Return the atlas id at grid cell
            return _grid[next_key];
        }

        return -1;
    }
    min::vec3<float> ray_trace_after(const min::ray<float, min::vec3> &r, const size_t length)
    {
        const min::vec3<float> cell_extent(1.0, 1.0, 1.0);

        // Calculate the ray trajectory for tracing in grid
        auto grid_ray = min::vec3<float>::grid_ray(cell_extent, r.get_origin(), r.get_direction(), r.get_inverse());

        // Calculate start point in grid index format
        auto index = min::vec3<float>::grid_index(_world.get_min(), cell_extent, r.get_origin());

        // Trace a ray from origin and stop at first populated cell
        bool is_valid = true;
        size_t next_key = grid_key(r.get_origin(), is_valid);
        if (is_valid)
        {
            // bad flag signals that we have hit the last valid cell
            bool bad_flag = false;
            unsigned count = 0;
            while (_grid[next_key] == -1 && !bad_flag && count < length)
            {
                next_key = min::vec3<float>::grid_ray_next(index, grid_ray, bad_flag, _grid_size);
                count++;
            }

            // return the snapped point
            return grid_center(next_key);
        }

        // return ray start point since it is not in the grid
        return r.get_origin();
    }
    min::vec3<float> ray_trace_before(const min::ray<float, min::vec3> &r, const size_t length)
    {
        const min::vec3<float> cell_extent(1.0, 1.0, 1.0);

        // Calculate the ray trajectory for tracing in grid
        auto grid_ray = min::vec3<float>::grid_ray(cell_extent, r.get_origin(), r.get_direction(), r.get_inverse());

        // Calculate start point in grid index format
        auto index = min::vec3<float>::grid_index(_world.get_min(), cell_extent, r.get_origin());

        // Trace a ray from origin and stop at before populated cell
        bool is_valid = true;
        size_t next_key = grid_key(r.get_origin(), is_valid);
        if (is_valid)
        {
            // bad flag signals that we have hit the last valid cell
            size_t before_key = next_key;
            bool bad_flag = false;
            unsigned count = 0;
            while (_grid[next_key] == -1 && !bad_flag && count < length)
            {
                before_key = next_key;
                next_key = min::vec3<float>::grid_ray_next(index, grid_ray, bad_flag, _grid_size);
                count++;
            }

            // return the snapped point
            return grid_center(before_key);
        }

        // return ray start point since it is not in the grid
        return r.get_origin();
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
                const std::vector<size_t> sc = get_surrounding_chunks(ckey);

                // Insert surrounding chunks into keys
                keys.insert(keys.end(), sc.begin(), sc.end());
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

        // Return the number of removed blocks
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
