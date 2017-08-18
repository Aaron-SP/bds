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
#ifndef __AI_TRAINER__
#define __AI_TRAINER__

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
    const min::vec3<float> start(0.0, 2.0, 0.0);
    const min::vec3<float> dest(13.0, -2.0, -36.0);

    // train the ai
    for (size_t i = 0; i < 100000; i++)
    {
        trainer.train(grid, start, dest);
    }

    // Create output stream for saving bot
    std::vector<uint8_t> stream;
    trainer.serialize(stream);

    // Write data to file
    game::save_file("bin/bot", stream);

    // return status
    return out;
}

#endif
