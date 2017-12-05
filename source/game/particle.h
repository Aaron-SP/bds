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

    // Texture stuff
    min::texture_buffer _tbuffer;
    GLuint _dds_id;

    // Particle stuff
    min::emitter_buffer<float, GL_FLOAT> _charge;
    min::emitter_buffer<float, GL_FLOAT> _explode;
    min::emitter_buffer<float, GL_FLOAT> *_selected;
    size_t _attract_index;
    float _time;
    unsigned _owner;
    bool _draw;

    // Cached camera settings
    min::vec3<float> _direction;
    min::vec3<float> _start;
    min::vec4<float> _reference;
    min::vec3<float> _velocity;

    inline void load_textures()
    {
        // Load textures
        const min::dds b = min::dds("data/texture/stone.dds");

        // Load texture buffer
        _dds_id = _tbuffer.add_dds_texture(b);
    }

  public:
    particle(const game::uniforms &uniforms)
        : _vertex("data/shader/emitter.vertex", GL_VERTEX_SHADER),
          _fragment("data/shader/emitter.fragment", GL_FRAGMENT_SHADER),
          _prog(_vertex, _fragment),
          _charge(min::vec3<float>(), 200, 5, 0.0625, 0.125, 0.5),
          _explode(min::vec3<float>(), 1000, 1, 0.10, 5.0, 10.0),
          _selected(&_explode),
          _time(0.0),
          _owner(0),
          _draw(false)
    {
        // Load textures
        load_textures();

        // Load the uniform buffer with program we will use
        uniforms.set_program(_prog);

        // Set the particle gravity for charge
        _charge.set_gravity(min::vec3<float>(0.0, 0.0, 0.0));
        _attract_index = _charge.attractor_add(min::vec3<float>(0.0, 0.0, 0.0), 1.0);

        // Set the particle gravity for explode
        _explode.set_gravity(min::vec3<float>(0.0, -10.0, 0.0));
    }
    void abort()
    {
        // Abort particle animation
        _time = -1.0;
    }
    void draw(const game::uniforms &uniforms)
    {
        if (_draw)
        {
            // Activate the uniform buffer
            uniforms.bind();

            // Bind this texture for drawing
            _tbuffer.bind(_dds_id, 0);

            // Use the shader program to draw models
            _prog.use();

            // Bind VAO
            _selected->bind();

            // Draw the particles
            _selected->draw();
        }
    }
    const min::vec4<float> &get_reference()
    {
        return _reference;
    }
    void load(const min::vec3<float> &position, const min::vec3<float> &direction, const float time)
    {
        // Set speed direction
        _selected->set_speed(direction);

        // Update the start position
        _selected->set_position(position);

        // Reset the particle animation
        _selected->reset();

        // Add more time to the clock
        _time = time;
    }
    void load(const float time)
    {
        // Set speed direction
        _selected->set_speed(_direction);

        // Update the start position
        _selected->set_position(_start);

        // Reset the particle animation
        _selected->reset();

        // Add more time to the clock
        _time = time;
    }
    bool is_owner(const unsigned owner) const
    {
        return _owner == owner;
    }
    void set_owner(const unsigned owner)
    {
        _owner = owner;

        // Select the appropriate particle system
        if (_owner == 1)
        {
            _selected = &_explode;
        }
        else if (_owner == 2)
        {
            _selected = &_charge;
        }
    }
    void set_velocity(const min::vec3<float> &velocity)
    {
        _velocity = velocity;
    }
    void set_charge_reference(const float size)
    {
        // Set reference position
        _reference = _start;

        // Set particle size
        _reference.w(size);
    }
    void set_explode_reference(const min::vec3<float> &ref, const float size)
    {
        _reference = min::vec4<float>(ref, size);
    }
    void update(min::camera<float> &cam, const double dt)
    {
        // Update cached camera settings
        _start = cam.project_point(0.05) + (cam.get_right() - cam.get_up()) * 0.1;
        _direction = cam.get_forward();

        // If charge owns the particle system
        if (_owner == 2)
        {
            // Update the particle position and direction
            const min::vec3<float> attr_position = _start + _direction * 0.125;

            // Update charge position, attractor, and direction vectors
            _charge.set_position(_start);
            _charge.set_rotation_axis(_direction);
            _charge.set_speed(_velocity + _direction);
            _charge.set_attractor(attr_position, 5.0, _attract_index);

            const min::vec3<float> offset = _direction * 0.25;
            _reference.x(_start.x() - offset.x());
            _reference.y(_start.y() - offset.y());
            _reference.z(_start.z() - offset.z());
        }

        // If caller owns the particle system
        if (_time > 0.0)
        {
            // Remove some of the time
            _time -= dt;

            // Signal to draw the particles
            _draw = true;

            // Update the particle positions
            _selected->step(dt);

            // Upload data to GPU
            _selected->upload();
        }
        else
        {
            _draw = false;
        }
    }
};
}

#endif
