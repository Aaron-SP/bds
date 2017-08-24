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
#ifndef __GAI_TRAINER__
#define __GAI_TRAINER__

#include <game/world.h>
#include <min/vec3.h>
#include <vector>

namespace game
{

class ai_trainer
{
  private:
    std::vector<min::vec3<float>> _goals;
    size_t _current_goal;
    size_t _stuck_count;
    min::vec3<float> _last;

  public:
    ai_trainer(world &w) : _current_goal(0), _stuck_count(0)
    {
        // Set goals
        _goals = {
            min::vec3<float>(0.5, 36.0, -0.5),
            min::vec3<float>(21.0, 23.0, 0.0),
            min::vec3<float>(-21.0, 23.0, 0.0),
            min::vec3<float>(0.0, 23.0, 21.0),
            min::vec3<float>(0.0, 23.0, -21.0),
            min::vec3<float>(-4.5, 30.5, 4.5),
            min::vec3<float>(-4.6, 31.5, 0.0),
            min::vec3<float>(-2.223, 32.5, -4.667),
            min::vec3<float>(2.0, 31.5, -4.5),
            min::vec3<float>(-4.5, 30.5, 0.0),
            min::vec3<float>(4.223, 32.5, 2.667),
            min::vec3<float>(4.5, 31.5, -2.0),
            min::vec3<float>(4.5, 30.5, 0.0),
            min::vec3<float>(4.5, 31.5, -4.5),
            min::vec3<float>(4.5, 31.5, 0.0),
            min::vec3<float>(0.0, 40.5, 0.0),
            min::vec3<float>(0.0, 25.5, 0.0)};

        // Set new goal in world
        w.set_train_point(_goals[0]);
    }
    void train(world &w)
    {
        // Train the AI
        w.train(10);

        // Get character position
        const min::vec3<float> &p = w.character_position();
        const float distance = (_goals[_current_goal] - p).magnitude();
        if (distance < 1.0)
        {
            // Increment goals
            _current_goal++;

            // Loop if reached the end of goal list
            _current_goal %= _goals.size();

            // Set next goal
            w.set_train_point(_goals[_current_goal]);
        }

        // Check if player is stuck
        const float moved = (p - _last).magnitude();
        if (moved < 0.1)
        {
            _stuck_count++;
            if (_stuck_count == 5)
            {
                _stuck_count = 0;
                w.character_warp(min::vec3<float>(0.0, 2.0, 0.0));
            }
        }

        // Update last position
        _last = p;
    }
};
}

#endif
