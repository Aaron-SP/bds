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
    // Opengl stuff
    min::shader _vertex;
    min::shader _fragment;
    min::program _prog;
    GLint _index_location;

    // Texture stuff
    min::texture_buffer _tbuffer;
    GLuint _dds_id;

    // Particle stuff
    min::emitter_buffer<float, GL_FLOAT> _charge;
    min::emitter_buffer<float, GL_FLOAT> _explode;
    int _attract_index;

    // Control
    float _charge_time;
    float _explode_time;
    float _launch_time;

    // Cached camera settings
    min::vec3<float> _cam_dir;
    min::vec3<float> _cam_pos;
    min::vec4<float> _charge_ref;
    min::vec4<float> _explode_ref;
    min::vec4<float> _launch_ref;

    inline void load_textures()
    {
        // Load textures
        const min::dds b = min::dds("data/texture/stone.dds");

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
        // Set the sampler reference point
        glUniform4f(_index_location, ref.x(), ref.y(), ref.z(), ref.w());
    }

  public:
    particle(const game::uniforms &uniforms)
        : _vertex("data/shader/emitter.vertex", GL_VERTEX_SHADER),
          _fragment("data/shader/emitter.fragment", GL_FRAGMENT_SHADER),
          _prog(_vertex, _fragment),
          _charge(min::vec3<float>(), 200, 5, 0.0625, 0.125, 0.5),
          _explode(min::vec3<float>(), 1000, 1, 0.10, 5.0, 10.0),
          _attract_index(-1),
          _charge_time(-1),
          _explode_time(-1),
          _launch_time(-1)
    {
        // Load textures
        load_textures();

        // Load program index
        load_program_index(uniforms);

        // Set the particle gravity for charge
        _charge.set_gravity(min::vec3<float>(0.0, 0.0, 0.0));

        // Set the particle gravity for explode
        _explode.set_gravity(min::vec3<float>(0.0, -10.0, 0.0));
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
    inline void draw_charge(const game::uniforms &uniforms) const
    {
        // Draw charge
        if (_charge_time > 0.0)
        {
            // Activate the uniform buffer
            uniforms.bind();

            // Bind this texture for drawing
            _tbuffer.bind(_dds_id, 0);

            // Use the shader program to draw models
            _prog.use();

            // Set the charge reference point
            set_reference(_charge_ref);

            // Bind VAO
            _charge.bind();

            // Draw the particles
            _charge.draw();
        }
    }
    inline void draw_explode(const game::uniforms &uniforms) const
    {
        // Draw explode
        if (_explode_time > 0.0)
        {
            // Activate the uniform buffer
            uniforms.bind();

            // Bind this texture for drawing
            _tbuffer.bind(_dds_id, 0);

            // Use the shader program to draw models
            _prog.use();

            // Set the explode reference point
            set_reference(_explode_ref);

            // Bind VAO
            _explode.bind();

            // Draw the particles
            _explode.draw();
        }
    }
    inline void draw_launch(const game::uniforms &uniforms) const
    {
        // Draw charge
        if (_launch_time > 0.0)
        {
            // Activate the uniform buffer
            uniforms.bind();

            // Bind this texture for drawing
            _tbuffer.bind(_dds_id, 0);

            // Use the shader program to draw models
            _prog.use();

            // Set the launch reference point
            set_reference(_launch_ref);

            // Bind VAO
            _charge.bind();

            // Draw the particles
            _charge.draw();
        }
    }
    inline void load_charge(const float time)
    {
        // Update the start position
        _charge.set_position(_cam_pos);

        // Set speed direction
        _charge.set_speed(_cam_dir);

        // Reset the particle animation
        _charge.reset();

        // Add more time to the clock
        _charge_time = time;
        _launch_time = -1;

        // Recreate the attractor
        if (_attract_index == -1)
        {
            _attract_index = _charge.attractor_add(min::vec3<float>(0.0, 0.0, 0.0), 1.0);
        }
    }
    inline void load_explode(const min::vec3<float> &position, const min::vec3<float> &direction, const float time)
    {
        // Update the start position
        _explode.set_position(position);

        // Set speed direction
        _explode.set_speed(direction);

        // Reset the particle animation
        _explode.reset();

        // Add more time to the clock
        _explode_time = time;
    }
    inline void load_launch(const float time)
    {
        // Update the start position
        _charge.set_position(_cam_pos);

        // Set speed direction
        _charge.set_speed(_cam_dir);

        // Reset the particle animation
        _charge.reset();

        // Add more time to the clock
        _charge_time = -1;
        _launch_time = time;

        // Delete any attractor
        _attract_index = -1;
        _charge.attractor_clear();
    }
    inline void set_charge_reference(const float size)
    {
        // Set reference position
        _charge_ref = min::vec4<float>(_cam_pos, size);

        // Reset the wind vector
        _charge.set_wind(min::vec3<float>(0.0, 0.0, 0.0));
    }
    inline void set_charge_velocity(const min::vec3<float> &velocity)
    {
        _charge.set_speed(velocity + _cam_dir);
    }
    inline void set_explode_reference(const min::vec3<float> &ref, const float size)
    {
        // Set reference position and particle size
        _explode_ref = min::vec4<float>(ref, size);
    }
    inline void set_launch_position(const min::vec3<float> &p)
    {
        // Update launch position
        _charge.set_position(p);

        // Set the reference position
        _launch_ref.x(p.x());
        _launch_ref.y(p.y());
        _launch_ref.z(p.z());
    }
    inline void set_launch_reference(const min::vec3<float> &dir, const float size)
    {
        // Set reference position
        _launch_ref = min::vec4<float>(_cam_pos, size);

        // Set the wind in opposite direction of launch direction
        _charge.set_wind(dir);
    }
    void update(min::camera<float> &cam, const double dt)
    {
        // Update cached camera settings
        _cam_pos = cam.project_point(0.05) + (cam.get_right() - cam.get_up()) * 0.1;
        _cam_dir = cam.get_forward();

        // Update charge or launch
        if (_charge_time > 0.0)
        {
            // Update the particle position and direction
            const min::vec3<float> attr_position = _cam_pos + _cam_dir * 0.125;

            // Update charge position, attractor, and direction vectors
            _charge.set_position(_cam_pos);
            _charge.set_rotation_axis(_cam_dir);
            _charge.set_attractor(attr_position, 5.0, _attract_index);

            // Set the reference position
            const min::vec3<float> offset = _cam_dir * 0.25;
            _charge_ref.x(_cam_pos.x() - offset.x());
            _charge_ref.y(_cam_pos.y() - offset.y());
            _charge_ref.z(_cam_pos.z() - offset.z());

            // Remove some of the time
            _charge_time -= dt;

            // Update the particle positions
            _charge.step(dt);

            // Upload data to GPU
            _charge.upload();
        }
        else if (_launch_time > 0.0)
        {
            // Remove some of the time
            _launch_time -= dt;

            // Update the particle positions
            _charge.step(dt);

            // Upload data to GPU
            _charge.upload();
        }

        // Update explode
        if (_explode_time > 0.0)
        {
            // Remove some of the time
            _explode_time -= dt;

            // Update the particle positions
            _explode.step(dt);

            // Upload data to GPU
            _explode.upload();
        }
    }
};
}

#endif
