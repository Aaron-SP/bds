#include <game/game.h>

inline void show_title(fractex &game, min::loop_sync &sync, const size_t frames)
{
    double frame_time = 0.0;

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
    }
}

inline void show_game(fractex &game, min::loop_sync &sync, const size_t frames)
{
    double frame_time = 0.0;

    // Register the game callbacks
    game.disable_title_screen();

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

            // Draw the model
            game.draw(frame_time);

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
    }
}

void run(const size_t frames, const size_t chunk, const size_t view)
{
    // Load window shaders and program, enable shader program
    fractex game(chunk, view);

    // Setup controller to run at 60 frames per second
    min::loop_sync sync(frames, 0.25, 0.25, 0.25);

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
        std::cout << "fractex: couldn't parse input: '"
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
        size_t view = 3;

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
            else if (input.compare("-view") == 0)
            {
                // Parse uint
                parse_uint(argv[i], view);
            }
            else
            {
                std::cout << "fractex: unknown flag '"
                          << input << "'\"" << std::endl;
            }
        }

        // Run the game
        run(frames, chunk, view);
    }
    catch (const std::exception &ex)
    {
        std::cout << ex.what() << std::endl;
    }

    return 0;
}
