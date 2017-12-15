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
#include <vector>

namespace game
{

class static_instance
{
  private:
    min::shader _vertex;
    min::shader _fragment;
    min::program _prog;
    GLint _index_location;

    // Buffers for model data and textures
    min::vertex_buffer<float, uint16_t, min::static_vertex, GL_FLOAT, GL_UNSIGNED_SHORT> _buffer;
    min::texture_buffer _texture_buffer;
    GLuint _cube_tid;
    GLuint _miss_tid;

    // Bounding box for instance model
    min::aabbox<float, min::vec3> _cube_box;
    min::aabbox<float, min::vec3> _miss_box;

    // Positions of each instance
    std::vector<min::mat4<float>> _cube_mat;
    std::vector<min::mat4<float>> _miss_mat;

    // Indices for each mesh
    size_t _cube_index;
    size_t _miss_index;

    inline void load_cube_model()
    {
        // Load cube data from binary mesh file
        min::mesh<float, uint16_t> cube_mesh("companion");
        cube_mesh.from_file("data/models/cube.bmesh");

        // Create bounding box from mesh data
        const min::aabbox<float, min::vec4> cube_box(cube_mesh.vertex);

        // Convert from vec4 to vec3
        _cube_box = min::aabbox<float, min::vec3>(cube_box.get_min(), cube_box.get_max());

        // Add mesh and update buffers
        _cube_index = _buffer.add_mesh(cube_mesh);
    }
    inline void load_missile_model()
    {
        // Load missile data from binary mesh file
        min::mesh<float, uint16_t> miss_mesh("missile");
        miss_mesh.from_file("data/models/missile.bmesh");

        // Create bounding box from mesh data
        const min::aabbox<float, min::vec4> miss_box(miss_mesh.vertex);

        // Convert from vec4 to vec3
        _miss_box = min::aabbox<float, min::vec3>(miss_box.get_min(), miss_box.get_max());

        // Add mesh and update buffers
        _miss_index = _buffer.add_mesh(miss_mesh);
    }

    inline void load_models()
    {
        // Load cube data
        load_cube_model();

        // Load missile data
        load_missile_model();

        // Load vertex buffer with data
        _buffer.upload();
    }
    inline void load_program_index()
    {
        // Get the start_index uniform location
        _index_location = glGetUniformLocation(_prog.id(), "start_index");
        if (_index_location == -1)
        {
            throw std::runtime_error("static_instance: could not find uniform 'start_index'");
        }
    }
    inline void load_textures()
    {
        // Load cube textures
        const min::dds cube = min::dds("data/texture/cube.dds");

        // Load dds into texture buffer
        _cube_tid = _texture_buffer.add_dds_texture(cube);

        // Load missile textures
        const min::dds missile = min::dds("data/texture/missile.dds");

        // Load dds into texture buffer
        _miss_tid = _texture_buffer.add_dds_texture(missile);
    }
    void set_start_index(const GLint start_index) const
    {
        // Set the sampler active texture
        glUniform1i(_index_location, start_index);
    }

  public:
    static_instance(const game::uniforms &uniforms)
        : _vertex("data/shader/instance.vertex", GL_VERTEX_SHADER),
          _fragment("data/shader/instance.fragment", GL_FRAGMENT_SHADER),
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
        load_program_index();

        // Load the uniform buffer with the program we will use
        uniforms.set_program(_prog);
    }
    size_t add_cube(const min::vec3<float> &p)
    {
        // Check for buffer overflow
        if (_cube_mat.size() == 10)
        {
            throw std::runtime_error("static_instance: must change default cube count");
        }

        // Push back location
        _cube_mat.push_back(p);

        // return mob id
        return _cube_mat.size() - 1;
    }
    size_t add_missile(const min::vec3<float> &p)
    {
        // Check for buffer overflow
        if (_miss_mat.size() == 10)
        {
            throw std::runtime_error("static_instance: must change default missile count");
        }

        // Push back location
        _miss_mat.push_back(p);

        // return mob id
        return _miss_mat.size() - 1;
    }
    void clear_missile()
    {
        _miss_mat.clear();
    }
    min::aabbox<float, min::vec3> box_cube(const size_t index) const
    {
        // Create box for this mob
        min::aabbox<float, min::vec3> box(_cube_box);

        // Move box to mob position
        box.set_position(_cube_mat[index].get_translation());

        // Return this box for collisions
        return box;
    }
    min::aabbox<float, min::vec3> box_missile(const size_t index) const
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
        // If we are going to draw anything
        if (_cube_mat.size() > 0 || _miss_mat.size() > 0)
        {
            // Activate the uniform buffer
            uniforms.bind();

            // Bind VAO
            _buffer.bind();

            // Change program to instance shaders
            _prog.use();
        }

        // Draw cubes
        if (_cube_mat.size() > 0)
        {
            // Bind this texture for drawing on channel '0'
            _texture_buffer.bind(_cube_tid, 0);

            // Set the start index for cubes
            set_start_index(5);

            // Draw mob instances
            const size_t size = _cube_mat.size();
            _buffer.draw_many(GL_TRIANGLES, _cube_index, size);
        }

        // Draw missiles
        if (_miss_mat.size() > 0)
        {
            // Bind this texture for drawing on channel '0'
            _texture_buffer.bind(_miss_tid, 0);

            // Set the start index for missiles
            set_start_index(15);

            // Draw missile instances
            const size_t size = _miss_mat.size();
            _buffer.draw_many(GL_TRIANGLES, _miss_index, size);
        }
    }
    const std::vector<min::mat4<float>> &get_cube_matrices() const
    {
        return _cube_mat;
    }
    const std::vector<min::mat4<float>> &get_missile_matrices() const
    {
        return _miss_mat;
    }
    size_t cube_size() const
    {
        return _cube_mat.size();
    }
    size_t missile_size() const
    {
        return _miss_mat.size();
    }
    void update_cube_position(const min::vec3<float> &p, const size_t index)
    {
        _cube_mat[index].set_translation(p);
    }
    void update_cube_rotation(const min::quat<float> &r, const size_t index)
    {
        _cube_mat[index].set_rotation(r);
    }
    void update_missile_position(const min::vec3<float> &p, const size_t index)
    {
        _miss_mat[index].set_translation(p);
    }
    void update_missile_rotation(const min::quat<float> &r, const size_t index)
    {
        _miss_mat[index].set_rotation(r);
    }
};
}

#endif
