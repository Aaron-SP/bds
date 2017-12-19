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
#ifndef __CONTROLS__
#define __CONTROLS__

#include <game/state.h>
#include <game/text.h>
#include <game/ui_overlay.h>
#include <game/world.h>
#include <iostream>
#include <min/camera.h>
#include <min/ray.h>
#include <min/window.h>
#include <stdexcept>
#include <thread>

namespace game
{

class controls
{
  private:
    min::window *_window;
    min::camera<float> *_camera;
    game::character *_character;
    game::state *_state;
    game::text *_text;
    game::ui_overlay *_ui;
    game::world *_world;

  public:
    controls(min::window &window, min::camera<float> &camera,
             game::character &ch, game::state &state,
             game::text &text, game::ui_overlay &ui, game::world &world)
        : _window(&window), _camera(&camera), _character(&ch),
          _state(&state), _text(&text), _ui(&ui), _world(&world)
    {
        // Check that pointers are valid
        if (!_window || !_character || !_camera || !_state || !_text || !_ui || !_world)
        {
            throw std::runtime_error("control: Invalid control pointers");
        }

        // Get access to the keyboard
        auto &keyboard = _window->get_keyboard();

        // Register click callback function for placing path
        _window->register_data((void *)this);

        // Register left click events
        _window->register_lclick_down(controls::left_click_down);
        _window->register_lclick_up(controls::left_click_up);
        _window->register_rclick_down(controls::right_click_down);
        _window->register_rclick_up(controls::right_click_up);
        _window->register_update(controls::on_resize);

        // Add FPS(WADS) keys to watch
        keyboard.add(min::window::key_code::F1);
        keyboard.add(min::window::key_code::F2);
        keyboard.add(min::window::key_code::F3);
        keyboard.add(min::window::key_code::KEYQ);
        keyboard.add(min::window::key_code::KEYR);
        keyboard.add(min::window::key_code::KEYW);
        keyboard.add(min::window::key_code::KEYS);
        keyboard.add(min::window::key_code::KEYA);
        keyboard.add(min::window::key_code::KEYD);
        keyboard.add(min::window::key_code::KEYE);
        keyboard.add(min::window::key_code::KEYZ);
        keyboard.add(min::window::key_code::KEYX);
        keyboard.add(min::window::key_code::KEYC);
        keyboard.add(min::window::key_code::KEY1);
        keyboard.add(min::window::key_code::KEY2);
        keyboard.add(min::window::key_code::KEY3);
        keyboard.add(min::window::key_code::KEY4);
        keyboard.add(min::window::key_code::KEY5);
        keyboard.add(min::window::key_code::KEY6);
        keyboard.add(min::window::key_code::KEY7);
        keyboard.add(min::window::key_code::KEY8);
        keyboard.add(min::window::key_code::SPACE);
        keyboard.add(min::window::key_code::KEYF);

        // Register callback function F1
        keyboard.register_keydown(min::window::key_code::F1, controls::close_window, (void *)_window);

        // Register callback function F2
        keyboard.register_keydown(min::window::key_code::F2, controls::toggle_text, (void *)_text);

        // Register callback function F3
        keyboard.register_keydown(min::window::key_code::F3, controls::toggle_pause, (void *)this);

        // Register callback function Q
        keyboard.register_keydown(min::window::key_code::KEYQ, controls::toggle_edit_mode, (void *)this);

        // Register callback function R
        keyboard.register_keydown(min::window::key_code::KEYR, controls::toggle_ai_mode, (void *)this);

        // Register callback function W
        keyboard.register_keydown(min::window::key_code::KEYW, controls::forward, (void *)this);
        keyboard.set_per_frame(min::window::key_code::KEYW, true);

        // Register callback function A
        keyboard.register_keydown(min::window::key_code::KEYA, controls::left, (void *)this);
        keyboard.set_per_frame(min::window::key_code::KEYA, true);

        // Register callback function D
        keyboard.register_keydown(min::window::key_code::KEYD, controls::right, (void *)this);
        keyboard.set_per_frame(min::window::key_code::KEYD, true);

        // Register callback function E
        keyboard.register_keydown(min::window::key_code::KEYE, controls::reset, (void *)this);

        // Register callback function S
        keyboard.register_keydown(min::window::key_code::KEYS, controls::back, (void *)this);
        keyboard.set_per_frame(min::window::key_code::KEYS, true);

        // Register callback function Z
        keyboard.register_keydown(min::window::key_code::KEYZ, controls::add_x, (void *)_world);

        // Register callback function X
        keyboard.register_keydown(min::window::key_code::KEYX, controls::add_y, (void *)_world);

        // Register callback function C
        keyboard.register_keydown(min::window::key_code::KEYC, controls::add_z, (void *)_world);

        // Register callback function KEY1 for switching texture
        keyboard.register_keydown(min::window::key_code::KEY1, controls::key1_down, (void *)this);
        keyboard.register_keyup(min::window::key_code::KEY1, controls::key1_up, (void *)this);

        // Register callback function KEY2 for switching texture
        keyboard.register_keydown(min::window::key_code::KEY2, controls::key2_down, (void *)this);
        keyboard.register_keyup(min::window::key_code::KEY2, controls::key2_up, (void *)this);

        // Register callback function KEY3 for switching texture
        keyboard.register_keydown(min::window::key_code::KEY3, controls::key3_down, (void *)this);
        keyboard.register_keyup(min::window::key_code::KEY3, controls::key3_up, (void *)this);

        // Register callback function KEY4 for switching texture
        keyboard.register_keydown(min::window::key_code::KEY4, controls::key4_down, (void *)this);
        keyboard.register_keyup(min::window::key_code::KEY4, controls::key4_up, (void *)this);

        // Register callback function KEY5 for switching texture
        keyboard.register_keydown(min::window::key_code::KEY5, controls::key5_down, (void *)this);
        keyboard.register_keyup(min::window::key_code::KEY5, controls::key5_up, (void *)this);

        // Register callback function KEY6 for switching texture
        keyboard.register_keydown(min::window::key_code::KEY6, controls::key6_down, (void *)this);
        keyboard.register_keyup(min::window::key_code::KEY6, controls::key6_up, (void *)this);

        // Register callback function KEY7 for switching texture
        keyboard.register_keydown(min::window::key_code::KEY7, controls::key7_down, (void *)this);
        keyboard.register_keyup(min::window::key_code::KEY7, controls::key7_up, (void *)this);

        // Register callback function KEY8 for switching texture
        keyboard.register_keydown(min::window::key_code::KEY8, controls::key8_down, (void *)this);
        keyboard.register_keyup(min::window::key_code::KEY8, controls::key8_up, (void *)this);

        // Register callback function SPACE
        keyboard.register_keydown(min::window::key_code::SPACE, controls::jump, (void *)_world);

        // Register callback function KEYF
        keyboard.register_keydown(min::window::key_code::KEYF, controls::switch_weapon, (void *)_state);
    }
    min::camera<float> *get_camera()
    {
        return _camera;
    }
    game::character *get_character()
    {
        return _character;
    }
    game::state *get_state()
    {
        return _state;
    }
    game::text *get_text()
    {
        return _text;
    }
    game::ui_overlay *get_ui()
    {
        return _ui;
    }
    game::world *get_world()
    {
        return _world;
    }
    min::window *get_window()
    {
        return _window;
    }
    static void close_window(void *ptr, double step)
    {
        // Cast to window pointer type and call shut down on window
        min::window *const win = reinterpret_cast<min::window *>(ptr);
        win->set_shutdown();

        // Alert that we received the call back
        std::cout << "controls: Shutdown called by user" << std::endl;
    }
    static void toggle_text(void *ptr, double step)
    {
        // Cast to text pointer type and toggle draw
        game::text *const text = reinterpret_cast<game::text *>(ptr);

        // Enable / Disable drawing text
        text->toggle_draw();
    }
    static void toggle_pause(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Cast to state pointer for pausing
        game::state *const state = control->get_state();

        // Cast to window pointer for setting window cursor
        min::window *const win = control->get_window();

        // Toggle the game pause
        const bool mode = state->toggle_game_pause();

        // set the game mode caption
        if (mode)
        {
            // Turn on cursor
            win->display_cursor(true);
            state->set_game_mode("MODE: PAUSE");
        }
        else
        {
            // Turn off cursor
            win->display_cursor(false);
            state->set_game_mode("MODE: PLAY");
        }
    }
    static void toggle_edit_mode(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the world and state pointers
        game::world *const world = control->get_world();
        game::state *const state = control->get_state();
        gun_state &gun = state->get_gun_state();

        // toggle edit mode
        const bool mode = world->toggle_edit_mode();

        // toggle fire mode if not in edit mode
        gun.set_fire_mode(!mode);

        // reset scale
        world->reset_scale();
    }
    static void toggle_ai_mode(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the world, state, and window pointers
        game::world *const world = control->get_world();
        game::state *const state = control->get_state();

        // toggle edit mode
        const bool mode = world->toggle_ai_mode();

        // set the game mode caption
        if (mode)
        {
            state->set_game_mode("MODE: AI PATH");
        }
        else
        {
            state->set_game_mode("MODE: PLAY");
        }
    }
    static void forward(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the camera and world pointers
        min::camera<float> *const camera = control->get_camera();
        game::world *const world = control->get_world();
        const min::vec3<float> &direction = camera->get_forward();
        world->character_move(direction);
    }
    static void left(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the camera and world pointers
        min::camera<float> *const camera = control->get_camera();
        game::world *const world = control->get_world();
        const min::vec3<float> &right = camera->get_frustum().get_right();
        world->character_move(right * -1.0);
    }
    static void right(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the camera and world pointers
        min::camera<float> *const camera = control->get_camera();
        game::world *const world = control->get_world();
        const min::vec3<float> &right = camera->get_frustum().get_right();
        world->character_move(right);
    }
    static void back(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the camera and world pointers
        min::camera<float> *const camera = control->get_camera();
        game::world *const world = control->get_world();
        const min::vec3<float> &direction = camera->get_forward();
        world->character_move(direction * -1.0);
    }
    static void key1_down(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the world and ui pointer
        game::world *const world = control->get_world();
        game::ui_overlay *const ui = control->get_ui();

        world->set_atlas_id(0);
        ui->set_key_down(0);
    }
    static void key2_down(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the world and ui pointer
        game::world *const world = control->get_world();
        game::ui_overlay *const ui = control->get_ui();

        world->set_atlas_id(1);
        ui->set_key_down(1);
    }
    static void key3_down(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the world and ui pointer
        game::world *const world = control->get_world();
        game::ui_overlay *const ui = control->get_ui();

        world->set_atlas_id(2);
        ui->set_key_down(2);
    }
    static void key4_down(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the world and ui pointer
        game::world *const world = control->get_world();
        game::ui_overlay *const ui = control->get_ui();

        world->set_atlas_id(3);
        ui->set_key_down(3);
    }
    static void key5_down(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the world and ui pointer
        game::world *const world = control->get_world();
        game::ui_overlay *const ui = control->get_ui();

        world->set_atlas_id(4);
        ui->set_key_down(4);
    }
    static void key6_down(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the world and ui pointer
        game::world *const world = control->get_world();
        game::ui_overlay *const ui = control->get_ui();

        world->set_atlas_id(5);
        ui->set_key_down(5);
    }
    static void key7_down(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the world and ui pointer
        game::world *const world = control->get_world();
        game::ui_overlay *const ui = control->get_ui();

        world->set_atlas_id(6);
        ui->set_key_down(6);
    }
    static void key8_down(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the world and ui pointer
        game::world *const world = control->get_world();
        game::ui_overlay *const ui = control->get_ui();

        world->set_atlas_id(7);
        ui->set_key_down(7);
    }
    static void key1_up(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the ui pointer
        game::ui_overlay *const ui = control->get_ui();
        ui->set_key_up(0);
    }
    static void key2_up(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the ui pointer
        game::ui_overlay *const ui = control->get_ui();
        ui->set_key_up(1);
    }
    static void key3_up(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the ui pointer
        game::ui_overlay *const ui = control->get_ui();
        ui->set_key_up(2);
    }
    static void key4_up(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the ui pointer
        game::ui_overlay *const ui = control->get_ui();
        ui->set_key_up(3);
    }
    static void key5_up(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the ui pointer
        game::ui_overlay *const ui = control->get_ui();
        ui->set_key_up(4);
    }
    static void key6_up(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the ui pointer
        game::ui_overlay *const ui = control->get_ui();
        ui->set_key_up(5);
    }
    static void key7_up(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the ui pointer
        game::ui_overlay *const ui = control->get_ui();
        ui->set_key_up(6);
    }
    static void key8_up(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the ui pointer
        game::ui_overlay *const ui = control->get_ui();
        ui->set_key_up(7);
    }
    static void add_x(void *ptr, double step)
    {
        // Cast to world pointer type
        game::world *const world = reinterpret_cast<game::world *>(ptr);

        // Increase x scale
        world->set_scale_x(1);
    }
    static void add_y(void *ptr, double step)
    {
        // Cast to world pointer type
        game::world *const world = reinterpret_cast<game::world *>(ptr);

        // Increase x scale
        world->set_scale_y(1);
    }
    static void add_z(void *ptr, double step)
    {
        // Cast to world pointer type
        game::world *const world = reinterpret_cast<game::world *>(ptr);

        // Increase x scale
        world->set_scale_z(1);
    }
    static void reset(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the world pointer
        game::world *const world = control->get_world();

        // Reset scale
        world->reset_scale();
    }
    static void left_click_down(void *ptr, const uint16_t x, const uint16_t y)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the camera, character, gun, and state pointers
        min::camera<float> *const cam = control->get_camera();
        game::character *const character = control->get_character();
        game::state *const state = control->get_state();
        gun_state &gun = state->get_gun_state();

        // Check if we are in beam mode
        if (gun.get_fire_mode() && gun.is_beam_mode())
        {
            // Check if we are off cooldown
            if (gun.check_cooldown())
            {
                // Record the start charge time
                gun.start_charge();

                // Activate charge animation
                character->set_animation_charge(*cam);
            }
        }
    }
    static void left_click_up(void *ptr, const uint16_t x, const uint16_t y)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the camera, world, and state pointers
        min::camera<float> *const camera = control->get_camera();
        game::world *const world = control->get_world();
        game::state *const state = control->get_state();
        game::character *const character = control->get_character();
        gun_state &gun = state->get_gun_state();
        game::ui_overlay *const ui = control->get_ui();

        // Check if we are in edit mode
        const bool mode = world->get_edit_mode();
        if (mode)
        {
            // Get the selected atlas
            const int8_t atlas = world->get_atlas_id();

            // Try to consume energy to create this resource
            const bool consumed = gun.will_consume(atlas);
            if (consumed)
            {
                // Calculate new point to add
                const min::vec3<float> point = camera->project_point(3.0);

                // Create a ray from camera to destination
                const min::ray<float, min::vec3> r(camera->get_position(), point);

                // Add block to world
                world->add_block(r);

                // Update the ui
                ui->set_energy(gun.get_energy() / 1048576.0);
            }
        }
        else if (gun.get_fire_mode())
        {
            // Abort the charge animation
            character->abort_animation_shoot();

            // Calculate point to remove from
            const min::vec3<float> point = camera->project_point(3.0);

            // Create a ray from camera to destination
            const min::ray<float, min::vec3> r(camera->get_position(), point);

            // If we are in beam mode and charged up
            if (gun.is_beam_charged())
            {
                // Remove block from world, get the removed atlas
                const int8_t atlas = world->remove_block(r);
                if (atlas >= 0.0)
                {
                    // Absorb energy to create this resource
                    gun.absorb(atlas);

                    // Update the ui energy
                    ui->set_energy(gun.get_energy() / 1048576.0);

                    // Activate shoot animation
                    character->set_animation_shoot();

                    // Start gun cooldown timer
                    gun.start_cooldown();
                }
            }
            else if (gun.is_missile_mode() && gun.check_cooldown())
            {
                // Try to consume energy to create missile
                const bool consumed = gun.will_consume(12);
                if (consumed)
                {
                    // Update the ui energy
                    ui->set_energy(gun.get_energy() / 1048576.0);

                    // Launch a missile
                    world->launch_missile(r);

                    // Activate shoot animation
                    character->set_animation_shoot();

                    // Start gun cooldown timer
                    gun.start_cooldown();
                }
            }
        }
    }
    static void right_click_down(void *ptr, const uint16_t x, const uint16_t y)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the camera, world, state, and character pointers
        min::camera<float> *const cam = control->get_camera();
        game::world *const world = control->get_world();
        game::state *const state = control->get_state();
        game::character *const character = control->get_character();
        gun_state &gun = state->get_gun_state();
        game::ui_overlay *const ui = control->get_ui();

        // Only allow grappling if in fire mode
        if (gun.get_fire_mode())
        {
            // Try to consume energy to power this resource
            const bool can_consume = gun.can_consume(10);
            if (can_consume)
            {
                // Calculate new point to add
                const min::vec3<float> proj = cam->project_point(3.0);

                // Create a ray from camera to destination
                const min::ray<float, min::vec3> r(cam->get_position(), proj);

                // Fire grappling hook
                min::vec3<float> point;
                const bool hit = world->hook_set(r, point);
                if (hit)
                {
                    // Consume energy
                    gun.consume(10);

                    // Update the ui energy
                    ui->set_energy(gun.get_energy() / 1048576.0);

                    // Activate grapple animation
                    character->set_animation_grapple(point);
                }
            }
        }
    }
    static void right_click_up(void *ptr, const uint16_t x, const uint16_t y)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the character and world pointer
        game::world *const world = control->get_world();
        game::character *const character = control->get_character();

        //Abort grappling hook
        world->hook_abort();
        character->abort_animation_grapple();
    }
    static void jump(void *ptr, double step)
    {
        // Get the world pointer
        game::world *const world = reinterpret_cast<game::world *>(ptr);
        world->character_jump(min::vec3<float>(0.0, 1.0, 0.0));
    }
    static void switch_weapon(void *ptr, double step)
    {
        // Get the gun_state pointer
        game::state *const state = reinterpret_cast<game::state *>(ptr);
        gun_state &gun = state->get_gun_state();

        // Switch weapon type
        gun.toggle_beam_mode();
    }
    static void on_resize(void *ptr, const uint16_t width, const uint16_t height)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the camera and text pointer
        min::camera<float> *const camera = control->get_camera();
        game::text *const text = control->get_text();
        game::ui_overlay *const ui = control->get_ui();

        // Get camera frustum
        auto &f = camera->get_frustum();

        // Update the aspect ratio
        f.set_aspect_ratio(width, height);
        f.make_dirty();
        camera->make_dirty();

        // Update the screen size for ui and text
        text->set_screen(width, height);
        ui->set_screen(width, height);
    }
};
}

#endif
