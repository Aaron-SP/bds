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

#include <game/game.h>

inline void show_title(bds &game, min::loop_sync &sync, const size_t frames)
{
    double frame_time = 0.0;

    // Play the background music
    game.play_music();

    // Draw the title screen before playing
    while (game.is_show_title())
    {
        for (size_t i = 0; i < frames; i++)
        {
            // Start synchronizing the loop
            sync.start();

            // Clear the background color
            game.clear_background();

            // Draw the model
            game.draw_title(frame_time);

            // Update the window after draw command
            game.update_window();

            // Calculate needed delay to hit target
            frame_time = sync.sync();
        }

        // Make console message blink
        game.blink_console_message();

        // Check for OpenGL errors
        if (game.check_gl_error())
        {
            std::cout << "OpenGL errors detected in show_title, quitting" << std::endl;
            break;
        }
    }
}

inline void show_game(bds &game, min::loop_sync &sync, const size_t frames)
{
    // Register the game callbacks
    game.disable_title_screen();

    // Calculate number of physics steps per frame
    double frame_time = 0.0;

    // User can close with Q or use window manager
    while (!game.is_closed())
    {
        for (size_t i = 0; i < frames; i++)
        {
            // Start synchronizing the loop
            sync.start();

            // Update the keyboard
            game.update_keyboard(frame_time);

            // Clear the background color
            game.clear_background();

            // Update the scene
            game.update(frame_time);

            // Draw the scene
            game.draw();

            // Update the window after draw command
            game.update_window();

            // Calculate needed delay to hit target
            frame_time = sync.sync();
        }

        // Calculate the number of 'average' frames per second
        const double fps = sync.get_fps();

        // Calculate the percentage of frame spent idle
        const double idle = sync.idle();

        // Update the debug text
        game.update_text(fps, idle);

        // Check for errors
        if (game.check_gl_error())
        {
            std::cout << "OpenGL errors detected in show_game, quitting" << std::endl;
            break;
        }
        else if (game.check_al_error())
        {
            std::cout << "OpenAL errors detected in show_game, quitting" << std::endl;
            break;
        }
    }
}

void run(const size_t frames, const size_t chunk, const size_t grid,
         const size_t view, const size_t width, const size_t height, const bool resize)
{
    // Load window shaders and program, enable shader program
    bds game(chunk, grid, view, width, height);

    // Setup controller to run at 60 frames per second
    min::loop_sync sync(frames, 0.25, 0.25, 0.25);

    // Maximize the window
    if (resize)
    {
        game.maximize();
    }

    // Show the title screen
    show_title(game, sync, 15);

    // Run the game after the title screen
    show_game(game, sync, frames);
}

void parse_uint(char *str, size_t &out)
{
    // Try to parse string input
    try
    {
        // Get next value in string buffer
        out = std::stoi(str);
    }
    catch (const std::exception &ex)
    {
        // Print parsing exception message
        std::cout << "bds: couldn't parse input: '"
                  << str << "', expected integral type" << std::endl;
    }
}

int main(int argc, char *argv[])
{
    try
    {
        // Default frame count
        size_t frames = 60;
        size_t chunk = 16;
        size_t grid = 64;
        size_t view = 3;
        size_t width = 720;
        size_t height = 480;
        bool resize = true;

        // Try to parse commandline args
        for (int i = 2; i < argc; i += 2)
        {
            // Get input flag
            std::string input(argv[i - 1]);

            // Check for fps flag
            if (input.compare("-fps") == 0)
            {
                // Parse uint
                parse_uint(argv[i], frames);
            }
            else if (input.compare("-chunk") == 0)
            {
                // Parse uint
                parse_uint(argv[i], chunk);
            }
            else if (input.compare("-grid") == 0)
            {
                // Parse uint
                parse_uint(argv[i], grid);

                // Warn user that grid sizes not compatible
                std::cout << "Resizing the grid: deleting old save caches" << std::endl;

                // Erase previous state files
                game::erase_file("bin/state");
                game::erase_file("bin/world.bmesh");
            }
            else if (input.compare("-view") == 0)
            {
                // Parse uint
                parse_uint(argv[i], view);
            }
            else if (input.compare("-width") == 0)
            {
                // Parse uint
                parse_uint(argv[i], width);
                resize = false;
            }
            else if (input.compare("-height") == 0)
            {
                // Parse uint
                parse_uint(argv[i], height);
                resize = false;
            }
            else
            {
                std::cout << "bds: unknown flag '"
                          << input << "'\"" << std::endl;
            }
        }

        // Do some sanity checks on values
        if (grid < 4)
        {
            std::cout << "bds: '-grid' must be atleast 4" << std::endl;
            return 0;
        }
        else if (chunk < 2)
        {
            std::cout << "bds: '-chunk' must be atleast 2" << std::endl;
            return 0;
        }
        else if (view < 3)
        {
            std::cout << "bds: '-view' must be atleast 3" << std::endl;
            return 0;
        }

        // Run the game
        run(frames, chunk, grid, view, width, height, resize);
    }
    catch (const std::exception &ex)
    {
        std::cout << ex.what() << std::endl;
    }

    return 0;
}
