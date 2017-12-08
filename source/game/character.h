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
#ifndef __MD5_CHARACTER__
#define __MD5_CHARACTER__

#include <game/particle.h>
#include <min/aabbox.h>
#include <min/camera.h>
#include <min/mat4.h>
#include <min/md5_anim.h>
#include <min/md5_model.h>
#include <min/program.h>
#include <min/shader.h>
#include <min/skeletal_vertex.h>
#include <min/texture_buffer.h>
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

    // Particle system
    particle *_particles;

    // Animation indices
    bool _need_bone_reset;
    size_t _charge_index;
    size_t _shoot_index;

    inline void load_model()
    {
        // Load animation
        _charge_index = _md5_model.load_animation("data/models/gun_charge.md5anim");

        // Load shoot animation
        _shoot_index = _md5_model.load_animation("data/models/gun_shoot.md5anim");

        // Setup the md5 mesh
        min::mesh<float, uint32_t> &md5 = _md5_model.get_meshes()[0];
        md5.calculate_normals();

        // Bind VAO
        _skbuffer.bind();

        // Add mesh and update buffers
        _skbuffer.add_mesh(md5);

        // Load vertex buffer with data
        _skbuffer.upload();
    }
    inline void load_textures()
    {
        // Load textures
        const min::dds d = min::dds("data/texture/skin.dds");

        // Load texture buffer
        _dds_id = _texture_buffer.add_dds_texture(d);
    }

  public:
    character(particle *const particles, const game::uniforms &uniforms)
        : _vertex("data/shader/md5.vertex", GL_VERTEX_SHADER),
          _fragment("data/shader/md5.fragment", GL_FRAGMENT_SHADER),
          _prog(_vertex, _fragment),
          _md5_model(std::move(min::md5_mesh<float, uint32_t>("data/models/gun.md5mesh"))),
          _particles(particles),
          _need_bone_reset(false)
    {
        // Load md5 model
        load_model();

        // Load md5 model textures
        load_textures();

        // Load the uniform buffer with the program we will use
        uniforms.set_program(_prog);
    }
    void abort_animation()
    {
        // Set number of animation loops to zero, stops animating
        _md5_model.get_current_animation().set_loop_count(0);

        // Abort the particle system
        _particles->abort();
    }
    void draw(const game::uniforms &uniforms)
    {
        // Draw the particles if we are using it
        if (_particles->is_owner(2))
        {
            _particles->draw(uniforms);
        }

        // clear depth for drawing character over terrain
        glClear(GL_DEPTH_BUFFER_BIT);

        // Activate the uniform buffer
        uniforms.bind();

        // Bind VAO
        _skbuffer.bind();

        // Bind this texture for drawing on channel '0'
        _texture_buffer.bind(_dds_id, 0);

        // Change program back to md5 shaders
        _prog.use();

        // Draw md5 model
        _skbuffer.draw(GL_TRIANGLES, 0);
    }
    const std::vector<min::mat4<float>> &get_bones() const
    {
        return _md5_model.get_bones();
    }
    void set_animation_charge()
    {
        // Flag to reset bones after animation
        _need_bone_reset = true;

        // Set ownership of particles
        _particles->set_owner(2);

        // Set the particle reference position
        _particles->set_charge_reference(10.0);

        // Add particle effects
        _particles->load(86400.0);

        // Set charge animation
        _md5_model.set_current_animation(_charge_index);

        // Activate 999 loop of full animation
        set_animation_count(86400);
    }
    void set_animation_count(const unsigned count)
    {
        _md5_model.get_current_animation().set_loop_count(count);
        _md5_model.get_current_animation().set_time(0);
    }
    void set_animation_shoot()
    {
        // Flag to reset bones after animation
        _need_bone_reset = true;

        // Set shoot animation
        _md5_model.set_current_animation(_shoot_index);

        // Activate 1 loop of full animation
        set_animation_count(1);
    }
    inline bool update(min::camera<float> &cam, const double dt)
    {
        // Only update bones if model is animating them
        if (_md5_model.is_animating())
        {
            // Update the md5 animation
            _md5_model.step(dt);

            // Signal need to update bones
            return true;
        }
        else if (_need_bone_reset)
        {
            // Flag off need reset
            _need_bone_reset = false;

            // Reset bones to identity
            _md5_model.reset_bones();

            // Signal need to update bones
            return true;
        }

        return false;
    }
};
}

#endif
