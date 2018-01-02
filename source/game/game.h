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
#ifndef __GAME_HEADER__
#define __GAME_HEADER__

#include <game/controls.h>
#include <game/goal_seek.h>
#include <game/particle.h>
#include <game/state.h>
#include <game/title.h>
#include <game/ui_overlay.h>
#include <game/uniforms.h>
#include <game/world.h>
#include <iostream>
#include <min/bmp.h>
#include <min/camera.h>
#include <min/loop_sync.h>
#include <min/settings.h>
#include <min/utility.h>
#include <min/window.h>
#include <string>
#include <utility>

class fractex
{
  private:
#ifdef MGL_VB43
    static constexpr size_t _gl_major = 4;
    static constexpr size_t _gl_minor = 3;
#else
    static constexpr size_t _gl_major = 3;
    static constexpr size_t _gl_minor = 3;
#endif
    static constexpr size_t _width = 720;
    static constexpr size_t _height = 480;

    min::window _win;
    game::uniforms _uniforms;
    game::ui_overlay _ui;
    game::particle _particles;
    game::character _character;
    game::state _state;
    game::world _world;
    game::controls _controls;
    game::title _title;
    game::goal_seek _goal_seek;

    inline void die()
    {
        // Set menu for dead
        _ui.set_menu_dead();

        // Show menu
        _ui.set_draw_menu(true);

        // Disable the keyboard
        auto &keyboard = _win.get_keyboard();
        keyboard.disable();
    }
    inline void respawn()
    {
        // Refresh state
        _state.respawn();

        // Refresh ui
        _ui.respawn();

        // Refresh the world exploded flag
        _world.respawn(_state.get_default_spawn());

        // Enable the keyboard
        auto &keyboard = _win.get_keyboard();
        keyboard.enable();
    }
    inline std::pair<unsigned, unsigned> user_input()
    {
        if (_state.get_user_input() && !_state.is_dead())
        {
            // Get the cursor coordinates
            const auto c = _win.get_cursor();

            // Reset cursor position
            update_cursor();

            // return the mouse coordinates
            return c;
        }

        // return no mouse movement
        return std::make_pair(_win.get_width() / 2, _win.get_height() / 2);
    }
    inline void update_die_respawn()
    {
        // Check if we exploded
        if (!_state.is_dead())
        {
            if (_world.is_exploded())
            {
                const int8_t ex_id = _world.get_explode_id();
                if (ex_id == 7 || ex_id == 5)
                {
                    // Kill the player
                    _state.consume_health(90.0);
                }
                else
                {
                    _state.consume_health(10.0);
                }
            }

            // Check if we need to die
            if (_state.is_dead())
            {
                die();
            }
        }
        else if (_state.is_respawn())
        {
            // Respawn
            respawn();
        }

        // Reset the explosion flag
        _world.reset_explode();
    }
    inline void update_uniforms(min::camera<float> &camera, const bool update_bones)
    {
        // Bind uniforms
        _uniforms.bind();

        // Update camera properties
        _uniforms.update_camera(camera);

        // Update world preview matrix
        _uniforms.update_preview(_world.get_preview_position());

        // Update md5 model matrix
        _uniforms.update_md5_model(_state.get_model_matrix());

        // Update ui matrices
        _uniforms.update_ui(_ui.get_scale(), _ui.get_uv());

        // Update mob and missile matrices
        const game::static_instance &instance = _world.get_instances();
        _uniforms.update_mobs(instance.get_cube_matrices());
        _uniforms.update_missiles(instance.get_missile_matrices());

        // Update md5 model bones
        if (update_bones)
        {
            _uniforms.update_bones(_character.get_bones());
        }

        // Update all matrices
        _uniforms.update_matrix_buffer();
    }

  public:
    // Load window shaders and program
    fractex(const size_t chunk, const size_t view)
        : _win("Fractex", _width, _height, _gl_major, _gl_minor),
          _uniforms(),
          _ui(_uniforms, _win.get_width(), _win.get_height()),
          _particles(_uniforms),
          _character(&_particles, _uniforms),
          _state(),
          _world(_state.get_load_state(), &_particles, _uniforms, 64, chunk, view),
          _controls(_win, _state.get_camera(), _character, _state, _ui, _world),
          _title(_state.get_camera(), _ui, _win),
          _goal_seek(_world)
    {
        // Set depth and cull settings
        min::settings::initialize();

        // Update cursor position for tracking
        update_cursor();

        // Test adding a mob
        const min::vec3<float> p(-4.5, 30.5, 4.5);
        _world.add_mob(p);
    }
    ~fractex()
    {
        // Save game data to file
        _state.save_state(_world.character_position());
    }
    void clear_background() const
    {
        // blue background
        const float color[] = {0.690, 0.875f, 0.901f, 1.0f};
        glClearBufferfv(GL_COLOR, 0, color);
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    void disable_title_screen()
    {
        // Register window callbacks
        _controls.register_control_callbacks();

        // Stop drawing title in UI
        _ui.set_draw_title(false);

        // Turn off cursor
        _win.display_cursor(false);

        // Maximize window
        _win.maximize();
    }
    void draw(const float dt)
    {
        // Get player physics body position
        const min::vec3<float> &p = _world.character_position();

        bool update = false;
        min::camera<float> &camera = _state.get_camera();

        // If game is not paused update game state
        if (!_state.get_game_pause())
        {
            // get user input
            const auto c = user_input();

            // Must update state properties, camera before drawing world
            _state.update(p, c, _win.get_width(), _win.get_height(), dt);

            // If AI is in control
            if (_world.get_ai_mode())
            {
                // Perform goal seek
                _goal_seek.seek(_world, 0);
            }

            // Update the world state
            _world.update(camera, dt);

            // Check if we died
            update_die_respawn();

            // Update the particle system
            _particles.set_velocity(_world.character_velocity());
            _particles.update(camera, dt);

            // Update the character state
            update = _character.update(camera, dt);

            // Update control class
            _controls.update(dt);
        }

        // Update all uniforms
        update_uniforms(camera, update);

        // Draw world geometry
        _world.draw(_uniforms);

        // Draw the character if fire mode activated
        if (_state.get_skill_state().is_gun_active())
        {
            _character.draw(_uniforms);
        }

        // Draw the ui
        _ui.draw(_uniforms);
    }
    void draw_title(const float dt)
    {
        min::camera<float> &camera = _state.get_camera();

        // Update all uniforms
        update_uniforms(camera, false);

        // Draw the ui
        _ui.draw(_uniforms);
    }
    bool is_closed() const
    {
        return _win.get_shutdown();
    }
    bool is_show_title() const
    {
        return _title.is_show_title();
    }
    void set_title(const std::string &title)
    {
        _win.set_title(title);
    }
    void update_cursor()
    {
        // Get the screen dimensions
        const uint16_t w = _win.get_width();
        const uint16_t h = _win.get_height();

        // Center cursor in middle of window
        _win.set_cursor(w / 2, h / 2);
    }
    void update_keyboard(const float dt)
    {
        // Update the keyboard
        auto &keyboard = _win.get_keyboard();
        keyboard.update(dt);
    }
    void update_text(const double fps, const double idle)
    {
        // Update player position debug text
        const min::vec3<float> &p = _world.character_position();
        const min::vec3<float> &f = _state.get_camera().get_forward();
        const std::string &mode = _state.get_game_mode();
        const min::vec3<float> &goal = _goal_seek.get_goal();
        const float health = _state.get_health();
        const float energy = _state.get_skill_state().get_energy();

        // Update all text and upload it
        _ui.update(p, f, mode, goal, health, energy, fps, idle);
    }
    void update_window()
    {
        // Update and swap buffers
        _win.update();
        _win.swap_buffers();
    }
};

#endif
