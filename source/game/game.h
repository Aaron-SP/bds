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
#include <game/particle.h>
#include <game/sound.h>
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

    min::window _win;
    game::uniforms _uniforms;
    game::ui_overlay _ui;
    game::particle _particles;
    game::character _character;
    game::state _state;
    game::world _world;
    game::controls _controls;
    game::title _title;
    game::sound _sound;

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
        const game::player &player = _world.get_player();
        if (_state.get_user_input() && !player.is_dead())
        {
            // Get the cursor coordinates
            const auto c = _win.get_cursor();

            // Reset cursor position
            center_cursor();

            // return the mouse coordinates
            return c;
        }

        // return no mouse movement
        return std::make_pair(_win.get_width() / 2, _win.get_height() / 2);
    }
    inline void update_die_respawn()
    {
        game::player &player = _world.get_player();

        // Check if we exploded
        if (!player.is_dead())
        {
            if (player.is_exploded())
            {
                const int8_t ex_id = _world.get_player().get_explode_id();
                if (ex_id == 21)
                {
                    player.consume_health(90.0);
                }
                else
                {
                    player.consume_health(10.0);
                }
            }
        }
        else if (_state.is_respawn())
        {
            // Respawn
            respawn();
        }
        else if (player.is_dead())
        {
            die();
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
        _uniforms.update_preview(_world.get_preview_matrix());

        // Update md5 model matrix
        _uniforms.update_md5_model(_state.get_model_matrix());

        // Update ui matrices
        _uniforms.update_ui(_ui.get_scale(), _ui.get_uv());

        // Update drone and missile matrices
        const game::static_instance &instance = _world.get_instances();
        _uniforms.update_drones(instance.get_drone_matrices());
        _uniforms.update_drops(instance.get_drop_matrices());
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
    fractex(const size_t chunk, const size_t grid, const size_t view, const size_t width, const size_t height)
        : _win("Fractex", width, height, _gl_major, _gl_minor),
          _uniforms(),
          _ui(_uniforms, _win.get_width(), _win.get_height()),
          _particles(_uniforms),
          _character(&_particles, _uniforms),
          _state(grid),
          _world(_state.get_load_state(), &_particles, &_sound, _uniforms, chunk, grid, view),
          _controls(_win, _state.get_camera(), _character, _state, _ui, _world, _sound),
          _title(_state.get_camera(), _ui, _win)
    {
        // Set depth and cull settings
        min::settings::initialize();

        // Update cursor position for tracking
        center_cursor();

        // Delete the mem-file data
        game::memory_map::memory.clear();
    }
    ~fractex()
    {
        // Save game data to file
        _state.save_state_file(_world.get_player().position());
    }
    void blink_console_message()
    {
        _ui.toggle_console();
    }
    bool check_gl_error() const
    {
        return min::check_gl_error();
    }
    bool check_al_error() const
    {
        return _sound.check_error();
    }
    void clear_background() const
    {
        // blue background
        const float color[] = {0.690, 0.875f, 0.901f, 1.0f};
        glClearBufferfv(GL_COLOR, 0, color);
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    void disable_title_screen(const bool max)
    {
        // Register window callbacks
        _controls.register_control_callbacks();

        // Stop drawing title in UI
        _ui.set_draw_title(false);
        _ui.set_draw_text_ui(true);

        // Turn off cursor
        _win.display_cursor(false);

        // Maximize window?
        if (max)
        {
            _win.maximize();
        }

        // Update the mouse cursor to center
        center_cursor();
    }
    void draw(const float dt)
    {
        // Get player physics body position
        const min::vec3<float> &p = _world.get_player().position();

        bool update = false;
        min::camera<float> &camera = _state.get_camera();

        // If game is not paused update game state
        if (!_state.get_game_pause())
        {
            // get user input
            const auto c = user_input();

            // Must update state properties, camera before drawing world
            _state.update(p, c, _win.get_width(), _win.get_height());

            // Update the world state
            _world.update(camera, dt);

            // Update the sound listener properties
            _sound.update(camera, _world.get_player().velocity());

            // Check if we died
            update_die_respawn();

            // Update the particle system
            const min::vec3<float> &v = _world.get_player().velocity();
            _particles.set_velocity(v);
            _particles.update(camera, dt);

            // Update the player walk sound if moving and not falling
            if (_world.get_player().has_landed())
            {
                // Volume is proportional to square y velocity
                const min::vec3<float> &lv = _world.get_player().land_velocity();
                const float speed = std::abs(lv.y());
                _sound.play_land(speed);
            }

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
        const game::skills &skill = _world.get_player().get_skills();
        if (skill.is_gun_active())
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
    void play_music()
    {
        _sound.play_bg(true);
    }
    void set_title(const std::string &title)
    {
        _win.set_title(title);
    }
    void center_cursor()
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
        const game::player &player = _world.get_player();
        const game::skills &skills = player.get_skills();
        const min::vec3<float> &p = _world.get_player().position();
        const min::vec3<float> &f = _state.get_camera().get_forward();
        const std::string &mode = _state.get_game_mode();
        const float health = player.get_health();
        const float energy = skills.get_energy();

        // Update all text and upload it
        _ui.update(p, f, mode, health, energy, fps, idle);
    }
    void update_window()
    {
        // Update and swap buffers
        _win.update();
        _win.swap_buffers();
    }
};

#endif
