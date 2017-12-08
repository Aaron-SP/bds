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
#ifndef __STATIC_MOB__
#define __STATIC_MOB__

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
#include <vector>

namespace game
{

class mob_instance
{
  private:
    min::shader _vertex;
    min::shader _fragment;
    min::program _prog;

    // Buffers for model data and textures
    min::vertex_buffer<float, uint16_t, min::static_vertex, GL_FLOAT, GL_UNSIGNED_SHORT> _buffer;
    min::texture_buffer _texture_buffer;
    GLuint _dds_id;

    // Bounding box for mob model
    min::aabbox<float, min::vec3> _box;

    // Positions of each mob
    std::vector<min::vec3<float>> _position;

    inline void load_model()
    {
        // Since we are using a BMESH, assert floating point compatibility
        static_assert(std::numeric_limits<float>::is_iec559, "IEEE 754 float required");
        static_assert(sizeof(float) == 4, "32 bit IEEE 754 float required");

        // Create empty companion box mesh
        min::mesh<float, uint16_t> box_mesh("companion");

        // Load cube data from binary mesh file
        box_mesh.from_file("data/models/art_cube.bmesh");

        // Create bounding box from mesh data
        min::aabbox<float, min::vec4> box(box_mesh.vertex);

        // Convert from vec4 to vec3
        _box = min::aabbox<float, min::vec3>(box.get_min(), box.get_max());

        // Bind VAO
        _buffer.bind();

        // Add mesh and update buffers
        _buffer.add_mesh(box_mesh);

        // Load vertex buffer with data
        _buffer.upload();
    }
    inline void load_textures()
    {
        // Load textures
        const min::dds d = min::dds("data/texture/art_cube.dds");

        // Load texture buffer
        _dds_id = _texture_buffer.add_dds_texture(d);
    }

  public:
    mob_instance(const game::uniforms &uniforms)
        : _vertex("data/shader/instance.vertex", GL_VERTEX_SHADER),
          _fragment("data/shader/instance.fragment", GL_FRAGMENT_SHADER),
          _prog(_vertex, _fragment)
    {
        // Load instance model
        load_model();

        // Load model textures
        load_textures();

        // Load the uniform buffer with the program we will use
        uniforms.set_program(_prog);
    }
    size_t add_mob(const min::vec3<float> &p)
    {
        // Check for buffer overflow
        if (_position.size() == 10)
        {
            throw std::runtime_error("mob: must change default mob count");
        }

        // Push back location
        _position.push_back(p);

        // return mob id
        return _position.size() - 1;
    }
    min::aabbox<float, min::vec3> mob_box(const size_t index) const
    {
        // Get mob position
        const min::vec3<float> &p = _position[index];

        // Create box for this mob
        min::aabbox<float, min::vec3> box(_box);

        // Move box to mob position
        box.set_position(p);

        // Return this box for collisions
        return box;
    }
    void draw(const game::uniforms &uniforms)
    {
        if (_position.size() > 0)
        {
            // Activate the uniform buffer
            uniforms.bind();

            // Bind VAO
            _buffer.bind();

            // Bind this texture for drawing on channel '0'
            _texture_buffer.bind(_dds_id, 0);

            // Change program back to instance shaders
            _prog.use();

            // Draw mob instances
            const size_t size = _position.size();
            _buffer.draw_many(GL_TRIANGLES, 0, size);
        }
    }
    const std::vector<min::vec3<float>> &get_positions() const
    {
        return _position;
    }
    size_t size() const
    {
        return _position.size();
    }
    void update_position(const min::vec3<float> &p, const size_t index)
    {
        _position[index] = p;
    }
};
}

#endif
