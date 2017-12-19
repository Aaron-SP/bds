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
#ifndef __PARTICLE__
#define __PARTICLE__

#include <game/uniforms.h>
#include <min/camera.h>
#include <min/dds.h>
#include <min/emitter_buffer.h>
#include <min/mat4.h>
#include <min/texture_buffer.h>

namespace game
{

class particle
{
  private:
    static constexpr size_t _static_count = 1000;
    static constexpr float _inv_static_count = 1.0 / _static_count;

    // Opengl stuff
    min::shader _vertex;
    min::shader _fragment;
    min::program _prog;
    GLint _index_location;

    // Texture stuff
    min::texture_buffer _tbuffer;
    GLuint _dds_id;

    // Particle stuff
    min::emitter_buffer<float, GL_FLOAT> _emit;
    min::emitter_buffer<float, GL_FLOAT> _static;
    int _attract_index;

    // Control
    float _charge_time;
    float _explode_time;
    float _launch_time;
    float _line_time;

    // Cached camera settings
    min::vec4<float> _charge_ref;
    min::vec4<float> _explode_ref;
    min::vec4<float> _launch_ref;
    min::vec4<float> _line_ref;
    min::vec3<float> _line_pos;
    min::vec3<float> _velocity;

    inline min::vec3<float> gun_position(const min::camera<float> &cam)
    {
        return cam.get_position() + (cam.get_right() - cam.get_up()) * 0.1;
    }
    inline void draw_emit(const game::uniforms &uniforms) const
    {
        // Activate the uniform buffer
        uniforms.bind();

        // Bind this texture for drawing
        _tbuffer.bind(_dds_id, 0);

        // Bind VAO
        _emit.bind();

        // Draw the particles
        _emit.draw();
    }
    inline void draw_static(const game::uniforms &uniforms) const
    {
        // Activate the uniform buffer
        uniforms.bind();

        // Bind this texture for drawing
        _tbuffer.bind(_dds_id, 0);

        // Bind VAO
        _static.bind();

        // Draw the particles
        _static.draw();
    }
    inline void load_textures()
    {
        // Load textures
        const min::dds b = min::dds("data/texture/smoke.dds");

        // Load texture buffer
        _dds_id = _tbuffer.add_dds_texture(b);
    }
    inline void load_program_index(const game::uniforms &uniforms)
    {
        // Get the start_index uniform location
        _index_location = glGetUniformLocation(_prog.id(), "camera_position");
        if (_index_location == -1)
        {
            throw std::runtime_error("static_instance: could not find uniform 'start_index'");
        }

        // Load the uniform buffer with program we will use
        uniforms.set_program(_prog);
    }
    inline void set_reference(const min::vec4<float> &ref) const
    {
        // Use the shader program to draw models
        _prog.use();

        // Set the sampler reference point
        glUniform4f(_index_location, ref.x(), ref.y(), ref.z(), ref.w());
    }

  public:
    particle(const game::uniforms &uniforms)
        : _vertex("data/shader/emitter.vertex", GL_VERTEX_SHADER),
          _fragment("data/shader/emitter.fragment", GL_FRAGMENT_SHADER),
          _prog(_vertex, _fragment),
          _emit(min::vec3<float>(), 200, 5, 0.0625, 0.125, 0.5),
          _static(min::vec3<float>(), _static_count, 1, 0.10, 5.0, 10.0),
          _attract_index(-1),
          _charge_time(-1),
          _explode_time(-1),
          _launch_time(-1),
          _line_time(-1)
    {
        // Load textures
        load_textures();

        // Load program index
        load_program_index(uniforms);

        // Set the particle gravity for charge
        _emit.set_gravity(min::vec3<float>(0.0, 0.0, 0.0));

        // Set the particle gravity for explode
        _static.set_gravity(min::vec3<float>(0.0, -10.0, 0.0));
    }
    inline void abort_charge()
    {
        // Abort particle animation
        _charge_time = -1.0;
    }
    inline void abort_explode()
    {
        // Abort particle animation
        _explode_time = -1.0;
    }
    inline void abort_launch()
    {
        // Abort particle animation
        _launch_time = -1.0;
    }
    inline void abort_line()
    {
        // Abort particle animation
        _line_time = -1.0;
    }
    inline void draw_emit_charge(const game::uniforms &uniforms) const
    {
        // Draw charge
        if (_charge_time > 0.0)
        {
            // Set the charge reference point
            set_reference(_charge_ref);

            // Draw on emit buffer
            draw_emit(uniforms);
        }
    }
    inline void draw_emit_launch(const game::uniforms &uniforms) const
    {
        // Draw launch
        if (_launch_time > 0.0)
        {
            // Set the launch reference point
            set_reference(_launch_ref);

            // Draw on emit buffer
            draw_emit(uniforms);
        }
    }
    inline void draw_static_explode(const game::uniforms &uniforms) const
    {
        // Draw explode
        if (_explode_time > 0.0)
        {
            // Set the explode reference point
            set_reference(_explode_ref);

            // Draw on static buffer
            draw_static(uniforms);
        }
    }
    inline void draw_static_line(const game::uniforms &uniforms) const
    {
        // Draw draw
        if (_line_time > 0.0)
        {
            // Set the explode reference point
            set_reference(_line_ref);

            // Draw on static buffer
            draw_static(uniforms);
        }
    }
    inline void load_emit_charge(const min::camera<float> &cam, const float time, const float size)
    {
        // Set the charge particle size
        _charge_ref.w(size);

        // Update the start position
        _emit.set_position(gun_position(cam));

        // Set speed direction
        _emit.set_speed(cam.get_forward());

        // Reset the wind vector
        _emit.set_wind(min::vec3<float>(0.0, 0.0, 0.0));

        // Reset the particle animation
        _emit.reset();

        // Add more time to the clock
        _charge_time = time;
        _launch_time = -1;

        // Recreate the attractor
        if (_attract_index == -1)
        {
            _attract_index = _emit.attractor_add(min::vec3<float>(0.0, 0.0, 0.0), 1.0);
        }
    }
    inline void load_emit_launch(const min::vec3<float> &p, const min::vec3<float> &wind, const float time, const float size)
    {
        // Abort charging
        abort_charge();

        // Set the launch particle size
        _launch_ref.w(size);

        // Update the start position
        _emit.set_position(p);

        // Set the wind in opposite direction of launch direction
        _emit.set_wind(wind);

        // Reset the particle animation
        _emit.reset();

        // Add more time to the clock
        _charge_time = -1;
        _launch_time = time;

        // Delete any attractor
        _attract_index = -1;
        _emit.attractor_clear();
    }
    inline void load_static_explode(const min::vec3<float> &p, const min::vec3<float> &direction, const float time, const float size)
    {
        // Set the explode reference position
        _explode_ref = min::vec4<float>(p, size);

        // Update the start position
        _static.set_position(p);

        // Set speed direction
        _static.set_speed(direction);

        // Reset the static buffer
        _static.reset();

        // Add more time to the clock
        _explode_time = time;
    }
    inline void load_static_line(const min::vec3<float> &p, const float time, const float size)
    {
        // Abort exploding
        abort_explode();

        // Set the line destination
        _line_pos = p;

        // Set the explode reference position
        _line_ref.w(size);

        // Add more time to the clock
        _line_time = time;
    }
    inline void set_velocity(const min::vec3<float> &velocity)
    {
        _velocity = velocity;
    }
    inline void set_launch_position(const min::vec3<float> &p)
    {
        // Update launch position
        _emit.set_position(p);

        // Set the reference position
        _launch_ref.x(p.x());
        _launch_ref.y(p.y());
        _launch_ref.z(p.z());
    }
    void update(min::camera<float> &cam, const double dt)
    {
        // Update charge or launch
        if (_charge_time > 0.0)
        {
            // Remove some of the time
            _charge_time -= dt;

            // Calculate camera settings
            const min::vec3<float> cam_pos = gun_position(cam);
            const min::vec3<float> cam_dir = cam.get_forward();

            // Update the particle position and direction
            const min::vec3<float> attr_position = cam_pos + cam_dir * 0.125;

            // Update charge position, attractor, and direction vectors
            _emit.set_position(cam_pos);
            _emit.set_rotation_axis(cam_dir);
            _emit.set_attractor(attr_position, 5.0, _attract_index);

            // Set the reference position
            const min::vec3<float> offset = cam_dir * 0.25;
            _charge_ref.x(cam_pos.x() - offset.x());
            _charge_ref.y(cam_pos.y() - offset.y());
            _charge_ref.z(cam_pos.z() - offset.z());

            // Turn on the velocity
            _emit.set_speed(_velocity + cam_dir);

            // Update the particle positions
            _emit.step(dt);

            // Upload data to GPU
            _emit.upload();
        }
        else if (_launch_time > 0.0)
        {
            // Remove some of the time
            _launch_time -= dt;

            // Turn off the velocity
            _emit.set_speed(min::vec3<float>(0.0, 0.0, 0.0));

            // Update the particle positions
            _emit.step(dt);

            // Upload data to GPU
            _emit.upload();
        }

        // Update explode
        if (_explode_time > 0.0)
        {
            // Remove some of the time
            _explode_time -= dt;

            // Update the particle positions
            _static.step(dt);

            // Upload data to GPU
            _static.upload();
        }
        else if (_line_time > 0.0)
        {
            // Remove some of the time
            _line_time -= dt;

            // Calculate camera settings
            const min::vec3<float> cam_pos = gun_position(cam);

            // Generate particles in a line
            size_t count = 0;
            const min::vec3<float> spacing = (_line_pos - cam_pos) * _inv_static_count;
            const auto f = [this, &count, &cam_pos, spacing](min::vec3<float> &position, min::vec3<float> &speed, const float inv_mass) -> void {

                // Calculate particle density
                const float density = (3.75E-6 * count) + 0.00125;

                // Calculate offset
                const min::vec3<float> offset = (spacing * count) + (_static.random() * density);

                // Update each particle at position
                position = cam_pos + offset;

                // Update counter
                count++;
            };

            // Update the line reference
            _line_ref.x(cam_pos.x());
            _line_ref.y(cam_pos.y());
            _line_ref.z(cam_pos.z());

            // Update the particle positions
            _static.set(f);

            // Upload data to GPU
            _static.upload();
        }
    }
};
}

#endif
