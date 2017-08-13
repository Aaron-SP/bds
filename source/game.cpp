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
#include <controls.h>
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
#include <text.h>
#include <world.h>

class mglcraft
{
  private:
    min::window _win;

    // Camera and uniform data
    min::camera<float> _cam;

    // Game specific classes
    game::world _world;
    game::controls _controls;
    game::text _text;

    void load_camera()
    {
        // Move and camera to -X and look at origin
        const min::vec3<float> pos = min::vec3<float>(0.0, 2.0, 0.0);
        const min::vec3<float> look = min::vec3<float>(1.0, 2.0, 0.0);

        // Test perspective projection
        // Create camera, set location and look at
        _cam.set_position(pos);
        _cam.set_look_at(look);
        auto &f = _cam.get_frustum();
        f.set_far(5000.0);
        f.set_fov(90.0);
        _cam.set_perspective();
    }
    void load_text()
    {
        // Set the screen size
        _text.set_screen(720, 480);

        // Add test text
        _text.add_text("MGLCRAFT:Official Demo", 10, 460);

        // Add cross hairs
        _text.add_text("(X)", 346, 226);

        // Add character position
        _text.add_text("X:Y:Z:", 10, 432);

        // Add character direction
        _text.add_text("X:Y:Z:", 10, 404);
    }

  public:
    // Load window shaders and program
    mglcraft()
        : _win("MGLCRAFT: FPS: ", 720, 480, 3, 3),
          _world(64, 8, 7),
          _controls(_win, _cam, _text, _world),
          _text(28)
    {
        // Set depth and cull settings
        min::settings::initialize();

        // Load text into the text buffer
        load_text();

        // Turn off cursor
        _win.display_cursor(false);

        // Maximize window
        _win.maximize();

        // Load the camera and fill uniform buffers with light and model matrix
        load_camera();
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
        // Draw world geometry
        _world.draw(_cam, dt);

        // Draw the text
        _text.draw();
    }
    bool is_closed() const
    {
        return _win.get_shutdown();
    }
    void set_title(const std::string &title)
    {
        _win.set_title(title);
    }
    void update_camera(const double step)
    {
        // Get the cursor coordinates
        const auto c = _win.get_cursor();

        // Update the keyboard
        auto &keyboard = _win.get_keyboard();
        keyboard.update(step);

        // Get the offset from screen center
        const float sensitivity = 0.10;
        float x = sensitivity * (c.first - (_win.get_width() / 2));
        float y = sensitivity * (c.second - (_win.get_height() / 2));

        // If the mouse coordinates moved at all
        if (std::abs(x) > 1E-3 || std::abs(y) > 1E-3)
        {
            // Get the camera forward vector
            const min::vec3<float> &forward = _cam.get_forward();

            // Check if we have looked too far on the global y axis
            const float dy = forward.dot(min::vec3<float>::up());
            if (dy > 0.975 && y < 0.0)
            {
                y = 0.0;
            }
            else if (dy < -0.975 && y > 0.0)
            {
                y = 0.0;
            }

            // Adjust the camera by the offset from screen center
            _cam.move_look_at(x, y);

            // Move the cursor back
            update_cursor();
        }
    }
    void update_cursor()
    {
        // Get the screen dimensions
        const uint16_t h = _win.get_height();
        const uint16_t w = _win.get_width();

        // Center cursor in middle of window
        _win.set_cursor(w / 2, h / 2);
    }
    void update_text()
    {
        // Update player position debug text
        const min::vec3<float> &p = _world.character_position();
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(4) << "X:" << p.x() << ",Y:" << p.y() << ",Z:" << p.z();
        _text.update_text(stream.str(), 2);

        // Clear and reset the stream
        stream.clear();
        stream.str(std::string());

        // Update player direction debug text
        const min::vec3<float> &f = _cam.get_forward();
        stream << "X:" << f.x() << ",Y:" << f.y() << ",Z:" << f.z();
        _text.update_text(stream.str(), 3);

        // Upload changes
        _text.upload();
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

            // Clear the background color
            game.clear_background();

            // Update the camera movement
            game.update_camera(frame_time);

            // Draw the model
            game.draw(frame_time);

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
