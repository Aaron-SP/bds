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
#ifndef __TITLE_CONTROLS__
#define __TITLE_CONTROLS__

#include <game/events.h>
#include <game/file.h>
#include <game/keymap.h>
#include <game/options.h>
#include <game/particle.h>
#include <game/sound.h>
#include <game/state.h>
#include <game/title.h>
#include <game/ui_overlay.h>
#include <game/world.h>
#include <iostream>
#include <min/camera.h>
#include <min/window.h>
#include <stdexcept>

namespace game
{

class title
{
  private:
    options *const _opt;
    game::particle *const _particles;
    min::window *const _win;
    sound *const _sound;
    character *const _character;
    world *const _world;
    state *const _state;
    min::camera<float> *const _camera;
    events *const _events;
    ui_overlay *const _ui;
    key_map *const _keymap;

    min::camera<float> *get_camera()
    {
        return _camera;
    }
    options *get_options()
    {
        return _opt;
    }
    sound *get_sound()
    {
        return _sound;
    }
    state *get_state()
    {
        return _state;
    }
    ui_overlay *get_ui()
    {
        return _ui;
    }
    min::window *get_window()
    {
        return _win;
    }
    game::menu_call menu_new_game_call()
    {
        // Create resume callback
        return [this]() -> void {
            // Set the next level menu
            game::ui_menu &menu = this->_ui->get_menu();
            menu.reset_save_menu();

            // Load next level callbacks
            for (size_t i = 0; i < 5; i++)
            {
                if (!exists_file("save/state." + std::to_string(i)))
                {
                    const auto f = [this, i]() -> void {
                        // Set the save slot
                        this->_opt->set_save_slot(i);

                        // Start new game
                        this->_world->new_game(*this->_opt);

                        // Load the keymap
                        this->_keymap->load(i);

                        // Reset the game
                        this->reset_game();

                        // Advance to the game controller
                        this->set_show_title(false);

                        // Disable user input
                        this->_state->set_user_input(false);
                    };

                    // Set the save callback
                    menu.set_string_empty_save(i);
                    menu.set_callback(i, f);
                }
            }
        };
    }
    game::menu_call menu_load_game_call()
    {
        // Create resume callback
        return [this]() -> void {
            // Set the next level menu
            game::ui_menu &menu = this->_ui->get_menu();
            menu.reset_save_menu();

            // Load next level callbacks
            for (size_t i = 0; i < 5; i++)
            {
                if (exists_file("save/state." + std::to_string(i)))
                {
                    const auto f = [this, i]() -> void {
                        // Set the save slot
                        this->_opt->set_save_slot(i);

                        // Load the world
                        this->_world->load(*this->_opt);

                        // Load the keymap
                        this->_keymap->load(i);

                        // Reset the game
                        this->reset_game();

                        // Advance to the game controller
                        this->set_show_title(false);

                        // Disable user input
                        this->_state->set_user_input(false);
                    };

                    // Set the save callback
                    menu.set_callback(i, f);
                }
                else
                {
                    menu.set_string_empty_save(i);

                    // Reset the menu
                    const auto f = [this]() -> void {
                        this->reset_menu();
                    };

                    // Set the save callback
                    menu.set_callback(i, f);
                }
            }
        };
    }
    game::menu_call menu_delete_game_call()
    {
        // Create resume callback
        return [this]() -> void {
            // Set the next level menu
            game::ui_menu &menu = this->_ui->get_menu();
            menu.reset_save_menu();

            // Load next level callbacks
            for (size_t i = 0; i < 5; i++)
            {
                if (exists_file("save/state." + std::to_string(i)))
                {
                    const auto f = [this, i]() -> void {
                        // If deleted save
                        if (erase_save(i))
                        {
                            game::ui_menu &menu = this->_ui->get_menu();
                            menu.set_string_empty_save(i);

                            // Reset the menu
                            const auto f = [this]() -> void {
                                this->reset_menu();
                            };

                            // Set the save callback
                            menu.set_callback(i, f);
                            menu.make_dirty();
                        }
                    };

                    // Set the save callback
                    menu.set_callback(i, f);
                }
                else
                {
                    menu.set_string_empty_save(i);

                    // Reset the menu
                    const auto f = [this]() -> void {
                        this->reset_menu();
                    };

                    // Set the save callback
                    menu.set_callback(i, f);
                }
            }
        };
    }
    game::menu_call menu_quit_game_call()
    {
        return [this]() -> void {
            // Return to title
            this->set_show_title(false);

            // Quit game
            this->_win->set_shutdown();

            // Alert that we received the call back
            std::cout << "controls: Shutdown called by user" << std::endl;
        };
    }
    void reset_menu()
    {
        game::ui_menu &menu = _ui->get_menu();
        menu.reset_title_menu();
        menu.set_callback(0, menu_new_game_call());
        menu.set_callback(1, menu_load_game_call());
        menu.set_callback(2, menu_delete_game_call());
        menu.set_callback(3, menu_quit_game_call());
    }
    void reset_game()
    {
        // Reset core game components
        _particles->reset();
        _character->reset();
        _world->reset(*_opt);
        *_state = game::state(*_opt, _world->get_load_state());
        *_events = game::events();
    }

  public:
    title(options &opt, particle &particles, min::window &window, sound &sound, character &ch, world &world,
          state &state, game::events &events, ui_overlay &ui, key_map &km)
        : _opt(&opt), _particles(&particles), _win(&window), _sound(&sound), _character(&ch), _world(&world),
          _state(&state), _camera(&state.get_camera()), _events(&events), _ui(&ui),
          _keymap(&km)
    {
        // Register callbacks
        register_control_callbacks();
    }
    void register_control_callbacks()
    {
        // Enable the console and set default message
        _ui->enable_console();
        _ui->set_console_string("Click To Start");

        // Get access to the keyboard
        auto &keyboard = _win->get_keyboard();

        // Clear any keys mapped to keyboard
        keyboard.clear();

        // Register data and function callbacks
        _win->register_data((void *)this);
        _win->register_lclick_down(title::left_click_down);
        _win->register_lclick_up(title::left_click_up);
        _win->register_rclick_down(nullptr);
        _win->register_rclick_up(nullptr);
        _win->register_update(title::on_resize);

        // Add keys to watch
        keyboard.add((*_keymap)[23]);

        // Register callback functions
        keyboard.register_keydown((*_keymap)[23], title::escape_menu, (void *)this);
    }
    static void escape_menu(void *const ptr, double step)
    {
        title *const title = reinterpret_cast<game::title *>(ptr);
        title->reset_menu();
    }
    static void left_click_down(void *ptr, const uint_fast16_t x, const uint_fast16_t y)
    {
        // Cast to title pointer
        title *const title = reinterpret_cast<game::title *>(ptr);
        state *const state = title->get_state();
        ui_overlay *const ui = title->get_ui();
        sound *const sound = title->get_sound();

        const bool input = state->get_user_input();
        if (input)
        {
            // Do mouse click on UI
            if (ui->click_down())
            {
                // Play click sound
                sound->play_click();
            }

            // Early exit
            return;
        }
        else
        {
            // Show the menu on the title screen
            state->set_user_input(true);
            ui->switch_mode_menu();

            // Set the menu and update
            title->reset_menu();
            ui->update_title();
        }
    }
    static void left_click_up(void *const ptr, const uint_fast16_t x, const uint_fast16_t y)
    {
        // Get the ui pointer
        title *const title = reinterpret_cast<game::title *>(ptr);
        ui_overlay *const ui = title->get_ui();

        // Click up event
        ui->click_up();
    }
    void enable()
    {
        // Register window callbacks
        register_control_callbacks();

        // Reset the sound and ui
        _sound->reset();
        _state->set_user_input(false);
        _ui->reset();

        // Get the screen dimensions
        const uint_fast16_t w = _win->get_width();
        const uint_fast16_t h = _win->get_height();
        const uint_fast16_t w2 = w / 2;
        const uint_fast16_t h2 = h / 2;
        _ui->set_screen(min::vec2<float>(w2, h2), w, h);

        // Turn off cursor
        _win->display_cursor(true);
    }
    bool is_show_title() const
    {
        return _ui->is_title_mode();
    }
    static void on_resize(void *ptr, const uint_fast16_t width, const uint_fast16_t height)
    {
        // Ignore minimizing window
        if (width == 0 && height == 0)
        {
            return;
        }

        // Cast to title pointer
        title *const title = reinterpret_cast<game::title *>(ptr);
        options *const opt = title->get_options();
        min::camera<float> *const camera = title->get_camera();
        game::ui_overlay *const ui = title->get_ui();

        // Set the current window width and height
        opt->set_width(width);
        opt->set_height(height);

        // Get camera frustum
        auto &f = camera->get_frustum();

        // Update the aspect ratio
        f.set_aspect_ratio(width, height);
        f.make_dirty();
        camera->make_dirty();

        // Get the screen dimensions
        const uint_fast16_t w2 = width / 2;
        const uint_fast16_t h2 = height / 2;

        // Update the screen size for ui and text
        ui->set_screen(min::vec2<float>(w2, h2), width, height);
    }
    void set_show_title(const bool flag)
    {
        _ui->set_title_mode(flag);
    }
};
}

#endif
