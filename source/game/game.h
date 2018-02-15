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
#ifndef __GAME_HEADER__
#define __GAME_HEADER__

#include <game/controls.h>
#include <game/events.h>
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

class bds
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
    game::particle _particles;
    game::character _character;
    game::state _state;
    game::world _world;
    game::events _events;
    game::ui_overlay _ui;
    game::controls _controls;
    game::title _title;
    game::sound _sound;
    std::pair<uint16_t, uint16_t> _cursor;
    double _fps;
    double _idle;

    void set_cursor_center()
    {
        // Get the screen dimensions
        const uint16_t w = _win.get_width();
        const uint16_t h = _win.get_height();

        // Center cursor in middle of window
        _win.set_cursor(w / 2, h / 2);
    }
    std::pair<uint16_t, uint16_t> cursor_center() const
    {
        // Get the screen dimensions
        const uint16_t w2 = _win.get_width() / 2;
        const uint16_t h2 = _win.get_height() / 2;

        // Return screen center
        return std::make_pair(w2, h2);
    }
    std::pair<uint16_t, uint16_t> get_cursor() const
    {
        // If player is dead return screen center
        if (_world.get_player().is_dead())
        {
            return cursor_center();
        }

        // Return cursor position
        return _win.get_cursor();
    }
    void load_gpu_info()
    {
        // Get OpenGL implementation info
        const char *vendor = _win.get_context_string(GL_VENDOR);
        const char *render = _win.get_context_string(GL_RENDERER);
        const char *version = _win.get_context_string(GL_VERSION);
        const char *shading = _win.get_context_string(GL_SHADING_LANGUAGE_VERSION);

        // Print out the OpenGL information that we got
        std::cout << "OpenGL Vendor: " << vendor << std::endl;
        std::cout << "OpenGL Renderer: " << render << std::endl;
        std::cout << "OpenGL Version: " << version << std::endl;
        std::cout << "OpenGL Shading Version: " << shading << std::endl;

        // Set the debug strings in ui overlay
        _ui.text().set_debug_title("Beyond Dying Skies: Official Demo");
        _ui.text().set_debug_vendor(vendor);
        _ui.text().set_debug_renderer(render);
        _ui.text().set_debug_version("VERSION: 0.1.193");
    }
    void update_die_respawn(const float dt)
    {
        game::player &player = _world.get_player();

        // Check if we need to respawn
        if (_state.is_respawn())
        {
            // Refresh state
            _state.respawn();

            // Refresh ui
            _ui.respawn();

            // Refresh the world exploded flag
            _world.respawn(_state.get_default_spawn());

            // Reset control class
            _controls.respawn();

            // Enable the keyboard
            auto &keyboard = _win.get_keyboard();
            keyboard.enable();

            // Reset the voice queue
            _sound.reset_voice_queue();
        }
        else if (player.is_dead())
        {
            // Check for debounce
            if (!_state.is_dead())
            {
                // Set menu for dead
                _ui.set_menu_dead();

                // Disable the keyboard
                auto &keyboard = _win.get_keyboard();
                keyboard.disable();

                // Play shutdown sound
                _sound.play_voice_shutdown();

                // Debounce this routine
                _state.set_dead(true);
            }
        }
    }
    void update_ui(const float dt)
    {
        // Update player position debug text
        const game::player &play = _world.get_player();
        const game::stats &stat = play.get_stats();
        const min::vec3<float> &p = play.position();
        const min::vec3<float> &f = _state.get_camera().get_forward();
        const float health = stat.get_health();
        const float energy = stat.get_energy();
        const size_t chunks = _world.get_chunks_in_view();

        // Update the ui overlay
        _ui.update_text(p, f, health, energy, _fps, _idle, chunks);

        // Process timer and upload changes
        _ui.update(dt);
    }
    void update_uniforms(min::camera<float> &camera, const bool update_bones)
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
        _uniforms.update_drones(instance.get_drone_matrix());
        _uniforms.update_drops(instance.get_drop_matrix());
        _uniforms.update_explosives(instance.get_explosive_matrix());
        _uniforms.update_missiles(instance.get_missile_matrix());

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
    bds(const size_t chunk, const size_t grid, const size_t view, const size_t width, const size_t height)
        : _win("Beyond Dying Skies Official", width, height, _gl_major, _gl_minor),
          _uniforms(),
          _particles(_uniforms),
          _character(&_particles, _uniforms),
          _state(grid),
          _world(_state.get_load_state(), &_particles, &_sound, _uniforms, chunk, grid, view),
          _ui(_uniforms, &_world.get_player().get_inventory(), &_world.get_player().get_stats(), _win.get_width(), _win.get_height()),
          _controls(_win, _state.get_camera(), _character, _state, _ui, _world, _sound),
          _title(_state.get_camera(), _ui, _win), _fps(0.0), _idle(0.0)
    {
        // Set depth and cull settings
        min::settings::initialize();

        // Update cursor position for tracking
        set_cursor_center();

        // Delete the mem-file data
        game::memory_map::memory.clear();

        // Load gpu information
        load_gpu_info();
    }
    ~bds()
    {
        // Get player object
        game::player &play = _world.get_player();

        // Save game data to file
        _state.save_state(play.get_inventory(), play.position());
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
        const float color[] = {0.145, 0.145f, 0.150f, 1.0f};
        glClearBufferfv(GL_COLOR, 0, color);
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    void disable_title_screen()
    {
        // Register window callbacks
        _controls.register_control_callbacks();

        // Stop drawing title in UI
        _ui.set_draw_title(false);
        _ui.set_draw_text_ui(true);

        // Turn off cursor
        _win.display_cursor(false);

        // Update the mouse cursor to center
        set_cursor_center();
    }
    void draw() const
    {
        // Draw the opaque ui
        _ui.draw_opaque();

        // Draw the character if fire mode activated
        const game::player &play = _world.get_player();
        if (play.get_mode() == game::play_mode::gun)
        {
            _character.draw();
        }

        // Draw world geometry
        _world.draw(_uniforms);

        // Draw particles
        _particles.draw();

        // Draw the transparent ui
        _ui.draw_transparent();

        // Draw the tooltip ui
        _ui.draw_tooltips();
    }
    void draw_title(const float dt)
    {
        min::camera<float> &camera = _state.get_camera();

        // Update all uniforms
        update_uniforms(camera, false);

        // Draw the ui
        _ui.draw_opaque();
        _ui.draw_transparent();
    }
    bool is_closed() const
    {
        return _win.get_shutdown();
    }
    bool is_show_title() const
    {
        return _title.is_show_title();
    }
    void maximize() const
    {
        _win.maximize();
    }
    void play_music()
    {
        _sound.play_bg(true);
    }
    void update(const float dt)
    {
        // Get player object
        game::player &player = _world.get_player();

        bool update = false;
        min::camera<float> &camera = _state.get_camera();

        // If game is not paused update game state
        if (!_state.get_pause())
        {
            if (!_state.get_user_input())
            {
                // Get the cursor coordinates
                const auto c = get_cursor();

                // Must update state properties, camera before drawing world
                _state.update(player.position(), c, _win.get_width(), _win.get_height());

                // Reset cursor position
                set_cursor_center();
            }
            else if (_state.get_user_input())
            {
                // Get the cursor coordinates
                const auto c = _win.get_cursor();

                // Flip the Y value to match screen coordinates
                const uint16_t height = _win.get_height() - c.second;
                _ui.overlap(min::vec2<float>(c.first, height));

                // Calculate the center of the screen
                const auto center = std::make_pair(_win.get_width() / 2, _win.get_height() / 2);

                // Must update state properties, camera before drawing world
                _state.update(player.position(), center, _win.get_width(), _win.get_height());
            }

            // Update the game events
            _events.update(_world, _ui, dt);

            // Update the world state
            _world.update(camera, dt);

            // Update the particle system
            _particles.set_velocity(player.velocity());
            _particles.update(camera, dt);

            // Update the player walk sound if moving and not falling
            if (player.is_landed())
            {
                // Volume is proportional to square y velocity
                const min::vec3<float> &lv = player.land_velocity();
                const float speed = std::abs(lv.y());
                _sound.play_land(speed);

                // Reset land flag
                player.reset_landed();
            }

            // Update the character state
            update = _character.update(camera, dt);

            // Update control class
            _controls.update();

            // Update the UI class
            update_ui(dt);

            // Check if we died
            update_die_respawn(dt);
        }

        // Update the sound listener properties
        const min::vec3<float> &v = player.velocity();
        _sound.update(camera, v, dt);

        // Update all uniforms
        update_uniforms(camera, update);
    }
    void update_keyboard(const float dt)
    {
        // Update the keyboard
        auto &keyboard = _win.get_keyboard();
        keyboard.update(dt);
    }
    void update_fps(const double fps, const double idle)
    {
        _fps = fps;
        _idle = idle;
    }
    void update_window()
    {
        // Update and swap buffers
        _win.update();
        _win.swap_buffers();
    }
};

#endif
