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
    bool _in_view;
    min::vec4<float> _ref;

  public:
    emitter(const min::vec3<float> &p, const size_t emit_count, const size_t emit_periods, const float emit_freq, const float spawn_freq, const float random)
        : _emit(p, emit_count, emit_periods, emit_freq, spawn_freq, random), _time(-1), _in_view(false) {}
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
    inline bool is_in_view() const
    {
        return _in_view;
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
    inline void set_view(const bool flag)
    {
        _in_view = flag;
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

enum class static_type
{
    explode,
    line,
    portal
};

class static_emitter : public emitter
{
  private:
    static_type _type;

  public:
    static constexpr size_t _static_count = 1000;
    static_emitter() : emitter(min::vec3<float>(), _static_count, 1, 0.0, 5.0, 10.0)
    {
        // Set the particle system gravity
        this->_emit.set_gravity(min::vec3<float>(0.0, -10.0, 0.0));
    }
    static_type get_type() const
    {
        return _type;
    }
    void set_type(const static_type type)
    {
        _type = type;
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
    const GLint _index_location;

    // Texture stuff
    min::texture_buffer _tbuffer;
    const GLuint _dds_id;

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
    inline void draw_emit_charge() const
    {
        // Draw charge
        if (_charge_time > 0.0)
        {
            // Bind this texture for drawing
            _tbuffer.bind(_dds_id, 0);

            // Use the shader program to draw models
            _prog.use();

            // Upload data to GPU
            _emit.upload();

            // Set the charge reference point
            set_reference(_charge_ref);

            // Bind VAO
            _emit.bind();

            // Draw the particles
            _emit.draw();
        }
    }
    inline void draw_miss_launch() const
    {
        // Bind this texture for drawing missiles
        _tbuffer.bind(_dds_id, 0);

        // Use the shader program to draw models
        _prog.use();

        // Draw all miss launch
        for (size_t i = 0; i < _miss_limit; i++)
        {
            if (_miss[i].time() > 0.0 && _miss[i].is_in_view())
            {
                // Upload data to GPU
                _miss[i].emit().upload();

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

        // Use the shader program to draw models
        _prog.use();

        // Draw all miss launch
        for (size_t i = 0; i < _static_limit; i++)
        {
            // Draw static
            if (_static[i].time() > 0.0 && _static[i].is_in_view())
            {
                // Upload data to GPU
                _static[i].emit().upload();

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
    inline GLuint load_textures()
    {
        // Load textures
        const min::mem_file &smoke = memory_map::memory.get_file("data/texture/smoke.dds");
        const min::dds b = min::dds(smoke);

        // Load texture buffer
        return _tbuffer.add_dds_texture(b, true);
    }
    inline GLint load_program_index(const game::uniforms &uniforms) const
    {
        // Get the start_index uniform location
        const GLint index_location = glGetUniformLocation(_prog.id(), "camera_position");
        if (index_location == -1)
        {
            throw std::runtime_error("particle: could not find uniform 'camera_position'");
        }

        // Load the uniform buffer with program we will use
        uniforms.set_program_matrix(_prog);

        // Return the index
        return index_location;
    }
    inline void set_reference(const min::vec4<float> &ref) const
    {
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
          _index_location(load_program_index(uniforms)),
          _dds_id(load_textures()),
          _emit(min::vec3<float>(), 200, 5, 0.0625, 0.125, 0.5),
          _miss(_miss_limit), _miss_old(0),
          _static(_static_limit), _static_old(0),
          _attract_index(-1),
          _charge_time(-1.0)
    {
        // Load all missile particle systems
        for (size_t i = 0; i < _miss_limit; i++)
        {
            _miss[i].emit().set_speed(min::vec3<float>(0.0, 0.0, 0.0));
            _miss[i].emit().set_gravity(min::vec3<float>(0.0, 0.0, 0.0));
        }

        // Set the particle gravity for charge
        _emit.set_gravity(min::vec3<float>(0.0, 0.0, 0.0));
    }
    inline void reset()
    {
        // Reset missiles
        for (size_t i = 0; i < _miss_limit; i++)
        {
            _miss[i].abort();
        }
        _miss_old = 0;

        // Reset static
        for (size_t i = 0; i < _static_limit; i++)
        {
            _static[i].abort();
        }
        _static_old = 0;

        // Reset charge
        _attract_index = -1;
        _charge_time = -1.0;
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
            if (_static[i].get_type() == static_type::line)
            {
                _static[i].abort();
            }
        }
    }
    inline void abort_portal()
    {
        for (size_t i = 0; i < _static_limit; i++)
        {
            // Abort line animation
            if (_static[i].get_type() == static_type::portal)
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
        // Skip over in use, non-explode particle systems
        while (_static[_static_old].time() > 0.0 && _static[_static_old].get_type() != static_type::explode)
        {
            // Increment counter
            (++_static_old) %= _static_limit;
        }

        // Get the emitter
        static_emitter &stat = _static[_static_old];

        // Add time to the clock
        stat.set_time(time);

        // Set static particle type
        stat.set_type(static_type::explode);

        // Set the static reference position
        stat.set_ref(p);
        stat.w(size);

        // Update the start position
        stat.emit().set_position(p);

        // Set speed direction
        stat.emit().set_speed(direction);

        // Reset the static buffer
        stat.emit().reset();

        // Increment counter
        (++_static_old) %= _static_limit;
    }
    inline void load_static_line(const min::vec3<float> &p, const float time, const float size)
    {
        // Get the emitter
        static_emitter &stat = _static[_static_old];

        // Add time to the clock
        stat.set_time(time);

        // Clear the accum
        stat.emit().reset_accum();

        // Set static particle type
        stat.set_type(static_type::line);

        // Set the line destination
        _line_pos = p;

        // Set the static reference position
        stat.w(size);

        // Increment counter
        (++_static_old) %= _static_limit;
    }
    inline void load_static_portal(const float time, const float size)
    {
        // Get the emitter
        static_emitter &stat = _static[_static_old];

        // Add time to the clock
        stat.set_time(time);

        // Clear the accum
        stat.emit().reset_accum();

        // Set static particle type
        stat.set_type(static_type::portal);

        // Set the static reference position
        stat.w(size);

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
    inline void update_static_explode(const size_t index, const min::frustum<float> &frust, const float dt)
    {
        // Get the emitter
        static_emitter &stat = _static[index];

        // Set the view flag
        const min::vec3<float> &p = stat.emit().get_position();
        stat.set_view(frust.point_inside(p));

        // Remove some of the time
        stat.time_dec(dt);

        // Update the particle positions
        stat.emit().step(dt);
    }
    inline void update_static_line(const size_t index, const min::camera<float> &cam, const float dt)
    {
        // Get the emitter
        static_emitter &stat = _static[index];

        // Set the view flag
        stat.set_view(true);

        // Remove some of the time
        stat.time_dec(dt);

        // Calculate camera settings
        const min::vec3<float> cam_pos = gun_position(cam);

        // Generate particles in a line
        size_t count = 0;
        const min::vec3<float> spacing = (_line_pos - cam_pos) * _inv_static_count;
        const auto f = [&cam_pos, &count, &spacing, &stat](min::vec3<float> &position, min::vec3<float> &speed, const float accum, const float inv_mass) -> void {
            // Calculate particle density
            const float density = (3.75E-6 * count) + 0.00125;

            // Calculate offset
            const min::vec3<float> offset = (spacing * count) + (stat.emit().random() * density);

            // Update each particle at position
            position = cam_pos + offset;

            // Update counter
            count++;
        };

        // Update the line reference
        stat.set_ref(cam_pos);

        // Update the particle positions
        stat.emit().set(f, dt);
    }
    inline void update_static_portal(const size_t index, const min::camera<float> &cam, const float dt)
    {
        // Get the emitter
        static_emitter &stat = _static[index];

        // Set the view flag
        stat.set_view(true);

        // Remove some of the time
        stat.time_dec(dt);

        // Calculate camera settings
        const min::vec3<float> cam_pos = gun_position(cam);

        // Generate particles in a line
        size_t count = 0;
        const auto f = [&cam, &cam_pos, &count](min::vec3<float> &position, min::vec3<float> &speed, const float accum, const float inv_mass) -> void {
            // Spiral properties
            const float R = 0.5;
            const float t = accum * count * 0.05;
            const float r = std::cos(t) * R;
            const float f = count * 0.01;
            const float u = std::sin(t) * R;

            // Calculate offset
            const min::vec3<float> offset = (cam.get_right() * r) + (cam.get_up() * u) + (cam.get_forward() * f);

            // Update each particle at position
            position = cam_pos + offset;

            // Update counter
            count++;
        };

        // Update the line reference
        stat.set_ref(cam_pos);

        // Update the particle positions
        stat.emit().set(f, dt);
    }
    void update(const min::camera<float> &cam, const float dt)
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
        }

        // Get the camera frustum
        const min::frustum<float> &frust = cam.get_frustum();

        // Update missile emitters
        for (size_t i = 0; i < _miss_limit; i++)
        {
            // Update missile
            if (_miss[i].time() > 0.0)
            {
                // Set the view flag
                const min::vec3<float> &p = _miss[i].emit().get_position();
                _miss[i].set_view(frust.point_inside(p));

                // Remove some of the time
                _miss[i].time_dec(dt);

                // Update the particle positions
                _miss[i].emit().step(dt);
            }
        }

        // Update static emitters
        for (size_t i = 0; i < _static_limit; i++)
        {
            // If this emitter is being animated
            if (_static[i].time() > 0.0)
            {
                // Update explode
                const static_type type = _static[i].get_type();
                switch (type)
                {
                case static_type::explode:

                    // Update explode emitter
                    update_static_explode(i, frust, dt);
                    break;
                case static_type::line:

                    // Update line emitter
                    update_static_line(i, cam, dt);
                    break;
                case static_type::portal:

                    // Update portal emitter
                    update_static_portal(i, cam, dt);
                    break;
                }
            }
        }
    }
};
}

#endif
