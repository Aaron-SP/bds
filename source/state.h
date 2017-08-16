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
    character _player;

    void update_player(const min::camera<float> &cam)
    {
        // Calculate the forward vector
        const min::vec3<float> &f = cam.get_forward();
        min::vec3<float> d(f.x(), 0.0, f.z());
        d.normalize();

        // Transform the model rotation around shortest arc or Y axis
        const min::vec3<float> x(-1.0, 0.0, 0.0);
        const min::vec3<float> y(0.0, 1.0, 0.0);
        const min::quat<float> roty(x, d, y);

        // Transform the model rotation around shortest arc or RIGHT axis
        const min::vec3<float> fup = cam.get_frustum().get_up();
        const min::vec3<float> fr = cam.get_frustum().get_right();
        const min::quat<float> rotzx(y, fup, fr);

        // Update the md5 model matrix
        const min::vec3<float> p = cam.get_position() + (cam.get_forward() - min::vec3<float>::up() + cam.get_right()) * 0.5;
        min::mat4<float> model(p, rotzx * roty);
        _player.set_model_matrix(model);
    }

  public:
    void animate_shoot_player()
    {
        // Activate shoot animation
        _player.set_animation_count(1);
    }
    void draw(min::camera<float> &cam, const float dt)
    {
        // Update the md5 model
        update_player(cam);

        // Draw the character
        _player.draw(cam, dt);
    }
};
}

#endif
