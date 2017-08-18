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
#ifndef __AI_PATH__
#define __AI_PATH__

#include <algorithm>
#include <game/cgrid.h>
#include <game/file.h>
#include <min/vec3.h>
#include <mml/nnet.h>
#include <mml/vec.h>

namespace game
{

class ai_path
{
  private:
    mml::nnet<float, 31, 3> _net;

  public:
    ai_path()
    {
        // Create output stream for loading AI
        std::vector<uint8_t> input;

        // Load data into stream from AI file
        game::load_file("bin/bot", input);
        if (input.size() != 0)
        {
            // If we got data, deserialize it
            this->deserialize(input);
        }
        else
        {
            throw std::runtime_error("ai_path: could not load AI from bin/bot file");
        }
    }
    void deserialize(std::vector<uint8_t> &stream)
    {
        // read data from stream
        size_t next = 0;
        const std::vector<float> data = min::read_le_vector<float>(stream, next);

        // Must definalize the net to deserialize it
        _net.reset();
        _net.deserialize(data);
    }
    static min::vec3<float> move(const cgrid &grid, mml::nnet<float, 31, 3> &net, const min::vec3<float> &start, const min::vec3<float> &dest)
    {
        // Must be 27 in size
        const std::vector<int8_t> neighbors = grid.get_neighbors(start);
        if (neighbors.size() != 27)
        {
            throw std::runtime_error("ai_path: shit is broken");
        }

        // Create input vector
        mml::vector<float, 31> in;
        for (size_t i = 0; i < 27; i++)
        {
            // 1 if empty, 0 if filled
            const int empty = static_cast<bool>(neighbors[i] == -1);
            in[i] = static_cast<float>(empty);
        }
        in[27] = static_cast<float>(dest.x());
        in[28] = static_cast<float>(dest.y());
        in[29] = static_cast<float>(dest.z());
        in[30] = static_cast<float>((dest - start).magnitude());

        // Set input and calculate output
        net.set_input(in);
        const mml::vector<float, 3> out = net.calculate();
        return min::vec3<float>(out[0], out[1], out[2]);
    }
    min::vec3<float> step(const cgrid &grid, const min::vec3<float> &start, const min::vec3<float> &dest)
    {
        return ai_path::move(grid, _net, start, dest);
    }
};
}

#endif
