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
#ifndef __EVENTS__
#define __EVENTS__

#include <chrono>
#include <game/ui_overlay.h>
#include <game/world.h>
#include <random>
#include <thread>

namespace game
{
class events
{
  private:
    constexpr static float _ast_duration = 30.0;
    constexpr static float _ast_tick_duration = 1.0;
    constexpr static size_t _spawn_count = 10;
    float _ast;
    float _ast_timer;
    float _ast_tick;
    float _swarm;
    bool _is_swarm;
    size_t _spawned;
    std::uniform_real_distribution<float> _dist;
    std::mt19937 _gen;

    inline void reset_ast()
    {
        _ast = _dist(_gen);
    }
    inline void reset_swarm()
    {
        _swarm = _dist(_gen);
    }
    inline void update_ast(world &w, ui_overlay &ui, const float dt)
    {
        // If time for asteroids
        if (_ast <= 0.0 && _ast_timer < 0.0)
        {
            // Enable swarm alert
            ui.set_alert_asteroid();

            // Set that swarm is occuring
            _ast_timer = _ast_duration;
            _ast_tick = _ast_duration;
        }
        else if (_ast <= 0.0 && _ast_timer >= 0.0)
        {
            // Decrement count
            _ast_timer -= dt;

            // Reset the event timer
            if (_ast_timer <= 0.0)
            {
                reset_ast();

                // Set debouncer
                _ast_timer = 1.0;
            }

            // Filter spawn rate from tick
            if (_ast_timer < _ast_tick)
            {
                // Tick
                _ast_tick -= _ast_tick_duration;

                // Spawn asteroids
                w.spawn_asteroid();
            }
        }
        else
        {
            // Debouncer for setting alert message
            if (_ast_timer >= 0.0)
            {
                _ast_timer = -1.0;

                // Disable alert
                ui.set_alert_peace();
            }

            // Decrement event timer
            _ast -= dt;
        }
    }
    inline void update_swarm(world &w, ui_overlay &ui, const float dt)
    {
        // Are the drones dead?
        const bool drones_dead = w.get_drones().size() == 0;

        // If drones are dead and its time for a swarm
        if (drones_dead && _swarm <= 0.0)
        {
            // Enable swarm alert
            ui.set_alert_swarm();

            // Reset the timer
            reset_swarm();

            // Spawn a drone
            w.spawn_drone();

            // Reset the swarm spawn counter
            _spawned = 1;

            // Set that swarm is occuring
            _is_swarm = true;
        }
        else if (drones_dead)
        {
            // Debouncer for setting alert message
            if (_is_swarm)
            {
                // Disable alert
                ui.set_alert_peace();

                // Disable swarm
                _is_swarm = false;
            }

            // Decrement event timer
            _swarm -= dt;
        }
    }

  public:
    events() : _ast(-1.0), _ast_timer(-1.0), _ast_tick(0.0),
               _swarm(-1.0), _is_swarm(false), _spawned(_spawn_count), _dist(180.0, 360.0),
               _gen(std::chrono::high_resolution_clock::now().time_since_epoch().count())
    {
        // Reset the ast time
        reset_ast();

        // Reset the swarm time
        reset_swarm();
    }
    inline void reset(world &w, ui_overlay &ui)
    {
        if (_is_swarm)
        {
            // Kill the drones
            w.kill_drones();

            // Disable alert
            ui.set_alert_swarm_kill();

            // Disable swarm
            _is_swarm = false;
        }
    }
    inline void update(world &w, ui_overlay &ui, const float dt)
    {
        // Update ast event
        update_ast(w, ui, dt);

        // Update swarm event
        update_swarm(w, ui, dt);
    }
    inline void update_second(world &w)
    {
        if (_is_swarm && _spawned < _spawn_count)
        {
            // Spawn a drone
            w.spawn_drone();

            // Increment counter
            _spawned++;
        }
    }
};
}

#endif
