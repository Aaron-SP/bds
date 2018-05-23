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
#ifndef __STATIC_INSTANCE__
#define __STATIC_INSTANCE__

#include <game/cgrid.h>
#include <game/geometry.h>
#include <game/id.h>
#include <game/memory_map.h>
#include <game/uniforms.h>
#include <min/aabbox.h>
#include <min/camera.h>
#include <min/grid.h>
#include <min/mat4.h>
#include <min/physics_nt.h>
#include <min/program.h>
#include <min/shader.h>
#include <min/static_vertex.h>
#include <min/texture_buffer.h>
#include <min/utility.h>
#include <min/vec3.h>
#include <min/vertex_buffer.h>
#include <min/wavefront.h>
#include <stdexcept>
#include <vector>

namespace game
{

enum class static_id : size_t
{
    PLAYER = 0,
    CHEST = 1,
    DRONE = 2,
    DROP = 3,
    EXPLOSIVE = 4,
    MISSILE = 5,
    ASSET_SIZE = MISSILE
};

inline constexpr size_t id_value(const static_id id)
{
    return static_cast<size_t>(id);
}

class static_asset
{
  private:
    const size_t _iid;
    const GLuint _tid;
    const size_t _start_index;
    const size_t _limit;
    const min::aabbox<float, min::vec3> _box;
    std::vector<size_t> _index;
    std::vector<min::mat4<float>> _mat;
    std::vector<min::mat4<float>> _mat_out;

    inline void reserve_memory(const size_t limit)
    {
        _index.reserve(limit);
        _mat.reserve(limit);
        _mat_out.reserve(limit);
    }

  public:
    static_asset(
        const GLuint iid, const size_t tid,
        const size_t index, const size_t limit,
        const min::aabbox<float, min::vec3> &box)
        : _iid(iid), _tid(tid), _start_index(index), _limit(limit), _box(box)
    {
        // Reserve memory
        reserve_memory(limit);
    }

    inline void add_index(const size_t index)
    {
        _index.push_back(index);
    }
    inline size_t add(const min::vec3<float> &p)
    {
        // Check for buffer overflow
        if (_mat.size() == _limit)
        {
            throw std::runtime_error("static_instance: must change default count");
        }

        // Push back location
        _mat.push_back(p);

        // Return chest id
        return _mat.size() - 1;
    }
    inline size_t add(const min::vec3<float> &p, const block_id atlas)
    {
        // Check for buffer overflow
        if (_mat.size() == _limit)
        {
            throw std::runtime_error("static_instance: must change default count");
        }

        // Push back location
        _mat.push_back(p);

        // Pack the matrix with the atlas id
        const float float_atlas = static_cast<float>(atlas);
        const float w = float_atlas + 2.1;
        _mat.back().w(w);

        // Return chest id
        return _mat.size() - 1;
    }
    inline void clear(const size_t index)
    {
        _mat.erase(_mat.begin() + index);
    }
    inline void clear()
    {
        _mat.clear();
    }
    inline void clear_index()
    {
        _index.clear();
    }
    inline void copy_mat_index()
    {
        // Cull empty index buffer
        const size_t size = _index.size();
        if (size > 0)
        {
            // Resize output buffer
            _mat_out.resize(size);

            // Copy elements from index buffer
            for (size_t i = 0; i < size; i++)
            {
                _mat_out[i] = _mat[_index[i]];
            }
        }
        else
        {
            _mat_out.clear();
        }
    }
    inline void cull_frustum(const cgrid &grid, const min::camera<float> &cam)
    {
        // Cull chests
        const size_t size = _mat.size();
        for (size_t i = 0; i < size; i++)
        {
            // Create chest bounding box from matrix position
            const min::aabbox<float, min::vec3> box = get_box(i);

            // If the box is within the frustum
            if (grid.is_viewable(cam, box))
            {
                _index.push_back(i);
            }
        }
    }
    inline bool is_full() const
    {
        return _mat.size() == _limit;
    }
    inline min::aabbox<float, min::vec3> get_box(const size_t index) const
    {
        // Create box for this mob
        min::aabbox<float, min::vec3> box(_box);

        // Move box to mob position
        box.set_position(_mat[index].get_translation());

        // Return this box for collisions
        return box;
    }
    inline size_t get_iid() const
    {
        return _iid;
    }
    inline const std::vector<min::mat4<float>> &get_in_matrix() const
    {
        return _mat;
    }
    inline const std::vector<min::mat4<float>> &get_out_matrix() const
    {
        return _mat_out;
    }
    inline size_t get_start_index() const
    {
        return _start_index;
    }
    inline size_t get_tid() const
    {
        return _tid;
    }
    inline size_t max() const
    {
        return _limit;
    }
    inline size_t view_size() const
    {
        return _mat_out.size();
    }
    inline void sort_prune_index(std::vector<size_t> &sort)
    {
        // Cull empty index buffer
        if (_index.size() > 0)
        {
            // Sort index keys using a radix sort
            min::uint_sort<size_t>(_index, sort, [](const size_t i) {
                return i;
            });

            // Make keys unique
            const auto last = std::unique(_index.begin(), _index.end());

            // Erase empty spaces in vector
            _index.erase(last, _index.end());
        }
    }
    inline void update_position(const size_t index, const min::vec3<float> &p)
    {
        _mat[index].set_translation(p);
    }
    inline void update_rotation(const size_t index, const min::quat<float> &r)
    {
        _mat[index].set_rotation(r);
    }
    inline void update_atlas(const size_t index, const block_id atlas)
    {
        const float float_atlas = static_cast<float>(atlas);
        const float w = float_atlas + 2.1;
        _mat[index].w(w);
    }
};

class static_instance
{
  private:
    typedef min::physics<float, uint16_t, uint32_t, min::vec3, min::aabbox, min::aabbox, min::grid> physics;
    static constexpr size_t _CHEST_LIMIT = 10;
    static constexpr size_t _DRONE_LIMIT = 10;
    static constexpr size_t _DROP_LIMIT = 50;
    static constexpr size_t _EXPLODE_LIMIT = 10;
    static constexpr size_t _MISS_LIMIT = 10;

    min::shader _vertex;
    min::shader _fragment;
    min::program _prog;
    GLint _index_location;

    // Buffers for model data and textures
    min::vertex_buffer<float, uint16_t, min::static_vertex, GL_FLOAT, GL_UNSIGNED_SHORT> _buffer;
    min::texture_buffer _texture_buffer;
    std::vector<static_asset> _assets;
    std::vector<size_t> _sort_index;

    inline void cull_frustum(const cgrid &grid, const min::camera<float> &cam)
    {
        const size_t size = _assets.size();
        for (size_t i = 0; i < size; i++)
        {
            _assets[i].cull_frustum(grid, cam);
        }
    }
    inline void cull_physics(const physics &sim, const cgrid &grid)
    {
        // Get the view chunks from grid
        const std::vector<view_chunk> &view_chunks = grid.get_view_chunks();

        // Get the index map
        const std::vector<uint16_t> &map = sim.get_index_map();

        // For each view chunk
        for (const view_chunk &vc : view_chunks)
        {
            // Perform overlapping of physics bodies
            const std::vector<std::pair<uint16_t, uint16_t>> &over = sim.get_overlap(vc.get_box());

            // For each body in view chunk
            const size_t size = over.size();
            for (size_t i = 0; i < size; i++)
            {
                // Get the body from the simulation
                const min::body<float, min::vec3> &b = sim.get_body(map[over[i].first]);

                // Get the type of body
                const size_t id = b.get_id();

                // If the body is not dead
                if (!b.is_dead() && id != id_value(static_id::PLAYER))
                {
                    _assets[id - 1].add_index(b.get_data().index);
                }
            }
        }
    }
    inline void load_chest_model()
    {
        // Load chest data from binary mesh file
        min::mesh<float, uint16_t> mesh("chest");
        const min::mem_file &bmesh = memory_map::memory.get_file("data/models/chest.bmesh");
        mesh.from_file(bmesh);

        // Add mesh and update buffers
        const size_t iid = _buffer.add_mesh(mesh);

        // Load chest textures
        const min::mem_file &text = memory_map::memory.get_file("data/texture/chest.dds");
        const min::dds chest = min::dds(text);

        // Load dds into texture buffer
        const size_t tid = _texture_buffer.add_dds_texture(chest);

        // Create bounding box from mesh data
        const min::aabbox<float, min::vec4> box(mesh.vertex);
        const min::aabbox<float, min::vec3> box3(box.get_min(), box.get_max());

        // Add to asset buffer
        const size_t limit = _CHEST_LIMIT;
        _assets.emplace_back(iid, tid, 245, limit, box3);
    }
    inline void load_drone_model()
    {
        // Load drone data from binary mesh file
        min::mesh<float, uint16_t> mesh("drone");
        const min::mem_file &bmesh = memory_map::memory.get_file("data/models/drone.bmesh");
        mesh.from_file(bmesh);

        // Add mesh and update buffers
        const size_t iid = _buffer.add_mesh(mesh);

        // Load drone textures
        const min::mem_file &text = memory_map::memory.get_file("data/texture/drone.dds");
        const min::dds drone = min::dds(text);

        // Load dds into texture buffer
        const size_t tid = _texture_buffer.add_dds_texture(drone);

        // Create bounding box from mesh data
        const min::aabbox<float, min::vec4> box(mesh.vertex);
        const min::aabbox<float, min::vec3> box3(box.get_min(), box.get_max());

        // Add to asset buffer
        const size_t limit = _DRONE_LIMIT;
        _assets.emplace_back(iid, tid, 255, limit, box3);
    }
    inline void load_drop_explode_model()
    {
        // Load drop data from geometry functions
        min::mesh<float, uint16_t> mesh("drop");

        // Define the box min and max
        const min::vec3<float> min(-0.25, -0.25, -0.25);
        const min::vec3<float> max(0.25, 0.25, 0.25);

        // Allocate mesh space
        mesh.vertex.resize(24);
        mesh.uv.resize(24);
        mesh.normal.resize(24);
        mesh.index.resize(36);

        // Calculate block vertices
        block_vertex(mesh.vertex, 0, min, max);

        // Calculate block uv's
        block_uv(mesh.uv, 0);

        // Calculate block normals
        block_normal(mesh.normal, 0);

        // Calculate block indices
        block_index<uint16_t>(mesh.index, 0, 0);

        // Calculate tangents
        mesh.calculate_tangents();

        // Add mesh and update buffers
        const size_t iid = _buffer.add_mesh(mesh);

        // Load drop textures
        const min::mem_file &text = memory_map::memory.get_file("data/texture/atlas.dds");
        const min::dds drop = min::dds(text);

        // Load dds into texture buffer
        const size_t tid = _texture_buffer.add_dds_texture(drop);

        // Create bounding box from box dimensions
        const min::aabbox<float, min::vec3> box(min, max);

        // Add to asset buffer
        const size_t drop_limit = _DROP_LIMIT;
        _assets.emplace_back(iid, tid, 265, drop_limit, box);
        const size_t explode_limit = _EXPLODE_LIMIT;
        _assets.emplace_back(iid, tid, 315, explode_limit, box);
    }
    inline void load_missile_model()
    {
        // Load missile data from binary mesh file
        min::mesh<float, uint16_t> mesh("missile");
        const min::mem_file &bmesh = memory_map::memory.get_file("data/models/missile.bmesh");
        mesh.from_file(bmesh);

        // Add mesh and update buffers
        const size_t iid = _buffer.add_mesh(mesh);

        // Load missile textures
        const min::mem_file &text = memory_map::memory.get_file("data/texture/missile.dds");
        const min::dds missile = min::dds(text);

        // Load dds into texture buffer
        const size_t tid = _texture_buffer.add_dds_texture(missile);

        // Create bounding box from mesh data
        const min::aabbox<float, min::vec4> box(mesh.vertex);
        const min::aabbox<float, min::vec3> box3(box.get_min(), box.get_max());

        // Add to asset buffer
        const size_t limit = _MISS_LIMIT;
        _assets.emplace_back(iid, tid, 325, limit, box3);
    }
    inline void load_models()
    {
        // Load chest data
        load_chest_model();

        // Load drone data
        load_drone_model();

        // Load drop and explode data
        load_drop_explode_model();

        // Load missile data
        load_missile_model();

        // Unbind the last VAO to prevent scrambling buffers
        _buffer.unbind();

        // Load vertex buffer with data
        _buffer.upload();
    }
    inline void load_program_index(const game::uniforms &uniforms)
    {
        // Get the start_index uniform location
        _index_location = glGetUniformLocation(_prog.id(), "start_index");
        if (_index_location == -1)
        {
            throw std::runtime_error("static_instance: could not find uniform 'start_index'");
        }

        // Load the uniform buffer with the program we will use
        uniforms.set_program_lights(_prog);
        uniforms.set_program_matrix(_prog);
    }
    inline void reserve_memory()
    {
        _sort_index.reserve(_DROP_LIMIT);
        _assets.reserve(id_value(static_id::ASSET_SIZE));
    }
    inline void set_start_index(const GLint start_index) const
    {
        // Set the sampler active texture
        glUniform1i(_index_location, start_index);
    }

  public:
    static_instance(const game::uniforms &uniforms)
        : _vertex(memory_map::memory.get_file("data/shader/instance.vertex"), GL_VERTEX_SHADER),
          _fragment(memory_map::memory.get_file("data/shader/instance.fragment"), GL_FRAGMENT_SHADER),
          _prog(_vertex, _fragment)
    {
        // Since we are using a BMESH, assert floating point compatibility
        static_assert(std::numeric_limits<float>::is_iec559, "IEEE 754 float required");
        static_assert(sizeof(float) == 4, "32 bit IEEE 754 float required");

        // Reserve memory
        reserve_memory();

        // Load instance model
        load_models();

        // Load program index
        load_program_index(uniforms);
    }
    void draw(const game::uniforms &uniforms) const
    {
        // Bind VAO
        _buffer.bind();

        // Change program to instance shaders
        _prog.use();

        // Draw all assets in view
        const size_t size = _assets.size();
        for (size_t i = 0; i < size; i++)
        {
            // Get asset information
            const size_t iid = _assets[i].get_iid();
            const size_t tid = _assets[i].get_tid();
            const size_t index = _assets[i].get_start_index();
            const size_t asset_size = _assets[i].view_size();

            // Bind this texture for drawing on channel '0'
            _texture_buffer.bind(tid, 0);

            // Set the start index for chests
            set_start_index(index);

            // Draw mob instances
            _buffer.draw_many(GL_TRIANGLES, iid, asset_size);
        }
    }
    inline static_asset &get_chest()
    {
        const size_t id = id_value(static_id::CHEST);
        return _assets[id - 1];
    }
    inline const static_asset &get_chest() const
    {
        const size_t id = id_value(static_id::CHEST);
        return _assets[id - 1];
    }
    inline static_asset &get_drone()
    {
        const size_t id = id_value(static_id::DRONE);
        return _assets[id - 1];
    }
    inline const static_asset &get_drone() const
    {
        const size_t id = id_value(static_id::DRONE);
        return _assets[id - 1];
    }
    inline static_asset &get_drop()
    {
        const size_t id = id_value(static_id::DROP);
        return _assets[id - 1];
    }
    inline const static_asset &get_drop() const
    {
        const size_t id = id_value(static_id::DROP);
        return _assets[id - 1];
    }
    inline static_asset &get_explosive()
    {
        const size_t id = id_value(static_id::EXPLOSIVE);
        return _assets[id - 1];
    }
    inline const static_asset &get_explosive() const
    {
        const size_t id = id_value(static_id::EXPLOSIVE);
        return _assets[id - 1];
    }
    inline static_asset &get_missile()
    {
        const size_t id = id_value(static_id::MISSILE);
        return _assets[id - 1];
    }
    inline const static_asset &get_missile() const
    {
        const size_t id = id_value(static_id::MISSILE);
        return _assets[id - 1];
    }
    inline size_t get_inst_in_view() const
    {
        size_t count = 0;

        // Count instances in view
        const size_t size = _assets.size();
        for (size_t i = 0; i < size; i++)
        {
            count += _assets[i].view_size();
        }

        // Number of assets in view
        return count;
    }
    inline static constexpr size_t max_alloc()
    {
        return _CHEST_LIMIT + _DRONE_LIMIT + _DROP_LIMIT + _EXPLODE_LIMIT + _MISS_LIMIT;
    }
    inline static constexpr size_t max_chests()
    {
        return _CHEST_LIMIT;
    }
    inline static constexpr size_t max_drones()
    {
        return _DRONE_LIMIT;
    }
    inline static constexpr size_t max_drops()
    {
        return _DROP_LIMIT;
    }
    inline static constexpr size_t max_explosives()
    {
        return _EXPLODE_LIMIT;
    }
    inline static constexpr size_t max_missiles()
    {
        return _MISS_LIMIT;
    }
    void update(const physics &sim, const cgrid &grid, const min::camera<float> &cam)
    {
        // Clear out the asset index buffer
        const size_t size = _assets.size();
        for (size_t i = 0; i < size; i++)
        {
            _assets[i].clear_index();
        }

        // Decide culling strategy
        if (sim.get_scale() >= grid.get_chunk_scale())
        {
            cull_physics(sim, grid);
        }
        else
        {
            cull_frustum(grid, cam);
        }

        // Sort all assets and copy matrix output buffers
        for (size_t i = 0; i < size; i++)
        {
            // Sort and remove duplicates
            _assets[i].sort_prune_index(_sort_index);

            // Copy to output buffer
            _assets[i].copy_mat_index();
        }
    }
};
}

#endif
