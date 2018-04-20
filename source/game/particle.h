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
#ifndef __PARTICLE__
#define __PARTICLE__

#include <game/memory_map.h>
#include <game/uniforms.h>
#include <min/camera.h>
#include <min/dds.h>
#include <min/emitter_buffer.h>
#include <min/mat4.h>
#include <min/texture_buffer.h>
#include <stdexcept>

namespace game
{

class emitter
{
  protected:
    min::emitter_buffer<float, GL_FLOAT> _emit;
    float _time;
    min::vec4<float> _ref;

  public:
    emitter(const min::vec3<float> &p, const size_t emit_count, const size_t emit_periods, const float emit_freq, const float spawn_freq, const float random)
        : _emit(p, emit_count, emit_periods, emit_freq, spawn_freq, random), _time(-1) {}
    void abort()
    {
        _time = -1;
    }
    min::emitter_buffer<float, GL_FLOAT> &emit()
    {
        return _emit;
    }
    const min::emitter_buffer<float, GL_FLOAT> &emit() const
    {
        return _emit;
    }
    void time_dec(const float dt)
    {
        _time -= dt;
    }
    float time() const
    {
        return _time;
    }
    void set_time(const float time)
    {
        _time = time;
    }
    const min::vec4<float> &ref() const
    {
        return _ref;
    }
    void set_ref(const min::vec3<float> &ref)
    {
        _ref.x(ref.x());
        _ref.y(ref.y());
        _ref.z(ref.z());
    }
    void w(const float w)
    {
        _ref.w(w);
    }
};

class miss_emitter : public emitter
{
  public:
    miss_emitter() : emitter(min::vec3<float>(), 25, 4, 0.0625, 0.125, 0.5) {}
};

class static_emitter : public emitter
{
  private:
    bool _is_explode;

  public:
    static constexpr size_t _static_count = 1000;
    static_emitter() : emitter(min::vec3<float>(), _static_count, 1, 0.10, 5.0, 10.0)
    {
        // Set the particle system gravity
        this->_emit.set_gravity(min::vec3<float>(0.0, -10.0, 0.0));
    }
    bool is_explode() const
    {
        return _is_explode;
    }
    void set_explode(const bool flag)
    {
        _is_explode = flag;
    }
};

class particle
{
  private:
    static constexpr size_t _miss_limit = 10;
    static constexpr size_t _static_limit = 10;
    static constexpr float _inv_static_count = 1.0 / static_emitter::_static_count;

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
    std::vector<miss_emitter> _miss;
    size_t _miss_old;
    std::vector<static_emitter> _static;
    size_t _static_old;
    int _attract_index;

    // Control
    float _charge_time;

    // Cached camera settings
    min::vec4<float> _charge_ref;
    min::vec3<float> _line_pos;
    min::vec3<float> _velocity;

    inline min::vec3<float> gun_position(const min::camera<float> &cam)
    {
        return cam.get_position() + (cam.get_right() - cam.get_up()) * 0.1;
    }
    inline void draw_emit() const
    {
        // Bind this texture for drawing
        _tbuffer.bind(_dds_id, 0);

        // Bind VAO
        _emit.bind();

        // Draw the particles
        _emit.draw();
    }
    inline void draw_emit_charge() const
    {
        // Draw charge
        if (_charge_time > 0.0)
        {
            // Set the charge reference point
            set_reference(_charge_ref);

            // Draw on emit buffer
            draw_emit();
        }
    }
    inline void draw_miss_launch() const
    {
        // Bind this texture for drawing missiles
        _tbuffer.bind(_dds_id, 0);

        // Draw all miss launch
        for (size_t i = 0; i < _miss_limit; i++)
        {
            if (_miss[i].time() > 0.0)
            {
                // Set the launch reference point
                set_reference(_miss[i].ref());

                // Bind VAO
                _miss[i].emit().bind();

                // Draw the particles
                _miss[i].emit().draw();
            }
        }
    }
    inline void draw_static() const
    {
        // Bind this texture for drawing static
        _tbuffer.bind(_dds_id, 0);

        // Draw all miss launch
        for (size_t i = 0; i < _static_limit; i++)
        {
            // Draw static
            if (_static[i].time() > 0.0)
            {
                // Set the static reference point
                set_reference(_static[i].ref());

                // Bind VAO
                _static[i].emit().bind();

                // Draw the particles
                _static[i].emit().draw();
            }
        }
    }
    inline void load_emit(const min::vec3<float> &p, const min::vec3<float> &speed, const min::vec3<float> &wind)
    {
        // Update the start position
        _emit.set_position(p);

        // Set speed direction
        _emit.set_speed(speed);

        // Reset the wind vector
        _emit.set_wind(wind);

        // Reset the particle animation
        _emit.reset();
    }
    inline void load_miss(const size_t index, const min::vec3<float> &p, const min::vec3<float> &wind)
    {
        min::emitter_buffer<float, GL_FLOAT> &emit = _miss[index].emit();

        // Update the start position
        emit.set_position(p);

        // Reset the wind vector
        emit.set_wind(wind);

        // Reset the particle animation
        emit.reset();
    }
    inline void load_textures()
    {
        // Load textures
        const min::mem_file &smoke = memory_map::memory.get_file("data/texture/smoke.dds");
        const min::dds b = min::dds(smoke);

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
        uniforms.set_program_lights(_prog);
        uniforms.set_program_matrix(_prog);
    }
    inline void set_reference(const min::vec4<float> &ref) const
    {
        // Use the shader program to draw models
        _prog.use();

        // Set the sampler reference point
        glUniform4f(_index_location, ref.x(), ref.y(), ref.z(), ref.w());
    }
    inline void set_charge_ref(const min::vec3<float> &ref)
    {
        _charge_ref.x(ref.x());
        _charge_ref.y(ref.y());
        _charge_ref.z(ref.z());
    }

  public:
    particle(const game::uniforms &uniforms)
        : _vertex(memory_map::memory.get_file("data/shader/emitter.vertex"), GL_VERTEX_SHADER),
          _fragment(memory_map::memory.get_file("data/shader/emitter.fragment"), GL_FRAGMENT_SHADER),
          _prog(_vertex, _fragment),
          _emit(min::vec3<float>(), 200, 5, 0.0625, 0.125, 0.5),
          _miss(_miss_limit), _miss_old(0),
          _static(_static_limit), _static_old(0),
          _attract_index(-1),
          _charge_time(-1)
    {
        // Load textures
        load_textures();

        // Load program index
        load_program_index(uniforms);

        // Load all missile particle systems
        for (size_t i = 0; i < _miss_limit; i++)
        {
            _miss[i].emit().set_speed(min::vec3<float>(0.0, 0.0, 0.0));
            _miss[i].emit().set_gravity(min::vec3<float>(0.0, 0.0, 0.0));
        }

        // Set the particle gravity for charge
        _emit.set_gravity(min::vec3<float>(0.0, 0.0, 0.0));
    }
    inline void abort_charge()
    {
        // Abort particle animation
        _charge_time = -1.0;
    }
    inline void abort_miss_launch(const size_t index)
    {
        _miss[index].abort();
    }
    inline void abort_line()
    {
        for (size_t i = 0; i < _static_limit; i++)
        {
            // Abort line animation
            if (!_static[i].is_explode())
            {
                _static[i].abort();
            }
        }
    }
    inline void draw() const
    {
        // Draw the explode and line particles
        draw_static();

        // Draw the charge particles
        draw_emit_charge();

        // Draw missiles
        draw_miss_launch();
    }
    inline size_t get_idle_miss_launch_id()
    {
        // Output id
        size_t id = 0;

        // Scan for unused particle system
        for (size_t i = 0; i < _miss_limit; i++)
        {
            // Start at the oldest index
            const size_t index = (_miss_old %= _miss_limit)++;

            // If index is unused
            if (_miss[index].time() < 0.0)
            {
                // Assign id for use
                id = index;

                // Break out since found
                break;
            }
        }

        return id;
    }
    inline void load_emit_charge(const min::camera<float> &cam, const float time, const float size)
    {
        // Add time to the clock
        _charge_time = time;

        // Set the charge particle size
        _charge_ref.w(size);

        // Load into emit buffer
        load_emit(gun_position(cam), cam.get_forward(), min::vec3<float>(0.0, 0.0, 0.0));

        // Recreate the attractor
        if (_attract_index == -1)
        {
            _attract_index = _emit.attractor_add(min::vec3<float>(0.0, 0.0, 0.0), 1.0);
        }
    }
    inline void load_miss_launch(const size_t index, const min::vec3<float> &p, const min::vec3<float> &wind, const float time, const float size)
    {
        // Add time to the clock
        _miss[index].set_time(time);

        // Set the launch particle size
        _miss[index].w(size);

        // Load into emit buffer
        load_miss(index, p, wind);
    }
    inline void load_static_explode(const min::vec3<float> &p, const min::vec3<float> &direction, const float time, const float size)
    {
        // Add time to the clock
        _static[_static_old].set_time(time);

        // Set static particle type
        _static[_static_old].set_explode(true);

        // Set the explode reference position
        _static[_static_old].set_ref(p);
        _static[_static_old].w(size);

        // Update the start position
        _static[_static_old].emit().set_position(p);

        // Set speed direction
        _static[_static_old].emit().set_speed(direction);

        // Reset the static buffer
        _static[_static_old].emit().reset();

        // Increment counter
        (++_static_old) %= _static_limit;
    }
    inline void load_static_line(const min::vec3<float> &p, const float time, const float size)
    {
        // Add time to the clock
        _static[_static_old].set_time(time);

        // Set static particle type
        _static[_static_old].set_explode(false);

        // Set the line destination
        _line_pos = p;

        // Set the explode reference position
        _static[_static_old].w(size);

        // Increment counter
        (++_static_old) %= _static_limit;
    }
    inline void set_velocity(const min::vec3<float> &velocity)
    {
        _velocity = velocity;
    }
    inline void set_miss_launch_position(const size_t index, const min::vec3<float> &p)
    {
        // Update launch position
        _miss[index].emit().set_position(p);

        // Set the reference position
        _miss[index].set_ref(p);
    }
    void update(min::camera<float> &cam, const double dt)
    {
        // Unbind the last VAO to prevent scrambling buffers
        _static[0].emit().unbind();

        // Update charge
        if (_charge_time > 0.0)
        {
            // Remove some of the time
            _charge_time -= dt;

            // Calculate camera settings
            const min::vec3<float> cam_pos = gun_position(cam);
            const min::vec3<float> cam_dir = cam.get_forward();

            // Update the particle attractor position and direction
            const min::vec3<float> attr_position = cam_pos + cam_dir * 0.125;

            // Update particle properties
            _emit.set_position(cam_pos);
            _emit.set_rotation_axis(cam_dir);
            _emit.set_attractor(attr_position, 5.0, _attract_index);
            _emit.set_speed(_velocity + cam_dir);

            // Set the reference position
            set_charge_ref(cam_pos - cam_dir * 0.25);

            // Update the particle positions
            _emit.step(dt);

            // Upload data to GPU
            _emit.upload();
        }

        // Update missile emitters
        for (size_t i = 0; i < _miss_limit; i++)
        {
            if (_miss[i].time() > 0.0)
            {
                // Remove some of the time
                _miss[i].time_dec(dt);

                // Update the particle positions
                _miss[i].emit().step(dt);

                // Upload data to GPU
                _miss[i].emit().upload();
            }
        }

        // Update static emitters
        for (size_t i = 0; i < _static_limit; i++)
        {
            // Update explode
            if (_static[i].is_explode() && _static[i].time() > 0.0)
            {
                // Remove some of the time
                _static[i].time_dec(dt);

                // Update the particle positions
                _static[i].emit().step(dt);

                // Upload data to GPU
                _static[i].emit().upload();
            }
            else if (!_static[i].is_explode() && _static[i].time() > 0.0)
            {
                // Remove some of the time
                _static[i].time_dec(dt);

                // Calculate camera settings
                const min::vec3<float> cam_pos = gun_position(cam);

                // Generate particles in a line
                size_t count = 0;
                const min::vec3<float> spacing = (_line_pos - cam_pos) * _inv_static_count;
                const auto f = [this, &count, &cam_pos, spacing, i](min::vec3<float> &position, min::vec3<float> &speed, const float inv_mass) -> void {

                    // Calculate particle density
                    const float density = (3.75E-6 * count) + 0.00125;

                    // Calculate offset
                    const min::vec3<float> offset = (spacing * count) + (_static[i].emit().random() * density);

                    // Update each particle at position
                    position = cam_pos + offset;

                    // Update counter
                    count++;
                };

                // Update the line reference
                _static[i].set_ref(cam_pos);

                // Update the particle positions
                _static[i].emit().set(f);

                // Upload data to GPU
                _static[i].emit().upload();
            }
        }
    }
};
}

#endif
