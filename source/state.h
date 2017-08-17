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
#ifndef __GAME_STATE__
#define __GAME_STATE__

#include <character.h>

namespace game
{

class state
{
  private:
    static constexpr unsigned _frame_average = 4;
    static constexpr unsigned _frame_update = 180;
    character _player;
    min::camera<float> _camera;
    min::quat<float> _q;
    bool _fire_mode;
    float _x[_frame_average];
    float _y[_frame_average];
    unsigned _frame_count;

    void load_camera()
    {
        // Set camera near and far plane, and set perspective
        auto &f = _camera.get_frustum();
        f.set_far(5000.0);
        f.set_fov(90.0);
        _camera.set_perspective();
    }
    void update_state(const min::vec3<float> &p)
    {
        // Set camera start position and look position
        _camera.set_position(p + min::vec3<float>(0.0, 1.0, 0.0));

        const min::vec3<float> &f = _camera.get_forward();
        const min::vec3<float> &fup = _camera.get_frustum().get_up();
        const min::vec3<float> &fr = _camera.get_frustum().get_right();

        // Do a slow update on update frame for correcting 'drift'
        if (_frame_count == _frame_update)
        {
            // Reset frame count
            _frame_count = 0;

            // Calculate the forward vector
            min::vec3<float> d(f.x(), 0.0, f.z());
            d.normalize();

            // Transform the model rotation around shortest arc or Y axis
            const min::vec3<float> y(0.0, 1.0, 0.0);
            const min::vec3<float> x(-1.0, 0.0, 0.0);
            const min::quat<float> roty(x, d, y);

            // Transform the model rotation around shortest arc or RIGHT axis
            const min::quat<float> rotzx(y, fup, fr);

            // Update model rotation
            _q = rotzx * roty;
        }

        // Update the md5 model matrix
        const min::vec3<float> offset = _camera.get_position() + (f - fup + fr) * 0.5;
        min::mat4<float> model(offset, _q);
        _player.set_model_matrix(model);
    }

  public:
    state() : _fire_mode(true), _x{}, _y{}, _frame_count(_frame_update)
    {
        // Load camera
        load_camera();
    }
    void animate_shoot_player()
    {
        // Activate shoot animation
        _player.set_animation_count(1);
    }
    void draw(min::camera<float> &cam, const float dt)
    {
        // Draw the character if fire mode activated
        if (_fire_mode)
        {
            _player.draw(cam, dt);
        }
    }
    min::camera<float> &get_camera()
    {
        return _camera;
    }
    const min::camera<float> &get_camera() const
    {
        return _camera;
    }
    void set_camera(const min::vec3<float> &p, const min::vec3<float> &look)
    {
        // Set camera start position and look position
        _camera.set_position(p + min::vec3<float>(0.0, 1.0, 0.0));
        _camera.set_look_at(look);

        // Force camera to update internals
        _camera.force_update();
    }
    bool get_fire_mode() const
    {
        return _fire_mode;
    }
    void set_fire_mode(const bool mode)
    {
        _fire_mode = mode;
    }
    void update(const min::vec3<float> &p, const std::pair<uint16_t, uint16_t> &c, const uint16_t w, const uint16_t h, const double step)
    {
        // Update state properties
        update_state(p);

        // Get the offset from screen center
        const float sensitivity = 0.10;
        const unsigned frame_index = (_frame_count % _frame_average);
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
            _q = _camera.move_look_at(x, y) * _q;
        }

        // Increment frame count for updating
        _frame_count++;
    }
};
}

#endif
