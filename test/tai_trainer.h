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
#ifndef __TEST_AI_TRAINER__
#define __TEST_AI_TRAINER__

#include <game/ai_trainer.h>
#include <game/cgrid.h>
#include <game/file.h>
#include <stdexcept>
#include <test.h>

bool test_ai_trainer()
{
    bool out = true;

    // Load the graph mesh with 128 pixel tile size
    game::cgrid grid(64, 8, 7);
    game::ai_trainer trainer;

    // Create start and destination points
    const min::vec3<float> start(1.0, -33.0, 1.0);
    const min::vec3<float> dest(0.0, 2.0, 0.0);

    // Create output stream for loading AI
    std::vector<uint8_t> input;

    // Load data into stream from AI file
    game::load_file("bin/bot", input);
    if (input.size() != 0)
    {
        // load the data into the trainer of previous run
        trainer.deserialize(input);
    }

    // train the ai
    for (size_t i = 0; i < 1000; i++)
    {
        trainer.train(grid, start, dest);
        std::cout << "iteration " << i << std::endl;
    }

    // Create output stream for saving bot
    std::vector<uint8_t> output;
    trainer.serialize(output);

    // Write data to file
    game::save_file("bin/bot", output);

    // return status
    return out;
}

#endif
