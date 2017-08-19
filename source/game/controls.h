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
#ifndef __CONTROLS__
#define __CONTROLS__

#include <game/state.h>
#include <game/text.h>
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
    game::state *_state;
    game::text *_text;
    game::world *_world;

  public:
    controls(min::window &window, min::camera<float> &camera, game::state &state, game::text &text, game::world &world)
        : _window(&window), _camera(&camera), _state(&state), _text(&text), _world(&world)
    {
        // Check that pointers are valid
        if (!_window || !_camera || !_state || !_text || !_world)
        {
            throw std::runtime_error("control: Invalid control pointers");
        }

        // Get access to the keyboard
        auto &keyboard = _window->get_keyboard();

        // Register click callback function for placing path
        _window->register_data((void *)this);
        _window->register_lclick(controls::left_click);
        _window->register_rclick(controls::right_click);
        _window->register_update(controls::on_resize);

        // Add FPS(WADS) keys to watch
        keyboard.add(min::window::key_code::F1);
        keyboard.add(min::window::key_code::F2);
        keyboard.add(min::window::key_code::F3);
        keyboard.add(min::window::key_code::KEYQ);
        keyboard.add(min::window::key_code::KEYR);
        keyboard.add(min::window::key_code::KEYT);
        keyboard.add(min::window::key_code::KEYY);
        keyboard.add(min::window::key_code::KEYW);
        keyboard.add(min::window::key_code::KEYS);
        keyboard.add(min::window::key_code::KEYA);
        keyboard.add(min::window::key_code::KEYD);
        keyboard.add(min::window::key_code::KEYE);
        keyboard.add(min::window::key_code::SPACE);
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

        // Register callback function T
        keyboard.register_keydown(min::window::key_code::KEYT, controls::toggle_train_mode, (void *)this);

        // Register callback function Y
        keyboard.register_keydown(min::window::key_code::KEYY, controls::set_train_destination, (void *)this);

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

        // Register callback function SPACE
        keyboard.register_keyup(min::window::key_code::SPACE, controls::jump, (void *)this);

        // Register callback function Z
        keyboard.register_keydown(min::window::key_code::KEYZ, controls::add_x, (void *)_world);

        // Register callback function X
        keyboard.register_keydown(min::window::key_code::KEYX, controls::add_y, (void *)_world);

        // Register callback function C
        keyboard.register_keydown(min::window::key_code::KEYC, controls::add_z, (void *)_world);

        // Register callback function KEY1 for switching texture to 'grass'
        keyboard.register_keydown(min::window::key_code::KEY1, controls::switch_grass, (void *)_world);

        // Register callback function KEY2 for switching texture to 'stone'
        keyboard.register_keydown(min::window::key_code::KEY2, controls::switch_stone, (void *)_world);

        // Register callback function KEY2 for switching texture to 'sand'
        keyboard.register_keydown(min::window::key_code::KEY3, controls::switch_sand, (void *)_world);

        // Register callback function KEY2 for switching texture to 'wood'
        keyboard.register_keydown(min::window::key_code::KEY4, controls::switch_wood, (void *)_world);

        // Register callback function KEY1 for switching texture to 'dirt'
        keyboard.register_keydown(min::window::key_code::KEY5, controls::switch_dirt, (void *)_world);

        // Register callback function KEY2 for switching texture to 'lava'
        keyboard.register_keydown(min::window::key_code::KEY6, controls::switch_lava, (void *)_world);

        // Register callback function KEY2 for switching texture to 'water'
        keyboard.register_keydown(min::window::key_code::KEY7, controls::switch_water, (void *)_world);

        // Register callback function KEY2 for switching texture to 'sulphur'
        keyboard.register_keydown(min::window::key_code::KEY8, controls::switch_sulphur, (void *)_world);
    }
    min::camera<float> *get_camera()
    {
        return _camera;
    }
    game::state *get_state()
    {
        return _state;
    }
    game::text *get_text()
    {
        return _text;
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

        // toggle edit mode
        const bool mode = world->toggle_edit_mode();

        // toggle fire mode if not in edit mode
        state->set_fire_mode(!mode);

        // reset scale
        world->reset_scale();
    }
    static void toggle_ai_mode(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the world and state pointers
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
    static void toggle_train_mode(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Cast to window pointer for setting window cursor
        min::window *const win = control->get_window();

        // Get the world and state pointers
        game::world *const world = control->get_world();
        game::state *const state = control->get_state();

        // Pause the game
        state->set_game_pause(true);
        state->pause_lock(true);

        // Turn off cursor if paused
        win->display_cursor(false);

        // Create background task
        const auto task = [world, state]() {
            // train AI for 100 iterations in background thread
            world->train(100);

            // Unpause the game
            state->pause_lock(false);
            state->set_game_pause(false);
        };

        // Launch the task in the background
        std::thread t(task);

        // Detach the thread
        t.detach();
    }
    static void set_train_destination(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the world pointer
        game::world *const world = control->get_world();

        // toggle edit mode
        world->set_train_destination();
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
    static void jump(void *ptr, double step)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the world pointer
        game::world *const world = control->get_world();
        world->character_jump(min::vec3<float>(0.0, 1.0, 0.0));
    }
    static void switch_grass(void *ptr, double step)
    {
        // Cast to world pointer type and set atlas id to 'grass'
        game::world *const world = reinterpret_cast<game::world *>(ptr);
        world->set_atlas_id(5);
    }
    static void switch_stone(void *ptr, double step)
    {
        // Cast to world pointer type and set atlas id to 'stone'
        game::world *const world = reinterpret_cast<game::world *>(ptr);
        world->set_atlas_id(0);
    }
    static void switch_sand(void *ptr, double step)
    {
        // Cast to world pointer type and set atlas id to 'sand'
        game::world *const world = reinterpret_cast<game::world *>(ptr);
        world->set_atlas_id(1);
    }
    static void switch_wood(void *ptr, double step)
    {
        // Cast to world pointer type and set atlas id to 'wood'
        game::world *const world = reinterpret_cast<game::world *>(ptr);
        world->set_atlas_id(3);
    }
    static void switch_dirt(void *ptr, double step)
    {
        // Cast to world pointer type and set atlas id to 'dirt'
        game::world *const world = reinterpret_cast<game::world *>(ptr);
        world->set_atlas_id(4);
    }
    static void switch_lava(void *ptr, double step)
    {
        // Cast to world pointer type and set atlas id to 'lava'
        game::world *const world = reinterpret_cast<game::world *>(ptr);
        world->set_atlas_id(2);
    }
    static void switch_water(void *ptr, double step)
    {
        // Cast to world pointer type and set atlas id to 'water'
        game::world *const world = reinterpret_cast<game::world *>(ptr);
        world->set_atlas_id(6);
    }
    static void switch_sulphur(void *ptr, double step)
    {
        // Cast to world pointer type and set atlas id to 'sulphur'
        game::world *const world = reinterpret_cast<game::world *>(ptr);
        world->set_atlas_id(7);
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
    static void left_click(void *ptr, const uint16_t x, const uint16_t y)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the camera world, and state pointers
        min::camera<float> *const camera = control->get_camera();
        game::world *const world = control->get_world();
        game::state *const state = control->get_state();

        // Check if we are in edit mode
        const bool mode = world->get_edit_mode();
        if (mode)
        {
            // Calculate new point to add
            const min::vec3<float> point = camera->project_point(3.0);

            // Create a ray from camera to destination
            const min::ray<float, min::vec3> r(camera->get_position(), point);

            // Add block to world
            world->add_block(r);
        }
        else
        {
            // Calculate point to remove from
            const min::vec3<float> point = camera->project_point(3.0);

            // Create a ray from camera to destination
            const min::ray<float, min::vec3> r(camera->get_position(), point);

            // Remove block from world
            world->remove_block(r);

            // Activate shoot animation
            state->animate_shoot_player();
        }
    }
    static void right_click(void *ptr, const uint16_t x, const uint16_t y)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the camera world, and state pointers
        min::camera<float> *const camera = control->get_camera();
        game::world *const world = control->get_world();
        game::state *const state = control->get_state();

        // Only allow grappling if in fire mode
        if (state->get_fire_mode())
        {
            // Calculate new point to add
            const min::vec3<float> point = camera->project_point(3.0);

            // Create a ray from camera to destination
            const min::ray<float, min::vec3> r(camera->get_position(), point);

            // Fire grappling hook
            world->grappling(r);

            // Activate shoot animation
            state->animate_shoot_player();
        }
    }
    static void on_resize(void *ptr, const uint16_t width, const uint16_t height)
    {
        // Cast to control pointer
        controls *const control = reinterpret_cast<controls *>(ptr);

        // Get the camera and text pointer
        min::camera<float> *const camera = control->get_camera();
        game::text *const text = control->get_text();

        // Get camera frustum
        auto &f = camera->get_frustum();

        // Update the aspect ratio
        f.set_aspect_ratio(width, height);
        f.make_dirty();
        camera->make_dirty();

        // Update the text screen size
        text->set_screen(width, height);
    }
};
}

#endif
