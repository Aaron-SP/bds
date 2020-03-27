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

#ifndef MGL_INLINE
#include <game/pch.hpp>
#endif

#include <game/game.h>

Uint64 last_time = 0;
Uint64 now_time = SDL_GetPerformanceCounter();
float frame_time = 0.0f;
size_t frame = 0;
size_t debounce = 0;

void main_tick_title(void *data)
{
    // Cast data point
    bds &game = *static_cast<bds *>(data);

    // Break out if not running
    const bool running = game.is_show_title();
    if (!running)
    {
        // Register the game callbacks
        if (debounce == 0)
        {
            debounce = 1;
            game.title_screen_disable();
        }

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

        // Run the per second update
        if (frame++ == 60)
        {
            // Reset frame count
            frame = 0;

            // Perform second update
            game.update_second();

            // Check for fatal errors
            game.throw_fatal_error();
        }

        // Calculate the number of 'average' frames per second
        const double fps = 1.0 / frame_time;

        // Update the debug text
        game.update_fps(fps, 0.0);
    }
    else
    {
        // Clear the background color
        game.clear_background();

        // Update the title screen
        game.update_title(frame_time);

        // Draw the title screen
        game.draw_title();

        // Update the window after draw command
        game.update_window();

        // Run the per second update
        if (frame++ == 60)
        {
            // Reset frame count
            frame = 0;

            // Make console message blink
            game.blink_console_message();

            // Check for OpenGL errors, this won't break out if error found
            // We want to flush out any non-fatal error before starting
            if (game.check_gl_error())
            {
                std::cout << "OpenGL errors detected in show_title" << std::endl;
            }
            else if (game.check_al_error())
            {
                std::cout << "OpenAL errors detected in show_game" << std::endl;
            }
        }
    }

    // Update the particles
    last_time = now_time;
    now_time = SDL_GetPerformanceCounter();
    frame_time = (now_time - last_time) / static_cast<float>(SDL_GetPerformanceFrequency());
}

void main_loop_title(bds &game)
{
#if __EMSCRIPTEN__
    try
    {
        emscripten_set_main_loop_arg(main_tick_title, &game, 0, true);
    }
    catch (const std::exception &ex)
    {
        std::cout << "Exiting title loop!" << std::endl;
    }
#else
    throw std::runtime_error("game.cpp: Not implemented!");
#endif
}

void show_title(bds &game)
{
    // Play the background music
    game.play_music();

    // Run the title main loop
    main_loop_title(game);
}

void run(const game::options &opt)
{
    game::memory_map::memory = new min::mem_chunk(DATA_FILE);

    // Load window shaders and program, enable shader program
    bds *game = new bds(opt);

    // Catch any errors and print to window
    try
    {
        // Maximize the window
        if (opt.resize())
        {
            game->maximize();
        }

        // Loop until we exit game
        while (!game->is_closed())
        {
            // Show the title screen
            show_title(*game);

            // If we are not closing the game
            if (!game->is_closed())
            {
                // Switch to title
                game->title_screen_enable();
            }
        }
    }
    catch (const std::exception &ex)
    {
        game->error_message(ex.what());
    }
}

bool parse_uint(char *str, size_t &out)
{
    // Try to parse string input
    try
    {
        // Get next value in string buffer
        out = std::stoi(str);

        // Return a successful parse
        return true;
    }
    catch (const std::exception &ex)
    {
        // Print parsing exception message
        std::cout << "bds: couldn't parse input: '"
                  << str << "', expected integral type" << std::endl;
    }

    // Bad parse
    return false;
}

int main(int argc, char *argv[])
{
    try
    {
        // Default parameters
        game::options opt;
        size_t parse;

        // Try to parse commandline args
        for (int i = 1; i < argc; i++)
        {
            // Get input flag
            const std::string input(argv[i]);

            // Match string
            if (input.compare("--qwerty") == 0)
            {
                opt.set_map(game::key_map_type::QWERTY);
            }
            else if (input.compare("--dvorak") == 0)
            {
                opt.set_map(game::key_map_type::DVORAK);
            }
            else if (input.compare("--no-persist") == 0)
            {
                opt.set_no_persist();
            }
            else if (i < (argc - 1))
            {
                if (input.compare("-fps") == 0)
                {
                    // Parse uint
                    if (parse_uint(argv[++i], parse))
                    {
                        opt.set_frames(parse);
                    }
                }
                else if (input.compare("-chunk") == 0)
                {
                    // Parse uint
                    if (parse_uint(argv[++i], parse))
                    {
                        opt.set_chunk(parse);
                    }
                }
                else if (input.compare("-grid") == 0)
                {
                    // Parse uint
                    if (parse_uint(argv[++i], parse))
                    {
                        opt.set_grid(parse);
                    }
                }
                else if (input.compare("-view") == 0)
                {
                    // Parse uint
                    if (parse_uint(argv[++i], parse))
                    {
                        opt.set_view(parse);
                    }
                }
                else if (input.compare("-width") == 0)
                {
                    // Parse uint
                    if (parse_uint(argv[++i], parse))
                    {
                        opt.set_width(static_cast<uint_fast16_t>(parse));
                        opt.set_resize(false);
                    }
                }
                else if (input.compare("-height") == 0)
                {
                    // Parse uint
                    if (parse_uint(argv[++i], parse))
                    {
                        opt.set_height(static_cast<uint_fast16_t>(parse));
                        opt.set_resize(false);
                    }
                }
                else
                {
                    std::cout << "bds: unknown flag '"
                              << input << "'" << std::endl;
                }
            }
            else
            {
                std::cout << "bds: not enough arguments passed for '"
                          << input << "'" << std::endl;
            }
        }

        // Exit if error in options
        if (opt.check_error())
        {
            return 0;
        }

        // Run the game
        run(opt);
    }
    catch (const std::exception &ex)
    {
        std::cout << "Beyond Dying Skies failed to launch!" << std::endl;
        std::cout << ex.what() << std::endl;
    }

    return 0;
}
