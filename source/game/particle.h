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

#include <min/camera.h>
#include <min/dds.h>
#include <min/emitter_buffer.h>
#include <min/mat4.h>
#include <min/texture_buffer.h>
#include <min/uniform_buffer.h>

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
    min::uniform_buffer<float> _ubuffer;
    size_t _attract_index;
    float _time;
    unsigned _owner;
    bool _draw;

    // Cached camera settings
    min::vec3<float> _start;
    min::vec3<float> _direction;

    inline void load_textures()
    {
        // Load textures
        const min::dds b = min::dds("data/texture/stone.dds");

        // Load texture buffer
        _dds_id = _tbuffer.add_dds_texture(b);
    }
    inline void load_uniforms()
    {
        // Load the uniform buffer with program we will use
        _ubuffer.set_program(_prog);

        // Add light to scene
        const min::vec4<float> col(1.0, 1.0, 1.0, 1.0);
        const min::vec4<float> pos(0.0, 100.0, 0.0, 1.0);
        const min::vec4<float> pow(0.3, 0.7, 0.0, 1.0);
        _ubuffer.add_light(min::light<float>(col, pos, pow));

        // Load projection and position matrix into uniform buffer
        _ubuffer.add_matrix(min::mat4<float>());
        _ubuffer.add_matrix(min::mat4<float>());

        // Load the buffer with data
        _ubuffer.update();
    }

  public:
    particle()
        : _vertex("data/shader/emitter.vertex", GL_VERTEX_SHADER),
          _fragment("data/shader/emitter.fragment", GL_FRAGMENT_SHADER),
          _prog(_vertex, _fragment),
          _charge(min::vec3<float>(), 200, 5, 0.125, 0.25, 0.5),
          _explode(min::vec3<float>(), 1000, 1, 0.10, 5.0, 10.0),
          _selected(&_explode),
          _ubuffer(1, 2),
          _time(0.0),
          _owner(0),
          _draw(false)
    {
        // Load textures
        load_textures();

        // Load uniforms
        load_uniforms();

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
    void draw()
    {
        if (_draw)
        {
            // Bind this texture for drawing
            _tbuffer.bind(_dds_id, 0);

            // Use the shader program to draw models
            _prog.use();

            // Bind this uniform buffer
            _ubuffer.bind();

            // Bind VAO
            _selected->bind();

            // Draw the particles
            _selected->draw();
        }
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
    void update(min::camera<float> &cam, const double dt)
    {
        // Update cached camera settings
        _start = cam.project_point(0.05) + (cam.get_right() - cam.get_up()) * 0.075;
        _direction = cam.get_forward();

        // Update the particle position and direction
        const min::vec3<float> attr_position = _start + _direction * 0.5;

        // Update attractor position
        _charge.set_attractor(attr_position, 10.0, _attract_index);

        // If caller owns the particle system
        if (_time > 0.0)
        {
            // Remove some of the time
            _time -= dt;

            // Signal to draw the particles
            _draw = true;

            // Update matrix uniforms
            _ubuffer.set_matrix(cam.get_pv_matrix(), 0);
            _ubuffer.set_matrix(min::mat4<float>(cam.get_position()), 1);

            // Update the matrix buffer
            _ubuffer.update_matrix();

            // If charge beam, continously update
            if (_owner == 2)
            {
                // Update rotation axis
                _selected->set_rotation_axis(_direction);

                // Set speed direction
                _selected->set_speed(_direction);

                // Update the start position
                _selected->set_position(_start);
            }

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
