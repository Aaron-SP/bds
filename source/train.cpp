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

#include <game/ai_opt.h>
#include <game/file.h>
#include <min/window.h>
#include <stdexcept>

void run(const bool debug = false)
{
    min::window win("MGLCRAFT: HEADLESS AI TRAINER", 720, 480, 3, 3);

    // Load world at point
    const min::vec3<float> p(-0.5, 30.5, 1.5);
    game::world world(std::make_pair(p, false), 64, 8, 7);
    game::goal_seek gs(world);

    // Get the optimizer for checking scores
    game::ai_opt opt(world, p, gs.get_goal());

    // Load the AI script
    std::vector<uint8_t> input;

    // Load data into stream from AI file
    game::load_file("data/ai/bot", input);
    if (input.size() != 0)
    {
        // load the data into the optimizer of previous run
        opt.deserialize(input);
    }

    // Debug the AI file
    if (debug)
    {
        opt.top_path().debug();
        return;
    }

    for (size_t i = 0; i < 1000; i++)
    {
        for (size_t j = 0; j < 6550; j++)
        {
            // Print iteration number
            std::cout << "iter i: " << i << " j: " << j << std::endl;

            // Print out the total goals
            std::cout << "Goals: " << gs.get_score() << std::endl;

            // Update world
            world.update_world_physics(0.01667);

            // Evolve the simulation
            opt.evolve(world);

            // Update goals
            opt.update_goal(world, gs);

            // Print debug information
            opt.debug();
        }

        // Create output stream for saving bot
        std::vector<uint8_t> output;
        opt.serialize(output);

        // Write AI data to file
        game::save_file("data/ai/bot", output);
    }
}

int main(int argc, char *argv[])
{
    try
    {
        // Parse input flags
        bool debug = false;
        if (argc == 2)
        {
            // Should we debug the AI file
            std::string input(argv[1]);

            if (input.compare("--debug") == 0)
            {
                debug = true;
            }
            else
            {
                std::cout << "Unknown flag '" << input << "'" << std::endl;
                return 0;
            }
        }

        // Run the main thread
        run(debug);
    }
    catch (const std::exception &ex)
    {
        std::cout << ex.what() << std::endl;
    }

    return 0;
}
