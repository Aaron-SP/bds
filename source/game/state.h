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
#ifndef __GAME_STATE__
#define __GAME_STATE__

#include <game/character.h>
#include <game/file.h>
#include <game/skill_state.h>

namespace game
{

class state
{
  private:
    static constexpr unsigned _frame_average = 4;
    static constexpr float _health_cap = 100.0;
    min::camera<float> _camera;
    min::quat<float> _q;
    min::mat4<float> _model;
    float _x[_frame_average];
    float _y[_frame_average];
    unsigned _frame_count;
    skill_state _skill_state;
    std::string _mode;
    std::pair<min::vec3<float>, bool> _load_state;
    min::vec3<float> _default_spawn;
    min::vec3<float> _default_look;
    min::vec3<float> _target;
    float _health;
    bool _fix_target;
    bool _pause_mode;
    bool _pause_lock;
    bool _user_input;
    bool _dead;
    bool _respawn;

    inline void load_camera()
    {
        // Set camera near and far plane, and set perspective
        auto &f = _camera.get_frustum();
        f.set_far(5000.0);
        f.set_fov(90.0);
        _camera.set_perspective();
    }
    inline void update_model_matrix()
    {
        const min::vec3<float> &f = _camera.get_forward();
        const min::vec3<float> &fup = _camera.get_frustum().get_up();
        const min::vec3<float> &fr = _camera.get_frustum().get_right();

        // Update the md5 model matrix
        const min::vec3<float> offset = _camera.get_position() + (f - fup + fr) * 0.5;
        _model = min::mat4<float>(offset, _q);
    }
    inline min::quat<float> update_model_rotation() const
    {
        const min::vec3<float> &f = _camera.get_forward();
        const min::vec3<float> &fup = _camera.get_frustum().get_up();
        const min::vec3<float> &fr = _camera.get_frustum().get_right();

        // Calculate the forward vector
        min::vec3<float> d(f.x(), 0.0, f.z());
        d.normalize();

        // Transform the model rotation around shortest arc or Y axis
        const min::vec3<float> y(0.0, 1.0, 0.0);
        const min::vec3<float> x(-1.0, 0.0, 0.0);
        const min::quat<float> roty(x, d, y);

        // Transform the model rotation around shortest arc or RIGHT axis
        const min::quat<float> rotzx(y, fup, fr);

        // Return the transformed model rotation
        return rotzx * roty;
    }
    inline void load_state()
    {
        // Create output stream for loading world
        std::vector<uint8_t> stream;

        // Load data into stream from file
        load_file("bin/state", stream);

        // If load failed dont try to parse stream data
        if (stream.size() != 0)
        {
            // Character position
            size_t next = 0;
            const float x = min::read_le<float>(stream, next);
            const float y = min::read_le<float>(stream, next);
            const float z = min::read_le<float>(stream, next);

            // Load character at this position
            const min::vec3<float> p(x, y, z);

            // Look direction
            const float lx = min::read_le<float>(stream, next);
            const float ly = min::read_le<float>(stream, next);
            const float lz = min::read_le<float>(stream, next);

            // Load camera settings
            const min::vec3<float> look(lx, ly, lz);
            set_camera(p, look);

            // return the state
            _load_state = std::make_pair(p, true);
        }
        else
        {
            // Load camera settings
            set_camera(_default_spawn, _default_look);

            // return the state
            _load_state = std::make_pair(_default_spawn, false);
        }
    }

  public:
    state()
        : _x{}, _y{}, _frame_count{},
          _mode("MODE: PLAY"),
          _default_spawn(0.0, -50.0, 0.0), _default_look(1.0, -50.0, 0.0),
          _health(_health_cap),
          _fix_target(false),
          _pause_mode(false), _pause_lock(false), _user_input(true),
          _dead(false), _respawn(false)
    {
        // Load camera
        load_camera();

        // Load state
        load_state();
    }
    inline void abort_tracking()
    {
        _fix_target = false;
    }
    inline void add_health(float health)
    {
        if (_health < _health_cap)
        {
            _health += health;

            // Cap health at health_cap;
            if (_health > _health_cap)
            {
                _health = _health_cap;
            }
        }
    }
    inline void consume_health(float health)
    {
        _health -= health;
        if (_health <= 0.0)
        {
            _dead = true;
        }
    }
    inline min::camera<float> &get_camera()
    {
        return _camera;
    }
    inline const min::camera<float> &get_camera() const
    {
        return _camera;
    }
    inline const std::string &get_game_mode() const
    {
        return _mode;
    }
    inline bool get_game_pause() const
    {
        return _pause_mode || _pause_lock;
    }
    inline float get_health() const
    {
        return _health;
    }
    inline float get_health_percent() const
    {
        return _health / _health_cap;
    }
    inline const std::pair<min::vec3<float>, bool> &get_load_state() const
    {
        return _load_state;
    }
    inline skill_state &get_skill_state()
    {
        return _skill_state;
    }
    inline const min::mat4<float> &get_model_matrix()
    {
        return _model;
    }
    inline const min::vec3<float> &get_default_spawn()
    {
        return _default_spawn;
    }
    inline bool get_user_input() const
    {
        return _user_input;
    }
    inline bool is_dead() const
    {
        return _dead;
    }
    inline bool is_respawn() const
    {
        return _respawn;
    }
    inline void pause_lock(const bool lock)
    {
        _pause_lock = lock;
    }
    inline void respawn()
    {
        // Reset flags
        _dead = false;
        _respawn = false;

        // Reset health
        _health = _health_cap;

        // Reset energy
        _skill_state.set_energy(0.0);

        // Reload camera settings
        set_camera(_default_spawn, _default_look);
    }
    inline void save_state(const min::vec3<float> &p)
    {
        // Create output stream for saving world
        std::vector<uint8_t> stream;

        // Write position into stream
        min::write_le<float>(stream, p.x());
        min::write_le<float>(stream, p.y());
        min::write_le<float>(stream, p.z());

        // Get the camera look position
        const min::vec3<float> look = _camera.project_point(3.0);

        // Write look into stream
        min::write_le<float>(stream, look.x());
        min::write_le<float>(stream, look.y());
        min::write_le<float>(stream, look.z());

        // Write data to file
        save_file("bin/state", stream);
    }
    inline void set_camera(const min::vec3<float> &p, const min::vec3<float> &look)
    {
        // Set camera start position and look position
        _camera.set_position(p + min::vec3<float>(0.0, 0.5, 0.0));
        _camera.set_look_at(look);

        // Force camera to update internals
        _camera.force_update();

        // Update rotation quaternion
        _q = update_model_rotation();
    }
    inline void set_dead(const bool flag)
    {
        _dead = flag;
    }
    inline void set_game_mode(const std::string &mode)
    {
        _mode = mode;
    }
    inline void set_game_pause(const bool mode)
    {
        // If not locked
        if (!_pause_lock)
        {
            _pause_mode = mode;
        }
    }
    inline void set_respawn(const bool flag)
    {
        _respawn = flag;
    }
    inline void set_user_input(const bool mode)
    {
        _user_input = mode;
    }
    inline void track_target(min::vec3<float> target)
    {
        // Set the look at target to track
        _target = target;

        // Enable fixed look at
        _fix_target = true;
    }
    inline bool toggle_game_pause()
    {
        // If not locked
        if (!_pause_lock)
        {
            return (_pause_mode = !_pause_mode);
        }

        // Force paused
        return this->get_game_pause();
    }
    void update(const min::vec3<float> &p, const std::pair<uint16_t, uint16_t> &c, const uint16_t w, const uint16_t h, const double step)
    {
        // Calculate position to move camera to
        const min::vec3<float> move = p + min::vec3<float>(0.0, 0.5, 0.0);

        // Check if we are fixing look at target
        if (_fix_target)
        {
            // Set camera start position and look position
            _camera.set(move, _target);

            // Force camera to update
            _camera.force_update();

            // Interpolate between the two rotations to avoid jerking
            _q = update_model_rotation();
        }
        else
        {
            // Set camera start position and look position
            _camera.set_position(move);

            // Get the offset from screen center
            const float sensitivity = 0.25;

            // Increment frame count for updating and hash to current frame index
            const unsigned frame_index = (_frame_count %= _frame_average)++;
            _x[frame_index] = sensitivity * (c.first - (w / 2));
            _y[frame_index] = sensitivity * (c.second - (h / 2));

            // Calculate average value of x and y
            float x = 0.0;
            float y = 0.0;
            for (unsigned i = 0; i < _frame_average; i++)
            {
                x += _x[i];
                y += _y[i];
            }

            // Average value for x and y for last N frames
            x /= _frame_average;
            y /= _frame_average;

            // If the mouse coordinates moved at all
            if (std::abs(x) > 1E-3 || std::abs(y) > 1E-3)
            {
                // Get the camera forward vector
                const min::vec3<float> &forward = _camera.get_forward();

                // Check if we have looked too far on the global y axis
                const float dy = forward.dot(min::vec3<float>::up());
                if (dy > 0.975 && y < 0.0)
                {
                    y = 0.0;
                }
                else if (dy < -0.975 && y > 0.0)
                {
                    y = 0.0;
                }

                // Adjust the camera by the offset from screen center
                _camera.move_look_at(x, y);

                // Force camera to update internals
                _camera.force_update();

                // Interpolate between the two rotations to avoid jerking
                _q = update_model_rotation();
            }
        }

        // Update the md5 model matrix
        update_model_matrix();
    }
};
}

#endif
