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
#ifndef __GOAL_SEEK__
#define __GOAL_SEEK__

#include <chrono>
#include <game/world.h>
#include <min/vec3.h>
#include <random>
#include <vector>

namespace game
{

class goal_seek
{
  private:
    std::vector<min::vec3<float>> _goals;
    size_t _start;
    size_t _current_goal;
    size_t _score;
    std::uniform_int_distribution<int> _int_dist;
    std::mt19937 _rgen;

  public:
    goal_seek(world &w)
        : _start(0), _current_goal(0), _score(0), _int_dist(0, 100),
          _rgen(std::chrono::high_resolution_clock::now().time_since_epoch().count())
    {
        // Set goals
        _goals = {
            min::vec3<float>(0.0, 32.0, 0.0),
            min::vec3<float>(0.0, 24.0, 22.0),
            min::vec3<float>(0.0, 24.0, -22.0),
            min::vec3<float>(22.0, 24.0, 0.0),
            min::vec3<float>(-22.0, 24.0, 0.0)};

        // Randomly select starting target
        _current_goal = _int_dist(_rgen) % _goals.size();

        // Set a random start position
        _start = _int_dist(_rgen) % _goals.size();
        while (_start == _current_goal)
        {
            _start = _int_dist(_rgen) % _goals.size();
        }

        // Set new goal in world
        w.set_destination(_goals[_current_goal]);
    }
    inline const min::vec3<float> &get_goal() const
    {
        return _goals[_current_goal];
    }
    inline size_t get_score() const
    {
        return _score;
    }
    inline const min::vec3<float> &get_start() const
    {
        return _goals[_start];
    }
    inline void seek_next(world &w)
    {
        // Increment score count
        _score++;

        // Randomly select next target
        _current_goal = _int_dist(_rgen) % _goals.size();

        // Set a random start position
        _start = _int_dist(_rgen) % _goals.size();
        while (_start == _current_goal)
        {
            _start = _int_dist(_rgen) % _goals.size();
        }

        // Set next goal
        w.set_destination(get_goal());
    }
    inline bool seek(world &w, const size_t mob_index)
    {
        // Get mob position
        const min::vec3<float> &p = w.mob_position(mob_index);

        // If we reached the goal reward
        const min::vec3<float> &goal = get_goal();
        const float distance = (goal - p).magnitude();
        if (distance < 1.0)
        {
            // Update seek destination
            seek_next(w);

            // Flag that we updated state
            return true;
        }

        // Flag we didn't update state
        return false;
    }
};
}

#endif
