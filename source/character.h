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
#ifndef __MD5_CHARACTER__
#define __MD5_CHARACTER__

#include <min/aabbox.h>
#include <min/camera.h>
#include <min/mat4.h>
#include <min/md5_anim.h>
#include <min/md5_model.h>
#include <min/program.h>
#include <min/shader.h>
#include <min/skeletal_vertex.h>
#include <min/texture_buffer.h>
#include <min/uniform_buffer.h>
#include <min/vec3.h>
#include <min/vertex_buffer.h>

namespace game
{

class character
{
  private:
    min::shader _vertex;
    min::shader _fragment;
    min::program _prog;

    // md5 model
    min::md5_model<float, uint32_t, min::vec4, min::aabbox> _md5_model;

    // Buffers for model data and textures
    min::vertex_buffer<float, uint32_t, min::skeletal_vertex, GL_FLOAT, GL_UNSIGNED_INT> _skbuffer;
    min::texture_buffer _texture_buffer;
    GLuint _dds_id;

    // Camera and uniform data
    min::uniform_buffer<float> _ubuffer;
    size_t _proj_view_id;
    size_t _view_id;
    size_t _model_id;

    // Light properties for rendering model
    min::vec4<float> _light_color;
    min::vec4<float> _light_position;
    min::vec4<float> _light_power;
    size_t _light_id;

    // For animating md5 model bones
    std::vector<size_t> _bone_id;

    // Bounding box for model
    min::mat4<float> _model_matrix;
    min::aabbox<float, min::vec3> _box;

    void load_model()
    {
        // Load animation
        _md5_model.load_animation("data/models/gun.md5anim");

        // Setup the md5 mesh
        min::mesh<float, uint32_t> &md5 = _md5_model.get_meshes()[0];
        md5.calculate_normals();

        // Get bounding box of first frame of animation
        _box = _md5_model.get_current_animation().get_bounds()[0];

        // Bind VAO
        _skbuffer.bind();

        // Add mesh and update buffers
        _skbuffer.add_mesh(md5);

        // Load vertex buffer with data
        _skbuffer.upload();
    }
    void load_textures()
    {
        // Load textures
        const min::dds d = min::dds("data/texture/skin.dds");

        // Set the texture channel for this program, we need to do this here because we render textures on channel '0'
        // _prog will be in use by the end of this call
        _texture_buffer.set_texture_uniform(_prog, "in_texture", 1);

        // Load texture buffer
        _dds_id = _texture_buffer.add_dds_texture(d);
    }
    void load_uniforms()
    {
        // Load the uniform buffer with the program we will use
        _ubuffer.set_program(_prog);

        // Load light into uniform buffer
        _light_id = _ubuffer.add_light(min::light<float>(_light_color, _light_position, _light_power));

        // Load projection and view matrix into uniform buffer
        _proj_view_id = _ubuffer.add_matrix(min::mat4<float>());
        _view_id = _ubuffer.add_matrix(min::mat4<float>());

        // Get model ID for later use
        _model_id = _ubuffer.add_matrix(_model_matrix);

        // Add bones matrices to uniform buffer
        for (const auto &bone : _md5_model.get_bones())
        {
            size_t bone_id = _ubuffer.add_matrix(bone);
            _bone_id.push_back(bone_id);
        }

        // Update the matrix and light buffer
        _ubuffer.update();
    }

  public:
    character() : _vertex("data/shader/md5.vertex", GL_VERTEX_SHADER),
                  _fragment("data/shader/md5.fragment", GL_FRAGMENT_SHADER),
                  _prog(_vertex, _fragment),
                  _md5_model(std::move(min::md5_mesh<float, uint32_t>("data/models/gun.md5mesh"))),
                  _ubuffer(1, 100),
                  _light_color(1.0, 1.0, 1.0, 1.0),
                  _light_position(0.0, 100.0, 0.0, 1.0),
                  _light_power(0.5, 1.0, 0.75, 1.0)

    {
        // Load md5 model
        load_model();

        // Load md5 model textures
        load_textures();

        // Load the md5 uniforms
        load_uniforms();
    }
    void draw(min::camera<float> &cam, const double time_step)
    {
        // Update matrix uniforms
        _ubuffer.set_matrix(cam.get_pv_matrix(), _proj_view_id);
        _ubuffer.set_matrix(cam.get_v_matrix(), _view_id);
        _ubuffer.set_matrix(_model_matrix, _model_id);

        // Only update bones if model is animating them
        if (_md5_model.is_animating())
        {
            // Update the md5 animation
            _md5_model.step(time_step);

            // update model bones matrices
            const auto &bones = _md5_model.get_bones();
            const size_t size = bones.size();
            for (size_t i = 0; i < size; i++)
            {
                _ubuffer.set_matrix(bones[i], _bone_id[i]);
            }
        }

        // Bind this uniform buffer for use
        _ubuffer.bind();

        // Update the matrix and light buffer
        _ubuffer.update_matrix();

        // Bind VAO
        _skbuffer.bind();

        // Bind this texture for drawing on channel '0'
        _texture_buffer.bind(_dds_id, 1);

        // Change program back to md5 shaders
        _prog.use();

        // Draw md5 model
        _skbuffer.draw(GL_TRIANGLES, 0);
    }
    const min::aabbox<float, min::vec3> &get_box() const
    {
        return _box;
    }
    void set_model_matrix(const min::mat4<float> &m)
    {
        _model_matrix = m;
    }
    void set_animation_count(const unsigned count)
    {
        _md5_model.get_current_animation().set_loop_count(count);
        _md5_model.get_current_animation().set_time(0);
    }
};
}

#endif
