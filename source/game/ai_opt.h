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
#ifndef __AI_OPTIMIZER__
#define __AI_OPTIMIZER__

#include <algorithm>
#include <functional>
#include <game/ai_path.h>
#include <game/cgrid.h>
#include <game/thread_pool.h>
#include <min/vec3.h>
#include <mml/nnet.h>
#include <mml/vec.h>
#include <utility>

namespace game
{

class ai_opt
{
  private:
    static constexpr unsigned _max_iter = 100;
    static constexpr unsigned _mutation_rate = 5;
    static constexpr unsigned _pool_size = 100;
    static constexpr unsigned _total_moves = 100;
    mml::net_rng<float> _rng;
    ai_path _paths[_pool_size];
    ai_path _top_path;
    float _scores[_pool_size];
    float _top;
    float _average_fitness;

    void average_fitness_score(size_t index[_pool_size])
    {
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
            _top_path = _paths[index[0]];
        }

        // Print out stuff
        std::cout << "Average fitness is " << _average_fitness << std::endl;
        std::cout << "Best fitness is " << _top << std::endl;
    }
    void fitness_score(const cgrid &grid, const min::vec3<float> &start, const min::vec3<float> &dest)
    {
        // Calculate top fitness
        path_data top_p_data(start, dest);
        _top += _top_path.fitness(grid, top_p_data);

        // Update fitness for all paths
        for (size_t i = 0; i < _pool_size; i++)
        {
            path_data p_data(start, dest);
            _scores[i] += _paths[i].fitness(grid, p_data);
        }
    }
    static float fitness_score_multi(const cgrid &grid, const ai_path &path, const std::vector<min::vec3<float>> &start, const min::vec3<float> &dest)
    {
        float out = 0.0;

        // Calculate average fitness for multiple input locations
        for (const auto &s : start)
        {
            path_data p_data(s, dest);
            out += path.fitness_multi(grid, p_data, _total_moves);
        }
        out /= start.size();

        return out;
    }
    void fitness_score_total_multi(const cgrid &grid, const std::vector<min::vec3<float>> &start, const std::vector<min::vec3<float>> &dest)
    {
        const size_t destinations = dest.size();
        if (destinations > 0)
        {
            // For all destinations, calculate average score
            for (size_t j = 0; j < destinations; j++)
            {
                _top += fitness_score_multi(grid, _top_path, start, dest[j]);
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
                    this->_scores[i] += fitness_score_multi(grid, this->_paths[i], start, dest[j]);
                }
                this->_scores[i] /= destinations;
            };

            // Run the job in parallel
            pool.run(work, 0, _pool_size);
        }
        else
        {
            throw std::runtime_error("ai_opt: fitness_score_total_multi, need at least one destination point");
        }
    }
    float optimize_multi(const cgrid &grid, ai_path &path, const std::vector<min::vec3<float>> &start, const min::vec3<float> &dest)
    {
        // Optimize for all start positions
        float error = 0.0;
        for (const auto &s : start)
        {
            error += path.optimize(_rng, grid, s, dest, _max_iter);
        }

        return error;
    }

    void evolve()
    {
        // Create index vector to sort 0 to N
        size_t index[_pool_size];
        average_fitness_score(index);

        // Kill off the bottom performers and breed with top tier
        size_t count = 0;
        for (size_t i = 0; i < _pool_size; i++)
        {
            if (_scores[i] < -10.0)
            {
                // Reset the score, and start life a new man
                _scores[i] = 0.0;

                // Breed genetics from top tier
                _paths[i] = ai_path::breed(_paths[index[count]], _paths[index[count + 1]]);

                // Increment top index
                count++;

                // Reset count if overflow
                if (count + 1 >= _pool_size)
                {
                    count = 0;
                }
            }
        }

        // Mutate random paths
        for (size_t i = 0; i < _mutation_rate; i++)
        {
            // Safe, range 0, _pool_size - 1
            const size_t index = static_cast<size_t>(_rng.random_int());
            _paths[index].mutate(_rng);
        }
    }

  public:
    ai_opt() : _rng(std::uniform_real_distribution<float>(-2.0, 2.0),
                    std::uniform_real_distribution<float>(-0.5, 0.5),
                    std::uniform_int_distribution<int>(0, _pool_size - 1)),
               _paths{},
               _top_path(),
               _top(0.0),
               _average_fitness(0.0)
    {
        // Initialize top_path
        _top_path.randomize(_rng);

        // Initialize all the paths
        for (size_t i = 0; i < _pool_size; i++)
        {
            // Create a fresh path
            _paths[i].randomize(_rng);
        }

        // Reset scores to 0
        reset_scores();
    }
    void deserialize(const std::vector<uint8_t> &stream)
    {
        // Initialize top path
        _top_path.deserialize(stream);

        // Initialize all the with previous top path
        for (size_t i = 0; i < _pool_size; i++)
        {
            // Must definalize the path to deserialize it
            _paths[i] = _top_path;

            // Mutate the path for added variation
            _paths[i].mutate(_rng);
        }
    }
    const ai_path &get_top_path() const
    {
        return _top_path;
    }
    void mutate_pool()
    {
        // Initialize all the with previous top path and mutate
        for (size_t i = 0; i < _pool_size; i++)
        {
            // Use the top path to reseed
            _paths[i] = _top_path;

            // Mutate the path for added variation
            _paths[i].mutate(_rng);
        }
    }
    void mutate_top()
    {
        _top_path.mutate(_rng);
    }
    void reset_scores()
    {
        // Zero out all scores
        for (size_t i = 0; i < _pool_size; i++)
        {
            _scores[i] = 0.0;
        }
        _top = 0.0;
    }
    void serialize(std::vector<uint8_t> &stream)
    {
        // Get the top path float data
        const std::vector<float> data = _top_path.serialize();

        // Write data into stream
        min::write_le_vector<float>(stream, data);
    }
    float top_fitness() const
    {
        // Return top fitness
        return _top;
    }
    float evolve(const cgrid &grid, const min::vec3<float> &start, const min::vec3<float> &dest)
    {
        // Fitness
        fitness_score(grid, start, dest);

        // Evolve the pool
        evolve();

        // return average fitness
        return _average_fitness;
    }
    float evolve_multi(const cgrid &grid, const std::vector<min::vec3<float>> &start, const std::vector<min::vec3<float>> &dest)
    {
        // Fitness
        fitness_score_total_multi(grid, start, dest);

        // Evolve the pool
        evolve();

        // return average fitness
        return _average_fitness;
    }
    float optimize(const cgrid &grid, const std::vector<min::vec3<float>> &start, const std::vector<min::vec3<float>> &dest)
    {
        // For all destinations, calculate average score
        float error = 0.0;
        const size_t destinations = dest.size();
        for (size_t i = 0; i < destinations; i++)
        {
            error += optimize_multi(grid, this->_top_path, start, dest[i]);
        }

        // return backprop error
        return error;
    }
};
}

#endif
