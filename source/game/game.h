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
#include <game/def.h>
#include <game/events.h>
#include <game/keymap.h>
#include <game/options.h>
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

    game::options _opt;
    min::window _win;
    game::uniforms _uniforms;
    game::particle _particles;
    game::sound _sound;
    game::character _character;
    game::world _world;
    game::state _state;
    game::events _events;
    game::ui_overlay _ui;
    game::key_map _keymap;
    game::title _title;
    game::controls _controls;
    std::pair<uint_fast16_t, uint_fast16_t> _cursor;
    double _fps;
    double _idle;

    void center_cursor()
    {
        // Get the screen dimensions
        const uint_fast16_t w = _win.get_width();
        const uint_fast16_t h = _win.get_height();

        // Center cursor in middle of window
        _win.set_cursor(w / 2, h / 2);
    }
    std::pair<uint_fast16_t, uint_fast16_t> get_center() const
    {
        // Get the screen dimensions
        const uint_fast16_t w2 = _win.get_width() / 2;
        const uint_fast16_t h2 = _win.get_height() / 2;

        // Return screen center
        return std::make_pair(w2, h2);
    }
    std::pair<uint_fast16_t, uint_fast16_t> get_cursor() const
    {
        // If player is dead return screen center
        if (_world.get_player().is_dead())
        {
            return get_center();
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
        _ui.text().set_debug_version("VERSION: 0.1.304");
    }
    void load_game_mode()
    {
        // Set the game mode
        const game::game_type gt = _opt.get_game_mode();
        if (gt == game::game_type::NORMAL)
        {
            std::cout << "Loading game in NORMAL mode" << std::endl;
            _ui.text().set_debug_game_mode("NORMAL MODE");
        }
        else if (gt == game::game_type::HARDCORE)
        {
            std::cout << "Loading game in HARDCORE mode" << std::endl;
            _ui.text().set_debug_game_mode("HARDCORE MODE");
        }
        else if (gt == game::game_type::CREATIVE)
        {
            std::cout << "Loading game in CREATIVE mode" << std::endl;
            _ui.text().set_debug_game_mode("CREATIVE MODE");
        }
    }
    void update_alerts()
    {
        // Get the stats from bg;
        game::stats &stat = _world.get_player().get_stats();
        const game::stat_alert alert = stat.get_alert();

        // Do action on an alert
        switch (alert)
        {
        case game::stat_alert::level:
            _sound.play_voice_level();
            _ui.set_alert_level();
            break;
        case game::stat_alert::dynamics:
            _sound.play_voice_thrust_alert();
            _ui.set_alert_dynamics();
            break;
        default:
            break;
        }

        // Clear any alerts
        stat.clear_alert();
    }
    void update_die_respawn(const float dt)
    {
        game::player &player = _world.get_player();

        // Check if we need to respawn
        if (_state.is_respawn())
        {
            // Refresh the world exploded flag
            _world.respawn(_opt);

            // Refresh state
            _state.respawn(_world.get_load_state());

            // Refresh ui
            _ui.respawn();

            // Reset control class
            _controls.respawn();

            // Enable the keyboard
            auto &keyboard = _win.get_keyboard();
            keyboard.enable();

            // Reset the voice queue
            _sound.reset_voice_queue();

            // Reset the events
            _events.reset(_world, _ui);
        }
        else if (player.is_dead())
        {
            // Check for debounce
            if (!_state.is_dead())
            {
                // Close GUI if up
                _controls.die();

                // Set menu for dead
                _ui.set_splash_dead();

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
        game::player &play = _world.get_player();
        game::stats &stat = play.get_stats();
        const min::vec3<float> &p = play.position();
        const min::vec3<float> &f = _state.get_camera().get_forward();
        const float health = stat.get_health();
        const float energy = stat.get_energy();
        const size_t chunks = _world.get_chunks_in_view();
        const size_t insts = _world.get_inst_in_view();

        // Check if player gave damage
        if (stat.is_crit())
        {
            // Add stream text
            _ui.add_stream_float("Crit!: ", stat.get_gave_dmg());

            // Clear the crit and dmg flags
            stat.clear_crit();
        }
        else if (stat.is_gave_dmg())
        {
            // Add stream text
            _ui.add_stream_float("Hit: ", stat.get_gave_dmg());

            // Clear the dmg flag
            stat.clear_gave_dmg();
        }

        // Check if player took damage
        if (stat.is_took_dmg())
        {
            // Add stream text
            _ui.add_stream_float("Damage: ", stat.get_took_dmg());

            // Clear the hit flag
            stat.clear_took_dmg();
        }

        // Get the target info
        const auto info = _world.get_target_info(play.get_target());

        // Get the drone time
        const float time = _events.get_drone_time();
        _ui.set_draw_timer((time > 0.0) && !_ui.is_focused());

        // Update the ui overlay, process timer and upload changes
        _ui.update(p, f, health, energy, _fps, _idle, chunks, insts, *info.first, time, dt);
    }
    void update_uniforms(min::camera<float> &camera, const bool update_bones)
    {
        // Bind uniforms
        _uniforms.bind();

        // Update the light position
        _uniforms.update_light_position(camera.get_position());
        _uniforms.update_light();

        // Update camera properties
        _uniforms.update_camera(camera);

        // Update world preview matrix
        _uniforms.update_preview(_world.get_preview_matrix());

        // Update md5 model matrix
        _uniforms.update_md5_model(_state.get_model_matrix());

        // Update ui matrices
        _uniforms.update_ui(_ui.get_scale(), _ui.get_uv());

        // Update drone and missile matrices
        const game::static_instance &instance = _world.get_instance();
        _uniforms.update_chests(instance.get_chest().get_out_matrix());
        _uniforms.update_drones(instance.get_drone().get_out_matrix());
        _uniforms.update_drops(instance.get_drop().get_out_matrix());
        _uniforms.update_explosives(instance.get_explosive().get_out_matrix());
        _uniforms.update_missiles(instance.get_missile().get_out_matrix());

        // Update md5 model bones
        if (update_bones)
        {
            _uniforms.update_bones(_character.get_bones());
        }

        // Update all matrices
        _uniforms.update_light_buffer();
        _uniforms.update_matrix_buffer();
    }

  public:
    // Load window shaders and program
    bds(const game::options &opt)
        : _opt(opt),
          _win("Beyond Dying Skies Official", opt.width(), opt.height(), _gl_major, _gl_minor),
          _uniforms(),
          _particles(_uniforms),
          _character(&_particles, _uniforms),
          _world(opt, _particles, _sound, _uniforms),
          _state(opt, _world.get_load_state()),
          _ui(_uniforms, _world.get_player().get_inventory(), _world.get_player().get_stats(), _win.get_width(), _win.get_height()),
          _keymap(opt),
          _title(_opt, _particles, _win, _sound, _character, _world, _state, _events, _ui, _keymap),
          _controls(_opt, _win, _sound, _character, _world, _state, _ui, _keymap, _title),
          _fps(0.0), _idle(0.0)
    {
        // Set depth and cull settings
        min::settings::initialize();
        min::settings::enable_gamma_correction();

        // Delete the mem-file data
        game::memory_map::memory.clear();

        // Load GPU info
        load_gpu_info();

        // Show the window
        _win.show();
    }
    void blink_console_message()
    {
        _ui.blink_console();
    }
    bool check_al_error() const
    {
        return _sound.check_error();
    }
    void throw_fatal_error() const
    {
        min::throw_al_error();
        min::throw_gl_error();
    }
    bool check_gl_error() const
    {
        return min::check_gl_error();
    }
    void clear_background() const
    {
        // blue background
        const float color[] = {0.025f, 0.025f, 0.025f, 1.0f};
        glClearBufferfv(GL_COLOR, 0, color);
        glClear(GL_DEPTH_BUFFER_BIT);
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
    void draw_title()
    {
        // Draw the ui
        _ui.draw_opaque();
        _ui.draw_transparent();
    }
    void error_message(const std::string &error)
    {
        _win.error_message(error);
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
    void title_screen_disable()
    {
        // Enable control mode
        _controls.enable();

        // Load gpu information
        load_game_mode();

        // Center cursor
        center_cursor();
    }
    void title_screen_enable()
    {
        // Reset the game state
        _title.enable();

        // Center cursor
        center_cursor();

        // Reset loop variables
        _fps = 0.0;
        _idle = 0.0;
    }
    void update(const float dt)
    {
        // Get player object
        game::player &player = _world.get_player();
        const min::vec3<float> &v = player.velocity();

        bool update = false;
        min::camera<float> &camera = _state.get_camera();

        // Process UI if user input
        if (_state.get_user_input())
        {
            // Get the cursor coordinates
            const auto c = _win.get_cursor();

            // Flip the Y value to match screen coordinates
            const uint_fast16_t height = _win.get_height() - c.second;
            _ui.overlap(min::vec2<float>(c.first, height));
        }

        // If game is not paused update game state
        if (!_state.get_pause())
        {
            // Update stat alerts
            update_alerts();

            // Calculate player velocity if not falling
            const float v_mag = (player.is_falling()) ? 0.0 : v.dot(player.forward());

            // Get user input
            if (!_state.get_user_input())
            {
                // Get the cursor coordinates
                const auto c = get_cursor();

                // Must update state properties, camera before drawing world
                _state.update(player.position(), c, _win.get_width(), _win.get_height(), v_mag, dt);

                // Reset cursor position
                center_cursor();
            }
            else if (_state.get_user_input())
            {
                // Calculate the center of the screen, to avoid camera movement
                const auto center = std::make_pair(_win.get_width() / 2, _win.get_height() / 2);

                // Must update state properties, camera before drawing world
                _state.update(player.position(), center, _win.get_width(), _win.get_height(), v_mag, dt);
            }

            // Update the game events
            _events.update(_world, _ui, dt);

            // Update the world state
            _world.update(camera, _state.get_tracking(), dt);

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
                player.clear_landed();
            }

            // Update the character state
            update = _character.update(camera, dt);

            // Update control class
            _controls.update();

            // Check if we died
            update_die_respawn(dt);
        }

        // Update the UI class
        update_ui(dt);

        // Update the sound listener properties
        _sound.update(camera, v, dt);

        // Update all uniforms
        update_uniforms(camera, update);
    }
    void update_title(const float dt)
    {
        min::camera<float> &camera = _state.get_camera();

        // Process UI if user input
        if (_state.get_user_input())
        {
            // Get the cursor coordinates
            const auto c = _win.get_cursor();

            // Flip the Y value to match screen coordinates
            const uint_fast16_t height = _win.get_height() - c.second;
            _ui.overlap(min::vec2<float>(c.first, height));
        }

        // Update the UI class
        _ui.update_title();

        // Update the sound listener properties
        _sound.update(camera, min::vec3<float>(), dt);

        // Update all uniforms
        update_uniforms(camera, false);
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
    void update_second()
    {
        // Update the events
        _events.update_second(_world);
    }
    void update_window()
    {
        // Update and swap buffers
        _win.update();
        _win.swap_buffers();
    }
};

#endif
