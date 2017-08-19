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
#include <game/ai_path.h> // problem with defines here
#include <game/controls.h>
#include <game/file.h>
#include <game/state.h>
#include <game/text.h>
#include <game/world.h>
#include <iomanip>
#include <iostream>
#include <min/bmp.h>
#include <min/camera.h>
#include <min/loop_sync.h>
#include <min/settings.h>
#include <min/utility.h>
#include <min/window.h>
#include <sstream>
#include <string>
#include <utility>

class mglcraft
{
  private:
    min::window _win;

    // Game specific classes
    game::state _state;
    game::text _text;
    game::world _world;
    game::controls _controls;

    void load_text()
    {
        // Set the screen size
        _text.set_screen(720, 480);

        // Add test text
        _text.add_text("MGLCRAFT: Official Demo", 10, 460);

        // Add cross hairs
        _text.add_text("(X)", 346, 226);

        // Add character position
        _text.add_text("X: Y: Z:", 10, 432);

        // Add character direction
        _text.add_text("X: Y: Z:", 10, 404);

        // Game mode
        _text.add_text("MODE: PLAY:", 10, 376);
    }
    std::pair<min::vec3<float>, bool> load_state()
    {
        // Create output stream for loading world
        std::vector<uint8_t> stream;

        // Load data into stream from file
        game::load_file("bin/state", stream);

        // If load failed dont try to parse stream data
        if (stream.size() != 0)
        {
            // Character position
            size_t next = 0;
            const float x = min::read_le<float>(stream, next);
            const float y = min::read_le<float>(stream, next);
            const float z = min::read_le<float>(stream, next);

            // Load character at this position
            const min::vec3<float> p(x, y, z);

            // Look direction
            const float lx = min::read_le<float>(stream, next);
            const float ly = min::read_le<float>(stream, next);
            const float lz = min::read_le<float>(stream, next);

            // Load camera settings
            const min::vec3<float> look(lx, ly, lz);
            _state.set_camera(p, look);

            // return the state
            return std::make_pair(p, true);
        }
        else
        {
            // Load character at the default position
            const min::vec3<float> p(0.0, 2.0, 0.0);

            // Load camera settings
            const min::vec3<float> look(1.0, 2.0, 0.0);
            _state.set_camera(p, look);

            // return the state
            return std::make_pair(p, false);
        }
    }
    void save_state()
    {
        // Create output stream for saving world
        std::vector<uint8_t> stream;

        // Get character position
        const min::vec3<float> &p = _world.character_position();

        // Write position into stream
        min::write_le<float>(stream, p.x());
        min::write_le<float>(stream, p.y());
        min::write_le<float>(stream, p.z());

        // Get the camera look position
        const min::vec3<float> look = _state.get_camera().project_point(3.0);

        // Write look into stream
        min::write_le<float>(stream, look.x());
        min::write_le<float>(stream, look.y());
        min::write_le<float>(stream, look.z());

        // Write data to file
        game::save_file("bin/state", stream);
    }

  public:
    // Load window shaders and program
    mglcraft()
        : _win("MGLCRAFT: LOADING", 720, 480, 3, 3),
          _text(28),
          _world(load_state(), 64, 8, 7),
          _controls(_win, _state.get_camera(), _state, _text, _world)
    {
        // Set depth and cull settings
        min::settings::initialize();

        // Load text into the text buffer
        load_text();

        // Turn off cursor
        _win.display_cursor(false);

        // Maximize window
        _win.maximize();

        // Update cursor position for tracking
        update_cursor();
    }
    ~mglcraft()
    {
        // Save game data to file
        save_state();
    }
    void clear_background() const
    {
        // blue background
        const float color[] = {0.690, 0.875f, 0.901f, 1.0f};
        glClearBufferfv(GL_COLOR, 0, color);
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    void draw(const float dt)
    {
        // Get player physics body position
        const min::vec3<float> &p = _world.character_position();

        // Get the cursor coordinates
        const auto c = _win.get_cursor();

        // Must update state properties, camera before drawing world
        _state.update(p, c, _win.get_width(), _win.get_height(), dt);

        // Reset cursor position
        update_cursor();

        // Update the world state
        _world.update(_state.get_camera(), dt);

        // Draw world geometry
        _world.draw(dt);

        // Draw things related to player state
        _state.draw(_state.get_camera(), dt);

        // Draw the text
        _text.draw();
    }
    bool is_closed() const
    {
        return _win.get_shutdown();
    }
    bool is_paused() const
    {
        return _state.get_game_pause();
    }
    void set_title(const std::string &title)
    {
        _win.set_title(title);
    }
    void update_cursor()
    {
        // Get the screen dimensions
        const uint16_t h = _win.get_height();
        const uint16_t w = _win.get_width();

        // Center cursor in middle of window
        _win.set_cursor(w / 2, h / 2);
    }
    void update_keyboard(const float dt)
    {
        // Update the keyboard
        auto &keyboard = _win.get_keyboard();
        keyboard.update(dt);
    }
    void update_text()
    {
        // If drawing text mode is on, update text
        if (_text.get_draw())
        {
            // Update player position debug text
            const min::vec3<float> &p = _world.character_position();
            std::ostringstream stream;
            stream << std::fixed << std::setprecision(4) << "X: " << p.x() << ", Y: " << p.y() << ", Z: " << p.z();
            _text.update_text(stream.str(), 2);

            // Clear and reset the stream
            stream.clear();
            stream.str(std::string());

            // Update player direction debug text
            const min::vec3<float> &f = _state.get_camera().get_forward();
            stream << "X: " << f.x() << ", Y: " << f.y() << ", Z: " << f.z();
            _text.update_text(stream.str(), 3);

            // Update the game mode text
            const std::string &mode = _state.get_game_mode();
            _text.update_text(mode, 4);

            // Upload changes
            _text.upload();
        }
    }
    void update_window()
    {
        // Update and swap buffers
        _win.update();
        _win.swap_buffers();
    }
};

void run()
{
    // Load window shaders and program, enable shader program
    mglcraft game;

    // Setup controller to run at 60 frames per second
    const int frames = 60;
    min::loop_sync sync(frames);
    double frame_time = 0.0;

    // User can close with Q or use window manager
    while (!game.is_closed())
    {
        for (int i = 0; i < frames; i++)
        {
            // Start synchronizing the loop
            sync.start();

            // Update the keyboard
            game.update_keyboard(frame_time);

            // Is the game paused?
            const bool skip = game.is_paused();
            if (!skip)
            {
                // Clear the background color
                game.clear_background();

                // Draw the model
                game.draw(frame_time);
            }

            // Update the window after draw command
            game.update_window();

            // Calculate needed delay to hit target
            frame_time = sync.sync();
        }

        // Update the debug text
        game.update_text();

        // Calculate the number of 'average' frames per second
        const double fps = sync.get_fps();

        // Update the window title with FPS count of last frame
        game.set_title("MGLCRAFT: FPS: " + std::to_string(fps));
    }
}

int main()
{
    try
    {
        run();
    }
    catch (const std::exception &ex)
    {
        std::cout << ex.what() << std::endl;
    }

    return 0;
}
