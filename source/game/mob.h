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
#ifndef __STATIC_MOB__
#define __STATIC_MOB__

#include <min/aabbox.h>
#include <min/camera.h>
#include <min/mat4.h>
#include <min/program.h>
#include <min/shader.h>
#include <min/static_vertex.h>
#include <min/texture_buffer.h>
#include <min/uniform_buffer.h>
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

    // Camera and uniform data
    min::uniform_buffer<float> _ubuffer;
    size_t _proj_view_id;
    size_t _view_id;
    std::vector<size_t> _model_ids;

    // Light properties for rendering model
    min::vec4<float> _light_color;
    min::vec4<float> _light_position;
    min::vec4<float> _light_power;
    size_t _light_id;

    // Bounding box for model
    std::vector<min::vec3<float>> _position;

    inline static min::aabbox<float, min::vec3> create_box(const min::vec3<float> &p)
    {
        // create min and max edge of box
        const min::vec3<float> min = p - 0.25;
        const min::vec3<float> max = p + 0.25;

        // return box
        return min::aabbox<float, min::vec3>(min, max);
    }
    inline void load_model()
    {
        // load box model
        min::vec3<float> p;
        min::aabbox<float, min::vec3> box = create_box(p);
        min::mesh<float, uint16_t> box_mesh = min::to_mesh<float, uint16_t>(box);
        box_mesh.calculate_normals();

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
        const min::dds d = min::dds("data/texture/atlas.dds");

        // Load texture buffer
        _dds_id = _texture_buffer.add_dds_texture(d);
    }
    inline void load_uniforms()
    {
        // Load the uniform buffer with the program we will use
        _ubuffer.set_program(_prog);

        // Load light into uniform buffer
        _light_id = _ubuffer.add_light(min::light<float>(_light_color, _light_position, _light_power));

        // Load projection and view matrix into uniform buffer
        _proj_view_id = _ubuffer.add_matrix(min::mat4<float>());
        _view_id = _ubuffer.add_matrix(min::mat4<float>());

        // Update the matrix and light buffer
        _ubuffer.update();
    }

  public:
    mob_instance() : _vertex("data/shader/instance.vertex", GL_VERTEX_SHADER),
                     _fragment("data/shader/instance.fragment", GL_FRAGMENT_SHADER),
                     _prog(_vertex, _fragment),
                     _ubuffer(1, 10),
                     _light_color(1.0, 1.0, 1.0, 1.0),
                     _light_position(0.0, 100.0, 0.0, 1.0),
                     _light_power(0.5, 1.0, 0.75, 1.0)

    {
        // Load instance model
        load_model();

        // Load model textures
        load_textures();

        // Load the instance uniforms
        load_uniforms();
    }
    size_t add_mob(const min::vec3<float> &p)
    {
        // Get model ID for later use
        const size_t id = _ubuffer.add_matrix(min::mat4<float>(p));
        _model_ids.push_back(id);

        // Push back location
        _position.push_back(p);

        // return mob id
        return _position.size() - 1;
    }
    min::aabbox<float, min::vec3> mob_box(const size_t index) const
    {
        const min::vec3<float> &p = _position[index];
        return create_box(p);
    }
    void draw()
    {
        if (_position.size() > 0)
        {
            // Bind this uniform buffer for use
            _ubuffer.bind();

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
    size_t size() const
    {
        return _position.size();
    }
    void update_position(const min::vec3<float> &p, const size_t index)
    {
        _position[index] = p;
    }
    void update(min::camera<float> &cam)
    {
        if (_position.size() > 0)
        {
            // Update matrix uniforms
            _ubuffer.set_matrix(cam.get_pv_matrix(), _proj_view_id);
            _ubuffer.set_matrix(cam.get_v_matrix(), _view_id);

            // Upload all mob positions
            const size_t size = _position.size();
            for (size_t i = 0; i < size; i++)
            {
                // Get mob matrix id
                const size_t id = _model_ids[i];

                // Update position matrix
                const min::mat4<float> m(_position[i]);
                _ubuffer.set_matrix(m, id);
            }

            // Update the matrix and light buffer
            _ubuffer.update_matrix();
        }
    }
};
}

#endif
