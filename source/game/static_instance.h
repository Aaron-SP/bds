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
#ifndef __STATIC_INSTANCE__
#define __STATIC_INSTANCE__

#include <game/geometry.h>
#include <game/memory_map.h>
#include <game/uniforms.h>
#include <min/aabbox.h>
#include <min/camera.h>
#include <min/mat4.h>
#include <min/program.h>
#include <min/shader.h>
#include <min/static_vertex.h>
#include <min/texture_buffer.h>
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
    static constexpr size_t _DRONE_LIMIT = 10;
    static constexpr size_t _DROP_LIMIT = 10;
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
    GLuint _miss_tid;

    // Bounding box for instance model
    min::aabbox<float, min::vec3> _drone_box;
    min::aabbox<float, min::vec3> _drop_box;
    min::aabbox<float, min::vec3> _miss_box;

    // Positions of each instance
    std::vector<min::mat4<float>> _drone_mat;
    std::vector<min::mat4<float>> _drop_mat;
    std::vector<min::mat4<float>> _miss_mat;

    // Indices for each mesh
    size_t _drone_index;
    size_t _drop_index;
    size_t _miss_index;

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
        _drone_index = _buffer.add_mesh(drone_mesh);
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
        _drop_index = _buffer.add_mesh(drop_mesh);
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
        _miss_index = _buffer.add_mesh(miss_mesh);
    }
    inline void load_models()
    {
        // Load drone data
        load_drone_model();

        // Load drop data
        load_drop_model();

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
        _drop_tid = _texture_buffer.add_dds_texture(drop);

        // Load missile textures
        const min::mem_file &miss_file = memory_map::memory.get_file("data/texture/missile.dds");
        const min::dds missile = min::dds(miss_file);

        // Load dds into texture buffer
        _miss_tid = _texture_buffer.add_dds_texture(missile);
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
    inline void clear_drop(const size_t index)
    {
        // Erase matrix at iterator
        _drop_mat.erase(_drop_mat.begin() + index);
    }
    inline void clear_drops()
    {
        _drop_mat.clear();
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
    inline void clear_missile(const size_t index)
    {
        // Erase matrix at iterator
        _miss_mat.erase(_miss_mat.begin() + index);
    }
    inline void clear_missiles()
    {
        _miss_mat.clear();
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
    inline min::aabbox<float, min::vec3> box_missile(const size_t index) const
    {
        // Create box for this mob
        min::aabbox<float, min::vec3> box(_miss_box);

        // Move box to mob position
        box.set_position(_miss_mat[index].get_translation());

        // Return this box for collisions
        return box;
    }
    void draw(const game::uniforms &uniforms) const
    {
        // Bind VAO
        _buffer.bind();

        // Change program to instance shaders
        _prog.use();

        // Draw drones
        if (_drone_mat.size() > 0)
        {
            // Bind this texture for drawing on channel '0'
            _texture_buffer.bind(_drone_tid, 0);

            // Set the start index for drones
            set_start_index(145);

            // Draw mob instances
            const size_t size = _drone_mat.size();
            _buffer.draw_many(GL_TRIANGLES, _drone_index, size);
        }

        // Draw drops
        if (_drop_mat.size() > 0)
        {
            // Bind this texture for drawing on channel '0'
            _texture_buffer.bind(_drop_tid, 0);

            // Set the start index for drops
            set_start_index(155);

            // Draw mob instances
            const size_t size = _drop_mat.size();
            _buffer.draw_many(GL_TRIANGLES, _drop_index, size);
        }

        // Draw missiles
        if (_miss_mat.size() > 0)
        {
            // Bind this texture for drawing on channel '0'
            _texture_buffer.bind(_miss_tid, 0);

            // Set the start index for missiles
            set_start_index(165);

            // Draw missile instances
            const size_t size = _miss_mat.size();
            _buffer.draw_many(GL_TRIANGLES, _miss_index, size);
        }
    }
    inline const std::vector<min::mat4<float>> &get_drone_matrices() const
    {
        return _drone_mat;
    }
    inline const std::vector<min::mat4<float>> &get_drop_matrices() const
    {
        return _drop_mat;
    }
    inline const std::vector<min::mat4<float>> &get_missile_matrices() const
    {
        return _miss_mat;
    }
    inline bool drone_full() const
    {
        return _drone_mat.size() == _DRONE_LIMIT;
    }
    inline size_t drone_size() const
    {
        return _drone_mat.size();
    }
    inline bool drop_full() const
    {
        return _drop_mat.size() == _DROP_LIMIT;
    }
    inline size_t drop_size() const
    {
        return _drop_mat.size();
    }
    inline bool missile_full() const
    {
        return _miss_mat.size() == _MISS_LIMIT;
    }
    inline size_t missile_size() const
    {
        return _miss_mat.size();
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
