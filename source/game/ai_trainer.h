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
#include <functional>
#include <game/ai_path.h>
#include <game/cgrid.h>
#include <game/thread_pool.h>
#include <min/intersect.h>
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
    static constexpr unsigned _mutation_rate = 5;
    static constexpr unsigned _total_moves = 20;
    mml::nnet<float, 32, 6> _nets[_pool_size];
    float _scores[_pool_size];
    mml::net_rng<float> _rng;
    mml::nnet<float, 32, 6> _top_net;
    float _top;
    float _average_fitness;

    static size_t collisions(const cgrid &grid, const min::vec3<float> &p)
    {
        size_t out = 0;

        // Create player mesh at location
        const min::vec3<float> half_extent(0.45, 0.95, 0.45);
        const min::aabbox<float, min::vec3> player(p - half_extent, p + half_extent);

        // Create collision blocks
        const std::vector<min::aabbox<float, min::vec3>> blocks = grid.create_collision_cells(p);
        for (const auto &b : blocks)
        {
            if (min::intersect(player, b))
            {
                out++;
            }
        }

        return out;
    }
    static void optimize(const cgrid &grid, mml::nnet<float, 32, 6> &net, const min::vec3<float> &start, const min::vec3<float> &dest)
    {
        const min::vec3<float> dir = (dest - start).normalize_safe(min::vec3<float>());
        const float travel = 0.0;
        const float remain = (dest - start).magnitude();

        // Load net with input
        ai_path::load(grid, net, start, dir, travel, remain);

        // Get output from ai model
        mml::vector<float, 6> output = ai_path::model(grid, net, start, dir, travel, remain);

        // Do 10 iterations of back propagation
        for (size_t i = 0; i < 10; i++)
        {
            net.calculate();
            net.backprop(output);
        }
    }
    static void optimize_multi(const cgrid &grid, mml::nnet<float, 32, 6> &net, const std::vector<min::vec3<float>> &start, const min::vec3<float> &dest)
    {
        // Optimize for all start positions
        for (const auto &s : start)
        {
            optimize(grid, net, s, dest);
        }
    }
    static float fitness_score(const cgrid &grid, mml::nnet<float, 32, 6> &net, const min::vec3<float> &start, const min::vec3<float> &dest)
    {
        min::vec3<float> current = start;
        min::vec3<float> dir = (dest - start).normalize_safe(min::vec3<float>());
        float score = 0.0;
        float travel = 0.0;
        float remain = (dest - start).magnitude();

        // For N moves
        for (size_t i = 0; i < _total_moves; i++)
        {
            // Get new travel direction
            const min::vec3<float> step = ai_path::solve(grid, net, current, dir, travel, remain);

            // Calculate distance to starting point
            const min::vec3<float> next = current + step;

            // Check if we picked a bad direction and return start position
            const int8_t atlas = grid.grid_value(next);
            if (atlas != -1)
            {
                // Punish collision
                score -= 1.0;
            }
            else
            {
                // Do not allow moving through walls
                current = next;

                // Calculate distance and direction
                dir = dest - current;
                remain = dir.magnitude();
                if (remain > 1.0)
                {
                    const float denom = 1.0 / remain;
                    dir *= denom;
                }
                travel = (current - start).magnitude();
            }

            // Discourage zero moves
            if (travel < 1.0)
            {
                score--;
            }

            // Reward venture out
            // Are we getting closer to destination
            score += travel / (remain + 1.0);

            // Punish collisions with walls
            score -= collisions(grid, current);
        }

        // return fitness score
        return score;
    }
    static float fitness_score_multi(const cgrid &grid, mml::nnet<float, 32, 6> &net, const std::vector<min::vec3<float>> &start, const min::vec3<float> &dest)
    {
        float out = 0.0;

        // Calculate average fitness for multiple inputs
        for (const auto &s : start)
        {
            out += fitness_score(grid, net, s, dest);
        }
        out /= start.size();

        return out;
    }
    void evolve()
    {
        // Assert that we do not overflow
        static_assert(((_breed_stock * _breed_stock + _breed_stock) / 2) <= _pool_size, "Invalid breed stock dimensions");

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
                _nets[current] = mml::nnet<float, 32, 6>::breed(_nets[i], _nets[j]);
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

  public:
    ai_trainer() : _rng(std::uniform_real_distribution<float>(-2.0, 2.0),
                        std::uniform_real_distribution<float>(-0.5, 0.5),
                        std::uniform_int_distribution<int>(0, _pool_size - 1)),
                   _top(0.0),
                   _average_fitness(0.0)
    {
        // Initialize all the nets
        for (size_t i = 0; i < _pool_size; i++)
        {
            // Create a fresh net
            mml::nnet<float, 32, 6> &net = _nets[i];
            net.add_layer(32);
            net.add_layer(16);
            net.finalize();
            net.randomize(_rng);
        }
    }
    void deserialize(const std::vector<uint8_t> &stream)
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
            mml::nnet<float, 32, 6> &net = _nets[i];

            // Must definalize the net to deserialize it
            net.reset();
            net.deserialize(data);

            // Mutate the net for added variation
            net.mutate(_rng);
        }
    }
    void mutate()
    {
        // Initialize all the with previous top net
        for (size_t i = 0; i < _pool_size; i++)
        {
            // Load previous net and mutate it
            mml::nnet<float, 32, 6> &net = _nets[i];

            // Use the top net to reseed
            net = _top_net;

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
    void train_optimize(const cgrid &grid, const min::vec3<float> &start, const min::vec3<float> &dest)
    {
        // Solve back propagation model
        for (size_t i = 0; i < _pool_size; i++)
        {
            optimize(grid, _nets[i], start, dest);
        }
    }
    void train_optimize(const cgrid &grid, const std::vector<min::vec3<float>> &start, const std::vector<min::vec3<float>> &dest)
    {
        const size_t destinations = dest.size();
        if (destinations > 0)
        {
            // Create a threadpool for doing work in parallel
            thread_pool pool;

            // Create working function
            const auto work = [this, &grid, &start, &dest](const size_t i) {
                // For all destinations, calculate average score
                const size_t destinations = dest.size();
                for (size_t j = 0; j < destinations; j++)
                {
                    optimize_multi(grid, this->_nets[i], start, dest[j]);
                }
            };

            // Run the job in parallel
            pool.run(work, 0, _pool_size);
        }
        else
        {
            throw std::runtime_error("ai_trainer: optimize_multi, need at least one destination point");
        }
    }
    void train_evolve(const cgrid &grid, const min::vec3<float> &start, const min::vec3<float> &dest)
    {
        // Calculate the top fitness score
        _top = fitness_score(grid, _top_net, start, dest);

        // Calculate fitness scores
        for (size_t i = 0; i < _pool_size; i++)
        {
            _scores[i] = fitness_score(grid, _nets[i], start, dest);
        }

        // Evolve the pool
        evolve();
    }
    void train_evolve(const cgrid &grid, const std::vector<min::vec3<float>> &start, const std::vector<min::vec3<float>> &dest)
    {
        // Zero out all scores
        for (size_t i = 0; i < _pool_size; i++)
        {
            _scores[i] = 0.0;
        }
        _top = 0.0;

        const size_t destinations = dest.size();
        if (destinations > 0)
        {
            // Calculate the top average fitness score
            for (size_t j = 0; j < destinations; j++)
            {
                _top += fitness_score_multi(grid, _top_net, start, dest[j]);
            }
            _top /= destinations;

            // Create a threadpool for doing work in parallel
            thread_pool pool;

            // Create working function
            const auto work = [this, &grid, &start, &dest](const size_t i) {
                // For all destinations, calculate average score
                const size_t destinations = dest.size();
                for (size_t j = 0; j < destinations; j++)
                {
                    this->_scores[i] += fitness_score_multi(grid, this->_nets[i], start, dest[j]);
                }
                this->_scores[i] /= destinations;
            };

            // Run the job in parallel
            pool.run(work, 0, _pool_size);

            // Evolve the pool
            evolve();
        }
        else
        {
            throw std::runtime_error("ai_trainer: train_multi, need at least one destination point");
        }
    }
};
}

#endif
