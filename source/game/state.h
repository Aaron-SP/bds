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

#include <chrono>
#include <game/character.h>
#include <min/sample.h>

namespace game
{

class state
{
  private:
    static constexpr unsigned _frame_average = 4;
    min::camera<float> _camera;
    min::quat<float> _q;
    min::mat4<float> _model;
    uint32_t _energy;
    bool _fire_mode;
    float _x[_frame_average];
    float _y[_frame_average];
    unsigned _frame_count;
    std::chrono::high_resolution_clock::time_point _charge_start;
    std::string _mode;
    bool _pause_mode;
    bool _pause_lock;
    bool _user_input;
    bool _shoot_cooldown;

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

  public:
    state()
        : _energy(0), _fire_mode(true),
          _x{}, _y{}, _frame_count{},
          _mode("MODE: PLAY"),
          _pause_mode(false), _pause_lock(false),
          _user_input(true), _shoot_cooldown(false)
    {
        // Load camera
        load_camera();
    }
    inline void absorb(const int8_t atlas_id)
    {
        // Absorb this amount of energy
        const uint16_t value = 0x1 << (atlas_id);
        _energy += value;
    }
    inline bool can_consume(const int8_t atlas_id)
    {
        // Try to consume energy
        const uint16_t value = 0x2 << (atlas_id);
        if (_energy >= value)
        {
            return true;
        }

        // Not enough energy
        return false;
    }
    inline void consume(const int8_t atlas_id)
    {
        // Consume energy
        const uint16_t value = 0x2 << (atlas_id);
        _energy -= value;
    }
    inline bool will_consume(const int8_t atlas_id)
    {
        // Try to consume energy
        const uint16_t value = 0x2 << (atlas_id);
        if (_energy >= value)
        {
            _energy -= value;
            return true;
        }

        // Not enough energy
        return false;
    }
    inline min::camera<float> &get_camera()
    {
        return _camera;
    }
    inline double get_charge_time() const
    {
        // Get the current time
        const std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();

        // return time since last sync
        return std::chrono::duration<double, std::milli>(now - _charge_start).count();
    }
    inline const min::camera<float> &get_camera() const
    {
        return _camera;
    }
    inline bool get_cooldown() const
    {
        return _shoot_cooldown;
    }
    inline uint32_t get_energy() const
    {
        return _energy;
    }
    inline bool get_fire_mode() const
    {
        return _fire_mode;
    }
    inline const std::string &get_game_mode() const
    {
        return _mode;
    }
    inline bool get_game_pause() const
    {
        return _pause_mode || _pause_lock;
    }
    inline const min::mat4<float> &get_model_matrix()
    {
        return _model;
    }
    inline bool get_user_input() const
    {
        return _user_input;
    }
    inline void pause_lock(const bool lock)
    {
        _pause_lock = lock;
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
    inline void set_charge_time()
    {
        _charge_start = std::chrono::high_resolution_clock::now();
    }
    inline void set_fire_mode(const bool mode)
    {
        _fire_mode = mode;
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
    inline void set_user_input(const bool mode)
    {
        _user_input = mode;
    }
    inline bool toggle_cooldown()
    {
        return _shoot_cooldown = !_shoot_cooldown;
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
        // Set camera start position and look position
        _camera.set_position(p + min::vec3<float>(0.0, 0.5, 0.0));

        // Update the md5 model matrix
        update_model_matrix();

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
};
}

#endif
