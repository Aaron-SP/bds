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
#ifndef __GAI_TRAINER__
#define __GAI_TRAINER__

#include <algorithm>
#include <game/ai_path.h>
#include <game/cgrid.h>
#include <min/vec3.h>
#include <mml/nnet.h>
#include <mml/vec.h>
#include <utility>

namespace game
{

class ai_trainer
{
  private:
    static constexpr unsigned _pool_size = 100;
    static constexpr unsigned _breed_stock = 13;
    static constexpr unsigned _mutation_rate = 50;
    static constexpr unsigned _total_moves = 20;
    mml::nnet<float, 28, 3> _nets[_pool_size];
    float _scores[_pool_size];
    mml::net_rng<float> _rng;
    mml::nnet<float, 28, 3> _top_net;
    float _top;
    float _average_fitness;

    static float fitness_score(const cgrid &grid, mml::nnet<float, 28, 3> &net, const min::vec3<float> &start)
    {
        min::vec3<float> current = start;
        bool stop = false;
        float score = 0.0;
        size_t moves = 1;
        float distance = 0;

        // while not dead
        while (!stop)
        {
            // Failed flag
            bool failed = false;

            // Get new location
            const min::vec3<float> step = ai_path::solve(grid, net, current, distance);

            // Calculate distance to destination
            current += step;
            const min::vec3<float> d = current - start;
            distance = d.magnitude();

            // Check if we failed and return start position
            const int8_t atlas = grid.grid_value(current);
            if (atlas != -1)
            {
                failed = true;
            }

            // Increment moves and break out if we used up all moves
            moves++;
            if (moves > _total_moves)
            {
                stop = true;
            }

            // Detect a zero move
            if (distance > 0.0)
            {
                // Reward / discipline moves
                if (failed)
                {
                    score -= 1.0;
                }
                else
                {
                    score += 1.0;
                }
            }
            else
            {
                score -= 1.0;
            }
        }

        // return fitness score
        return score;
    }

  public:
    ai_trainer() : _rng(std::uniform_real_distribution<float>(-1E2, 1E2),
                        std::uniform_real_distribution<float>(-1E2, 1E2),
                        std::uniform_int_distribution<int>(0, _pool_size - 1)),
                   _average_fitness(0.0)
    {
        // Initialize all the nets
        for (size_t i = 0; i < _pool_size; i++)
        {
            // Create a fresh net
            mml::nnet<float, 28, 3> &net = _nets[i];
            net.add_layer(16);
            net.add_layer(16);
            net.finalize();
            net.randomize(_rng);
        }
    }
    void deserialize(std::vector<uint8_t> &stream)
    {
        // read data from stream
        size_t next = 0;
        const std::vector<float> data = min::read_le_vector<float>(stream, next);

        // Initialize top net
        _top_net.reset();
        _top_net.deserialize(data);

        // Initialize all the with previous top net
        for (size_t i = 0; i < _pool_size; i++)
        {
            // Load previous net and mutate it
            mml::nnet<float, 28, 3> &net = _nets[i];

            // Must definalize the net to deserialize it
            net.reset();
            net.deserialize(data);

            // Mutate the net for added variation
            net.mutate(_rng);
        }
    }
    void serialize(std::vector<uint8_t> &stream)
    {
        // Get the net float data
        const std::vector<float> data = _top_net.serialize();

        // Write data into stream
        min::write_le_vector<float>(stream, data);
    }
    void train(const cgrid &grid, const min::vec3<float> &start)
    {
        // Assert that we do not overflow
        static_assert(((_breed_stock * _breed_stock + _breed_stock) / 2) <= _pool_size, "Invalid breed stock dimensions");

        // Calculate fitness scores
        for (size_t i = 0; i < _pool_size; i++)
        {
            _scores[i] = fitness_score(grid, _nets[i], start);
        }

        // Create index vector to sort 0 to N
        size_t index[_pool_size];
        std::iota(index, index + _pool_size, 0);
        std::sort(index, index + _pool_size, [this](const size_t a, const size_t b) {
            return this->_scores[a] > this->_scores[b];
        });

        // Calculate average fitness
        _average_fitness = 0.0;
        for (size_t i = 0; i < _pool_size; i++)
        {
            _average_fitness += _scores[i];
        }
        _average_fitness /= _pool_size;

        // Record the best score of all time
        if (_scores[index[0]] > _top)
        {
            _top = _scores[index[0]];
            _top_net = _nets[index[0]];
        }

        // Choose the top performers for breeding
        for (size_t i = 0; i < _breed_stock; i++)
        {
            _nets[i] = _nets[index[i]];
        }

        // Breed others (N^2 - N)/2 bred nets
        size_t current = _breed_stock;
        for (size_t i = 0; i < _breed_stock; i++)
        {
            for (size_t j = i + 1; j < _breed_stock; j++)
            {
                _nets[current] = mml::nnet<float, 28, 3>::breed(_nets[i], _nets[j]);
                current++;
            }
        }

        // remaining add new random genes to pool
        const size_t remain = _pool_size - ((_breed_stock * _breed_stock + _breed_stock) / 2);
        for (size_t i = 0; i < remain; i++)
        {
            _nets[current].randomize(_rng);
            current++;
        }

        // Mutate random nets
        for (size_t i = 0; i < _mutation_rate; i++)
        {
            // Safe, range 0, _pool_size - 1
            size_t index = (size_t)_rng.random_int();
            _nets[index].mutate(_rng);
        }

        // Print out stuff
        std::cout << "Average fitness is " << _average_fitness << std::endl;
        std::cout << "Best fitness is " << _top << std::endl;
    }
};
}

#endif
