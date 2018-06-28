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
#ifndef __MD5_CHARACTER__
#define __MD5_CHARACTER__

#include <game/memory_map.h>
#include <game/particle.h>
#include <min/aabbox.h>
#include <min/camera.h>
#include <min/mat4.h>
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
    const size_t _charge_index;
    const size_t _shoot_index;

    // Buffers for model data and textures
    min::vertex_buffer<float, uint32_t, min::skeletal_vertex, GL_FLOAT, GL_UNSIGNED_INT> _skbuffer;
    min::texture_buffer _texture_buffer;
    const GLuint _dds_id;

    // Particle system
    particle *const _particles;

    // Animation indices
    bool _need_bone_reset;

    inline size_t load_charge_anim()
    {
        // Load charge animation
        const min::mem_file &gun_charge = memory_map::memory.get_file("data/models/gun_charge.md5anim");
        return _md5_model.load_animation(gun_charge);
    }
    inline size_t load_shoot_anim()
    {
        // Load shoot animation
        const min::mem_file &gun_shoot = memory_map::memory.get_file("data/models/gun_shoot.md5anim");
        return _md5_model.load_animation(gun_shoot);
    }
    inline void load_model()
    {
        // Setup the md5 mesh
        min::mesh<float, uint32_t> &md5 = _md5_model.get_meshes()[0];
        md5.calculate_normals();

        // Unbind the last VAO to prevent scrambling buffers
        _skbuffer.unbind();

        // Add mesh and update buffers
        _skbuffer.add_mesh(md5);

        // Load vertex buffer with data
        _skbuffer.upload();
    }
    inline GLuint load_texture()
    {
        // Load textures
        const min::mem_file &skin = memory_map::memory.get_file("data/texture/skin.dds");
        const min::dds d = min::dds(skin);

        // Load texture buffer
        return _texture_buffer.add_dds_texture(d, true);
    }
    inline void reset_animation()
    {
        // Flag off need reset
        _need_bone_reset = false;

        // Reset bones to identity
        _md5_model.reset_bones();
    }
    inline void set_animation(const size_t index, const unsigned count)
    {
        // Flag to reset bones after animation is FINISHED
        _need_bone_reset = true;

        // Set shoot animation
        _md5_model.set_current_animation(index);

        // Set number of loops
        _md5_model.get_current_animation().set_loop_count(count);

        // Restart animation at beginning
        _md5_model.get_current_animation().set_time(0.0);
    }

  public:
    character(particle *const particles, const uniforms &uniforms)
        : _vertex(memory_map::memory.get_file("data/shader/character.vertex"), GL_VERTEX_SHADER),
          _fragment(memory_map::memory.get_file("data/shader/character.fragment"), GL_FRAGMENT_SHADER),
          _prog(_vertex, _fragment),
          _md5_model(min::md5_mesh<float, uint32_t>(memory_map::memory.get_file("data/models/gun.md5mesh"))),
          _charge_index(load_charge_anim()), _shoot_index(load_shoot_anim()), _dds_id(load_texture()),
          _particles(particles), _need_bone_reset(false)
    {
        // Load md5 model
        load_model();

        // Load the uniform buffer with the program we will use
        uniforms.set_program_lights(_prog);
        uniforms.set_program_matrix(_prog);
    }
    inline void reset()
    {
        _md5_model.get_current_animation().set_loop_count(0);
        _need_bone_reset = false;
    }
    inline void abort_animation_grapple()
    {
        // Set number of animation loops to zero, stops animating
        _md5_model.get_current_animation().set_loop_count(0);

        // Abort the particle system
        _particles->abort_line();
    }
    inline void abort_animation_portal()
    {
        // Set number of animation loops to zero, stops animating
        _md5_model.get_current_animation().set_loop_count(0);

        // Abort the particle system
        _particles->abort_portal();
    }
    inline void abort_animation_shoot()
    {
        // Set number of animation loops to zero, stops animating
        _md5_model.get_current_animation().set_loop_count(0);

        // Abort the particle system
        _particles->abort_charge();
    }
    inline void draw() const
    {
        // Bind VAO
        _skbuffer.bind();

        // Bind this texture for drawing on channel '0'
        _texture_buffer.bind(_dds_id, 0);

        // Change program back to md5 shaders
        _prog.use();

        // Draw md5 model
        _skbuffer.draw(GL_TRIANGLES, 0);
    }
    inline const std::vector<min::mat4<float>> &get_bones() const
    {
        return _md5_model.get_bones();
    }
    inline void set_animation_charge(const min::camera<float> &cam)
    {
        // Add charge particle effects
        _particles->load_emit_charge(cam, 86400.0, 15.0);

        // Activate 999 loop of full animation
        set_animation(_charge_index, 86400);
    }
    inline void set_animation_grapple(const min::vec3<float> &p)
    {
        // Add grapple particle effects
        _particles->load_static_line(p, 86400.0, 30.0);

        // Activate 999 loop of full animation
        set_animation(_charge_index, 86400);
    }
    inline void set_animation_portal()
    {
        // Add grapple particle effects
        _particles->load_static_portal(86400.0, 30.0);

        // Activate 999 loop of full animation
        set_animation(_charge_index, 86400);
    }
    inline void set_animation_shoot()
    {
        // Activate 1 loop of full animation
        set_animation(_shoot_index, 1);
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
            // Reset model to bind pose
            reset_animation();

            // Signal need to update bones
            return true;
        }

        return false;
    }
};
}

#endif
