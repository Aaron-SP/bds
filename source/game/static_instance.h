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
#include <game/memory_map.h>
#include <game/uniforms.h>
#include <min/aabbox.h>
#include <min/camera.h>
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

class static_instance
{
  private:
    typedef min::physics<float, uint16_t, uint32_t, min::vec3, min::aabbox, min::aabbox, min::grid> physics;
    static constexpr size_t _DRONE_LIMIT = 10;
    static constexpr size_t _DROP_LIMIT = 10;
    static constexpr size_t _EXPLODE_LIMIT = 10;
    static constexpr size_t _MISS_LIMIT = 10;

    min::shader _vertex;
    min::shader _fragment;
    min::program _prog;
    GLint _index_location;

    // Buffers for model data and textures
    min::vertex_buffer<float, uint16_t, min::static_vertex, GL_FLOAT, GL_UNSIGNED_SHORT> _buffer;
    min::texture_buffer _texture_buffer;
    GLuint _drone_tid;
    GLuint _drop_tid;
    GLuint _explode_tid;
    GLuint _miss_tid;

    // Bounding box for instance model
    min::aabbox<float, min::vec3> _drone_box;
    min::aabbox<float, min::vec3> _drop_box;
    min::aabbox<float, min::vec3> _explode_box;
    min::aabbox<float, min::vec3> _miss_box;

    // Positions of each instance
    std::vector<size_t> _sort_index;
    std::vector<size_t> _drone_index;
    std::vector<min::mat4<float>> _drone_mat;
    std::vector<min::mat4<float>> _drone_mat_out;
    std::vector<size_t> _drop_index;
    std::vector<min::mat4<float>> _drop_mat;
    std::vector<min::mat4<float>> _drop_mat_out;
    std::vector<size_t> _explode_index;
    std::vector<min::mat4<float>> _explode_mat;
    std::vector<min::mat4<float>> _explode_mat_out;
    std::vector<size_t> _miss_index;
    std::vector<min::mat4<float>> _miss_mat;
    std::vector<min::mat4<float>> _miss_mat_out;

    // Indices for each mesh
    size_t _drone_inst;
    size_t _drop_inst;
    size_t _explode_inst;
    size_t _miss_inst;

    inline void copy_mat_index(std::vector<min::mat4<float>> &mat, std::vector<min::mat4<float>> &mat_out, std::vector<size_t> &index)
    {
        // Cull empty index buffer
        const size_t size = index.size();
        if (size > 0)
        {
            // Resize output buffer
            mat_out.resize(size);

            // Copy elements from index buffer
            for (size_t i = 0; i < size; i++)
            {
                mat_out[i] = mat[index[i]];
            }
        }
        else
        {
            mat_out.clear();
        }
    }
    inline void cull_frustum(const min::camera<float> &cam)
    {
        // Cull drones
        const size_t drone_size = _drone_mat.size();
        for (size_t i = 0; i < drone_size; i++)
        {
            // Create drone bounding box from matrix position
            const min::aabbox<float, min::vec3> box = box_drone(i);

            // If the box is within the frustum
            if (min::intersect<float>(cam.get_frustum(), box))
            {
                _drone_index.push_back(i);
            }
        }

        // Cull drops
        const size_t drop_size = _drop_mat.size();
        for (size_t i = 0; i < drop_size; i++)
        {
            // Create drop bounding box from matrix position
            const min::aabbox<float, min::vec3> box = box_drop(i);

            // If the box is within the frustum
            if (min::intersect<float>(cam.get_frustum(), box))
            {
                _drop_index.push_back(i);
            }
        }

        // Cull explosives
        const size_t explode_size = _explode_mat.size();
        for (size_t i = 0; i < explode_size; i++)
        {
            // Create explosive bounding box from matrix position
            const min::aabbox<float, min::vec3> box = box_explosive(i);

            // If the box is within the frustum
            if (min::intersect<float>(cam.get_frustum(), box))
            {
                _explode_index.push_back(i);
            }
        }

        // Cull missiles
        const size_t miss_size = _miss_mat.size();
        for (size_t i = 0; i < miss_size; i++)
        {
            // Create missile bounding box from matrix position
            const min::aabbox<float, min::vec3> box = box_missile(i);

            // If the box is within the frustum
            if (min::intersect<float>(cam.get_frustum(), box))
            {
                _miss_index.push_back(i);
            }
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
                const uint16_t body_index = map[over[i].first];
                const min::body<float, min::vec3> &b = sim.get_body(body_index);

                // If the body is not dead
                if (!b.is_dead())
                {
                    // Get the index of the type
                    const size_t index = b.get_data().index;

                    // Get the type of body
                    const size_t id = b.get_id();
                    switch (id)
                    {
                    case 1:
                        _drone_index.push_back(index);
                        break;
                    case 2:
                        _drop_index.push_back(index);
                        break;
                    case 3:
                        _explode_index.push_back(index);
                        break;
                    case 4:
                        _miss_index.push_back(index);
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
    inline void load_drone_model()
    {
        // Load drone data from binary mesh file
        min::mesh<float, uint16_t> drone_mesh("drone");
        const min::mem_file &drone_file = memory_map::memory.get_file("data/models/drone.bmesh");
        drone_mesh.from_file(drone_file);

        // Create bounding box from mesh data
        const min::aabbox<float, min::vec4> drone_box(drone_mesh.vertex);

        // Convert from vec4 to vec3
        _drone_box = min::aabbox<float, min::vec3>(drone_box.get_min(), drone_box.get_max());

        // Add mesh and update buffers
        _drone_inst = _buffer.add_mesh(drone_mesh);
    }
    inline void load_drop_model()
    {
        // Load drop data from geometry functions
        min::mesh<float, uint16_t> drop_mesh("drop");

        // Define the box min and max
        const min::vec3<float> min(-0.25, -0.25, -0.25);
        const min::vec3<float> max(0.25, 0.25, 0.25);

        // Allocate mesh space
        drop_mesh.vertex.resize(24);
        drop_mesh.uv.resize(24);
        drop_mesh.normal.resize(24);
        drop_mesh.index.resize(36);

        // Calculate block vertices
        block_vertex(drop_mesh.vertex, 0, min, max);

        // Calculate block uv's
        block_uv(drop_mesh.uv, 0);

        // Calculate block normals
        block_normal(drop_mesh.normal, 0);

        // Calculate block indices
        block_index<uint16_t>(drop_mesh.index, 0, 0);

        // Calculate tangents
        drop_mesh.calculate_tangents();

        // Create bounding box from mesh data
        const min::aabbox<float, min::vec4> drop_box(drop_mesh.vertex);

        // Convert from vec4 to vec3
        _drop_box = min::aabbox<float, min::vec3>(min, max);

        // Add mesh and update buffers
        _drop_inst = _buffer.add_mesh(drop_mesh);
    }
    inline void load_explode_model()
    {
        // Use the drop box and model for explode
        _explode_box = _drop_box;
        _explode_inst = _drop_inst;
    }
    inline void load_missile_model()
    {
        // Load missile data from binary mesh file
        min::mesh<float, uint16_t> miss_mesh("missile");
        const min::mem_file &miss_file = memory_map::memory.get_file("data/models/missile.bmesh");
        miss_mesh.from_file(miss_file);

        // Create bounding box from mesh data
        const min::aabbox<float, min::vec4> miss_box(miss_mesh.vertex);

        // Convert from vec4 to vec3
        _miss_box = min::aabbox<float, min::vec3>(miss_box.get_min(), miss_box.get_max());

        // Add mesh and update buffers
        _miss_inst = _buffer.add_mesh(miss_mesh);
    }
    inline void load_models()
    {
        // Load drone data
        load_drone_model();

        // Load drop data
        load_drop_model();

        // Load explode model
        load_explode_model();

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
    inline void load_textures()
    {
        // Load drone textures
        const min::mem_file &drone_file = memory_map::memory.get_file("data/texture/drone.dds");
        const min::dds drone = min::dds(drone_file);

        // Load dds into texture buffer
        _drone_tid = _texture_buffer.add_dds_texture(drone);

        // Load drop textures
        const min::mem_file &drop_file = memory_map::memory.get_file("data/texture/atlas.dds");
        const min::dds drop = min::dds(drop_file);

        // Load dds into texture buffer
        _drop_tid = _explode_tid = _texture_buffer.add_dds_texture(drop);

        // Load missile textures
        const min::mem_file &miss_file = memory_map::memory.get_file("data/texture/missile.dds");
        const min::dds missile = min::dds(miss_file);

        // Load dds into texture buffer
        _miss_tid = _texture_buffer.add_dds_texture(missile);
    }
    inline void reserve_memory()
    {
        // Sorting
        _sort_index.reserve(_DROP_LIMIT);

        // Drones
        _drone_index.reserve(_DRONE_LIMIT);
        _drone_mat.reserve(_DRONE_LIMIT);
        _drone_mat_out.reserve(_DRONE_LIMIT);

        // Drops
        _drop_index.reserve(_DROP_LIMIT);
        _drop_mat.reserve(_DROP_LIMIT);
        _drop_mat_out.reserve(_DROP_LIMIT);

        // Explosives
        _explode_index.reserve(_EXPLODE_LIMIT);
        _explode_mat.reserve(_EXPLODE_LIMIT);
        _explode_mat_out.reserve(_EXPLODE_LIMIT);

        // Missiles
        _miss_index.reserve(_MISS_LIMIT);
        _miss_mat.reserve(_MISS_LIMIT);
        _miss_mat_out.reserve(_MISS_LIMIT);
    }
    inline void set_start_index(const GLint start_index) const
    {
        // Set the sampler active texture
        glUniform1i(_index_location, start_index);
    }
    inline void sort_prune_index(std::vector<size_t> &index)
    {
        // Cull empty index buffer
        if (index.size() > 0)
        {
            // Sort index keys using a radix sort
            min::uint_sort<size_t>(index, _sort_index, [](const size_t i) {
                return i;
            });

            // Make keys unique
            const auto last = std::unique(index.begin(), index.end());

            // Erase empty spaces in vector
            index.erase(last, index.end());
        }
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

        // Load model textures
        load_textures();

        // Load program index
        load_program_index(uniforms);
    }
    inline size_t add_drone(const min::vec3<float> &p)
    {
        // Check for buffer overflow
        if (_drone_mat.size() == _DRONE_LIMIT)
        {
            throw std::runtime_error("static_instance: must change default drone count");
        }

        // Push back location
        _drone_mat.push_back(p);

        // return mob id
        return _drone_mat.size() - 1;
    }
    inline size_t add_drop(const min::vec3<float> &p, const int8_t atlas)
    {
        // Check for buffer overflow
        if (_drop_mat.size() == _DROP_LIMIT)
        {
            throw std::runtime_error("static_instance: must change default drop count");
        }

        // Push back location
        _drop_mat.push_back(p);

        // Pack the matrix with the atlas id
        _drop_mat.back().w(atlas + 2.1);

        // return mob id
        return _drop_mat.size() - 1;
    }
    inline size_t add_explosive(const min::vec3<float> &p, const int8_t atlas)
    {
        // Check for buffer overflow
        if (_explode_mat.size() == _EXPLODE_LIMIT)
        {
            throw std::runtime_error("static_instance: must change default explode count");
        }

        // Push back location
        _explode_mat.push_back(p);

        // Pack the matrix with the atlas id
        _explode_mat.back().w(atlas + 2.1);

        // return mob id
        return _explode_mat.size() - 1;
    }
    inline size_t add_missile(const min::vec3<float> &p)
    {
        // Check for buffer overflow
        if (_miss_mat.size() == _MISS_LIMIT)
        {
            throw std::runtime_error("static_instance: must change default missile count");
        }

        // Push back location
        _miss_mat.push_back(p);

        // return mob id
        return _miss_mat.size() - 1;
    }
    inline min::aabbox<float, min::vec3> box_drone(const size_t index) const
    {
        // Create box for this mob
        min::aabbox<float, min::vec3> box(_drone_box);

        // Move box to mob position
        box.set_position(_drone_mat[index].get_translation());

        // Return this box for collisions
        return box;
    }
    inline min::aabbox<float, min::vec3> box_drop(const size_t index) const
    {
        // Create box for this mob
        min::aabbox<float, min::vec3> box(_drop_box);

        // Move box to mob position
        box.set_position(_drop_mat[index].get_translation());

        // Return this box for collisions
        return box;
    }
    inline min::aabbox<float, min::vec3> box_explosive(const size_t index) const
    {
        // Create box for this mob
        min::aabbox<float, min::vec3> box(_explode_box);

        // Move box to mob position
        box.set_position(_explode_mat[index].get_translation());

        // Return this box for collisions
        return box;
    }
    inline min::aabbox<float, min::vec3> box_missile(const size_t index) const
    {
        // Create box for this mob
        min::aabbox<float, min::vec3> box(_miss_box);

        // Move box to mob position
        box.set_position(_miss_mat[index].get_translation());

        // Return this box for collisions
        return box;
    }
    inline void clear_drone(const size_t index)
    {
        // Erase matrix at iterator
        _drone_mat.erase(_drone_mat.begin() + index);
    }
    inline void clear_drones()
    {
        _drone_mat.clear();
    }
    inline void clear_drop(const size_t index)
    {
        // Erase matrix at iterator
        _drop_mat.erase(_drop_mat.begin() + index);
    }
    inline void clear_drops()
    {
        _drop_mat.clear();
    }
    inline void clear_explosive(const size_t index)
    {
        // Erase matrix at iterator
        _explode_mat.erase(_explode_mat.begin() + index);
    }
    inline void clear_explosives()
    {
        _explode_mat.clear();
    }
    inline void clear_missile(const size_t index)
    {
        // Erase matrix at iterator
        _miss_mat.erase(_miss_mat.begin() + index);
    }
    inline void clear_missiles()
    {
        _miss_mat.clear();
    }
    void draw(const game::uniforms &uniforms) const
    {
        // Bind VAO
        _buffer.bind();

        // Change program to instance shaders
        _prog.use();

        // Draw drones
        const size_t drone_size = _drone_mat_out.size();
        if (drone_size > 0)
        {
            // Bind this texture for drawing on channel '0'
            _texture_buffer.bind(_drone_tid, 0);

            // Set the start index for drones
            set_start_index(225);

            // Draw mob instances
            _buffer.draw_many(GL_TRIANGLES, _drone_inst, drone_size);
        }

        // Draw drops
        const size_t drop_size = _drop_mat_out.size();
        if (drop_size > 0)
        {
            // Bind this texture for drawing on channel '0'
            _texture_buffer.bind(_drop_tid, 0);

            // Set the start index for drops
            set_start_index(235);

            // Draw mob instances
            _buffer.draw_many(GL_TRIANGLES, _drop_inst, drop_size);
        }

        // Draw explosives
        const size_t explode_size = _explode_mat_out.size();
        if (explode_size > 0)
        {
            // Bind this texture for drawing on channel '0'
            _texture_buffer.bind(_explode_tid, 0);

            // Set the start index for drops
            set_start_index(245);

            // Draw mob instances
            _buffer.draw_many(GL_TRIANGLES, _explode_inst, explode_size);
        }

        // Draw missiles
        const size_t miss_size = _miss_mat_out.size();
        if (miss_size)
        {
            // Bind this texture for drawing on channel '0'
            _texture_buffer.bind(_miss_tid, 0);

            // Set the start index for missiles
            set_start_index(255);

            // Draw missile instances
            _buffer.draw_many(GL_TRIANGLES, _miss_inst, miss_size);
        }
    }
    inline bool drone_full() const
    {
        return _drone_mat.size() == _DRONE_LIMIT;
    }
    inline static constexpr size_t max_alloc()
    {
        return _DRONE_LIMIT + _DROP_LIMIT + _EXPLODE_LIMIT + _MISS_LIMIT;
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
    inline bool drop_full() const
    {
        return _drop_mat.size() == _DROP_LIMIT;
    }
    inline bool explosive_full() const
    {
        return _explode_mat.size() == _EXPLODE_LIMIT;
    }
    inline bool missile_full() const
    {
        return _miss_mat.size() == _MISS_LIMIT;
    }
    inline const std::vector<min::mat4<float>> &get_drone_matrix() const
    {
        return _drone_mat_out;
    }
    inline const std::vector<min::mat4<float>> &get_drop_matrix() const
    {
        return _drop_mat_out;
    }
    inline const std::vector<min::mat4<float>> &get_explosive_matrix() const
    {
        return _explode_mat_out;
    }
    inline const std::vector<min::mat4<float>> &get_missile_matrix() const
    {
        return _miss_mat_out;
    }
    inline size_t get_inst_in_view() const
    {
        return _drone_mat_out.size() + _drop_mat_out.size() + _explode_mat_out.size() + _miss_mat_out.size();
    }
    void update(const physics &sim, const cgrid &grid, const min::camera<float> &cam)
    {
        // Clear out the output buffers
        _drone_index.clear();
        _drop_index.clear();
        _explode_index.clear();
        _miss_index.clear();

        if (sim.get_scale() >= 8)
        {
            cull_physics(sim, grid);
        }
        else
        {
            cull_frustum(cam);
        }

        // Sort drones and remove duplicates
        sort_prune_index(_drone_index);

        // Sort drops and remove duplicates
        sort_prune_index(_drop_index);

        // Sort explosives and remove duplicates
        sort_prune_index(_explode_index);

        // Sort missiles and remove duplicates
        sort_prune_index(_miss_index);

        // Copy drones to output buffer
        copy_mat_index(_drone_mat, _drone_mat_out, _drone_index);

        // Copy drops to output buffer
        copy_mat_index(_drop_mat, _drop_mat_out, _drop_index);

        // Copy explosives to output buffer
        copy_mat_index(_explode_mat, _explode_mat_out, _explode_index);

        // Copy missiles to output buffer
        copy_mat_index(_miss_mat, _miss_mat_out, _miss_index);
    }
    inline void update_drone_position(const size_t index, const min::vec3<float> &p)
    {
        _drone_mat[index].set_translation(p);
    }
    inline void update_drone_rotation(const size_t index, const min::quat<float> &r)
    {
        _drone_mat[index].set_rotation(r);
    }
    inline void update_drop_atlas(const size_t index, const int8_t atlas)
    {
        _drop_mat[index].w(atlas + 2.1);
    }
    inline void update_drop_position(const size_t index, const min::vec3<float> &p)
    {
        _drop_mat[index].set_translation(p);
    }
    inline void update_drop_rotation(const size_t index, const min::quat<float> &r)
    {
        _drop_mat[index].set_rotation(r);
    }
    inline void update_explosive_position(const size_t index, const min::vec3<float> &p)
    {
        _explode_mat[index].set_translation(p);
    }
    inline void update_explosive_rotation(const size_t index, const min::quat<float> &r)
    {
        _explode_mat[index].set_rotation(r);
    }
    inline void update_missile_position(const size_t index, const min::vec3<float> &p)
    {
        _miss_mat[index].set_translation(p);
    }
    inline void update_missile_rotation(const size_t index, const min::quat<float> &r)
    {
        _miss_mat[index].set_rotation(r);
    }
};
}

#endif
