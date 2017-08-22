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
    static constexpr size_t IN = 4;
    static constexpr size_t OUT = 6;
    static constexpr float _step_size = 0.5;
    mml::nnet<float, IN, OUT> _net;

  public:
    ai_path()
    {
        // Create output stream for loading AI
        std::vector<uint8_t> input;

        std::cout << "Loading new AI path" << std::endl;

        // Load data into stream from AI file
        game::load_file("data/ai/bot", input);
        if (input.size() != 0)
        {
            // If we got data, deserialize it
            this->deserialize(input);
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
    static mml::vector<float, OUT> model(const cgrid &grid, mml::nnet<float, IN, OUT> &net, const min::vec3<float> &p, const min::vec3<float> &dir, const float travel, const float remain)
    {
        // Must be 27 in size
        const std::vector<int8_t> neighbors = grid.get_neighbors(p);
        if (neighbors.size() != 27)
        {
            throw std::runtime_error("ai_path: shit is broken");
        }

        // Calculate search direction with respect to gradient
        min::vec3<float> step = dir;

        // Create output
        mml::vector<float, OUT> output;
        output[0] = 0.0;
        output[1] = 0.0;
        output[2] = 0.0;
        output[3] = 0.0;
        output[4] = 0.0;
        output[5] = 0.0;

        // Check x collisions
        bool x_flag = true;
        {
            if (step.x() > 0.0)
            {
                output[0] = step.x();
                x_flag = x_flag && (neighbors[21] == -1);
                x_flag = x_flag && (neighbors[22] == -1);
                x_flag = x_flag && (neighbors[23] == -1);
            }
            else
            {
                output[3] = step.x() * -1.0;
                x_flag = x_flag && (neighbors[3] == -1);
                x_flag = x_flag && (neighbors[4] == -1);
                x_flag = x_flag && (neighbors[5] == -1);
            }

            // Zero out x
            if (!x_flag)
            {
                output[0] = 0.0;
                output[3] = 0.0;
            }
        }

        // Check y collisions
        bool y_flag = true;
        {
            if (step.y() > 0.0)
            {
                output[1] = step.y();
                y_flag = y_flag && (neighbors[16] == -1);
            }
            else
            {
                output[4] = step.y() * -1.0;
                y_flag = y_flag && (neighbors[10] == -1);
            }

            // Zero out y
            if (!y_flag)
            {
                output[1] = 0.0;
                output[4] = 0.0;
            }
        }

        // Check z collisions
        bool z_flag = true;
        {
            if (step.z() > 0.0)
            {
                output[2] = step.z();
                z_flag = z_flag && (neighbors[5] == -1);
                z_flag = z_flag && (neighbors[14] == -1);
                z_flag = z_flag && (neighbors[23] == -1);
            }
            else
            {
                output[5] = step.z() * -1.0;
                z_flag = z_flag && (neighbors[3] == -1);
                z_flag = z_flag && (neighbors[12] == -1);
                z_flag = z_flag && (neighbors[21] == -1);
            }

            // Zero out z
            if (!z_flag)
            {
                output[2] = 0.0;
                output[5] = 0.0;
            }
        }

        // Choose the smallest of X or Z to move around corners
        if (!x_flag && std::abs(step.x()) <= std::abs(step.z()))
        {
            if (step.x() > 0.0)
            {
                output[0] = step.x();
            }
            else
            {
                output[3] = step.x() * -1.0;
            }
        }
        else if (!z_flag && std::abs(step.z()) <= std::abs(step.x()))
        {
            if (step.z() > 0.0)
            {
                output[2] = step.z();
            }
            else
            {
                output[5] = step.z() * -1.0;
            }
        }

        // Hurdle obstacle
        {
            bool hurdle = false;
            hurdle = hurdle || (neighbors[9] != -1 && neighbors[18] == -1);
            hurdle = hurdle || (neighbors[10] != -1 && neighbors[19] == -1);
            hurdle = hurdle || (neighbors[11] != -1 && neighbors[20] == -1);
            hurdle = hurdle || (neighbors[12] != -1 && neighbors[21] == -1);
            hurdle = hurdle || (neighbors[13] != -1 && neighbors[22] == -1);
            hurdle = hurdle || (neighbors[14] != -1 && neighbors[23] == -1);
            hurdle = hurdle || (neighbors[15] != -1 && neighbors[24] == -1);
            hurdle = hurdle || (neighbors[16] != -1 && neighbors[25] == -1);

            const bool moving_x = (output[0] > 0.1) || (output[3] > 0.1);
            const bool moving_z = (output[2] > 0.1) || (output[5] > 0.1);
            if (hurdle && (!moving_x || !moving_z))
            {
                output[1] = 1.0;
            }
        }

        // Override settings if reached goal
        if (remain < 0.25)
        {
            output[0] = 0.0;
            output[1] = 0.0;
            output[2] = 0.0;
            output[3] = 0.0;
            output[4] = 0.0;
            output[5] = 0.0;
        }

        return output;
    }
    static void load(const cgrid &grid, mml::nnet<float, IN, OUT> &net, const min::vec3<float> &p, const min::vec3<float> &dir, const float travel, const float remain)
    {
        // Must be 27 in size
        const std::vector<int8_t> neighbors = grid.get_neighbors(p);
        if (neighbors.size() != 27)
        {
            throw std::runtime_error("ai_path: shit is broken");
        }

        // Create terrain input encoding
        int terrain_encode = 0;
        for (size_t i = 0; i < 27; i++)
        {
            terrain_encode |= (static_cast<int>(neighbors[i] == -1) << i);
        }

        // Create direction input encoding
        int dir_encode = 0;
        dir_encode |= static_cast<int>(dir.x() > 0.0) << 0;
        dir_encode |= static_cast<int>(dir.y() > 0.0) << 1;
        dir_encode |= static_cast<int>(dir.z() > 0.0) << 2;

        // Set inputs
        mml::vector<float, IN> in;
        in[0] = static_cast<float>(terrain_encode);
        in[1] = static_cast<float>(dir_encode);
        in[2] = travel;
        in[3] = remain;

        // Set input and calculate output
        net.set_input(in);
    }
    static min::vec3<float> unload(const mml::vector<float, OUT> &output)
    {
        // Calculate direction to move
        const float x = output[0] - output[3];
        const float y = output[1] - output[4];
        const float z = output[2] - output[5];
        return min::vec3<float>(x, y, z) * _step_size;
    }
    static min::vec3<float> solve(const cgrid &grid, mml::nnet<float, IN, OUT> &net, const min::vec3<float> &p, const min::vec3<float> &dir, const float travel, const float remain)
    {
        // Load neural net
        load(grid, net, p, dir, travel, remain);

        // Calculate output
        const mml::vector<float, OUT> out = net.calculate();

        // Unload output
        return unload(out);
    }
    static min::vec3<float> simulate(const cgrid &grid, mml::nnet<float, IN, OUT> &net, const min::vec3<float> &p, const min::vec3<float> &dir, const float travel, const float remain)
    {
        return ai_path::unload(ai_path::model(grid, net, p, dir, travel, remain));
    }
    min::vec3<float> simulate(const cgrid &grid, const min::vec3<float> &p, const min::vec3<float> &dir, const float travel, const float remain)
    {
        return ai_path::unload(ai_path::model(grid, _net, p, dir, travel, remain));
    }
    min::vec3<float> solve(const cgrid &grid, const min::vec3<float> &p, const min::vec3<float> &dir, const float travel, const float remain)
    {
        return ai_path::solve(grid, _net, p, dir, travel, remain);
    }
};
}

#endif
