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
#include <game/goal_seek.h>
#include <game/thread_pool.h>
#include <game/world.h>
#include <min/vec3.h>
#include <utility>

namespace game
{

class ai_opt
{
  private:
    static constexpr unsigned _pool_size = 512;
    static constexpr float _inv_pool_size = 1.0 / _pool_size;
    static constexpr unsigned _species = 64;
    static constexpr unsigned _species_size = _pool_size / _species;
    static constexpr unsigned _species_half_size = (_species_size / 2) - 1;
    static constexpr float _inv_species = 1.0 / _species;
    static constexpr float _inv_species_size = 1.0 / _species_size;
    static constexpr float _inv_sample_size = 1.0 / (_species_size - 1);
    static constexpr unsigned _mutation_rate = 1.0 * _pool_size;
    static constexpr unsigned _start_life = 1;
    static constexpr unsigned _epoch = 6550;
    mutable mml::net_rng<float> _rng;
    thread_pool _pool;
    std::vector<std::vector<size_t>> _mob_index;
    std::vector<std::vector<ai_path>> _paths;
    std::vector<std::vector<path_data>> _data;
    std::vector<std::vector<float>> _scores;
    std::vector<std::vector<size_t>> _breed_stock;
    float _ave[_species];
    size_t _species_top[_species];
    std::pair<size_t, size_t> _top;
    float _average_top;
    float _average_fitness;
    mutable ai_path _top_path;
    mutable float _top_score;
    mutable unsigned _mutations;
    mutable unsigned _year;
    mutable bool _cataclysm;

    void average_fitness_score()
    {
        // Initialize statistical variables
        _average_top = 0.0;
        _average_fitness = 0.0;
        for (size_t i = 0; i < _species; i++)
        {
            _ave[i] = 0.0;
        }

        // Calculate sum of scores per species
        for (size_t i = 0; i < _species; i++)
        {
            for (size_t j = 0; j < _species_size; j++)
            {
                _ave[i] += _scores[i][j];
            }
        }

        // Calculate average score per species
        for (size_t i = 0; i < _species; i++)
        {
            // Calculate total average
            _average_fitness += _ave[i];

            // Normalize species average
            _ave[i] *= _inv_species_size;
        }

        // Normalize total average
        _average_fitness *= _inv_pool_size;

        // Create breeding partners greater than average score
        for (size_t i = 0; i < _species; i++)
        {
            size_t k = 0;
            for (size_t j = 0; j < _species_size; j++)
            {
                if (_scores[i][j] > _ave[i])
                {
                    _breed_stock[i][k] = j;
                    k++;
                }
            }
        }

        // Find top performer in each species
        for (size_t i = 0; i < _species; i++)
        {
            float top_score = _scores[i][0];
            _species_top[i] = 0;
            for (size_t j = 1; j < _species_size; j++)
            {
                // Get top index for this species
                if (_scores[i][j] > top_score)
                {
                    top_score = _scores[i][j];
                    _species_top[i] = j;
                }
            }
        }

        // Calculate average top species score
        for (size_t i = 0; i < _species; i++)
        {
            const size_t j = _species_top[i];
            _average_top += _scores[i][j];
        }

        // Normalize average top species score
        _average_top *= _inv_species;

        // Record the top score
        for (size_t i = 0; i < _species; i++)
        {
            // Create the top breeder
            const size_t top_index = _species_top[i];
            _breed_stock[i][0] = top_index;

            // If best of species is better than global top
            const bool exceeds_top = _scores[i][top_index] > this->top_fitness();
            if (exceeds_top)
            {
                // New top found
                _top = std::make_pair(i, top_index);
            }
        }

        // Cache the top performing path for thread safety
        _cataclysm = ((_year % _epoch) == 0) || (this->top_fitness() > 100.0);
        if (_cataclysm)
        {
            _top_path = top_path();
            _top_score = top_fitness();
        }
    }
    void evolve()
    {
        // Create index vector to sort 0 to N
        average_fitness_score();

        // Breed in parallel
        const auto breed = [this](const size_t i) {

            // Kill off species if average species performance is below kill threshold
            if (_cataclysm && _ave[i] < _average_fitness)
            {
                // Reseed this species with top performer
                for (size_t j = 0; j < _species_size; j++)
                {
                    // Reset the score, and start life a new man
                    _scores[i][j] = _top_score;

                    // Let the top path structure take over this species
                    _paths[i][j] = _top_path;

                    // Try to improve a winning formula
                    _paths[i][j].mutate(_rng);
                }
            }
            // Breed species groups, if a top exists
            else
            {
                // Create breeding stock
                size_t alpha = 0; // Parent A
                size_t beta = 1;  // Parent B
                for (size_t j = 0; j < _species_size; j++)
                {
                    if (_scores[i][j] < _ave[i])
                    {
                        // Reset the score, and start life a new man
                        _scores[i][j] = _ave[i];

                        // Breed new workers
                        const size_t m = _breed_stock[i][alpha];
                        const size_t n = _breed_stock[i][beta];
                        _paths[i][j] = ai_path::breed(_paths[i][m], _paths[i][n]);

                        // Increment parent B
                        beta++;

                        // (N^2 + N)/2 breeding pairs
                        if (beta > _species_half_size)
                        {
                            alpha++;
                            beta = alpha + 1;
                        }
                    }
                }
            }
        };

        // Run the job in parallel
        _pool.run(breed, 0, _species);

        // Calculate mutations from max fitness
        const float one = 1.0;
        const float approx_max_fitness = std::max(one, std::abs(_average_top - _average_fitness));
        _mutations = (_mutation_rate / approx_max_fitness);

        // Mutate random paths
        for (size_t i = 0; i < _mutations; i++)
        {
            // Safe non negative, see constructor
            const size_t j = _rng.random_int() % _species;
            const size_t k = _rng.random_int() % _species_size;

            // Mutate any path without restriction
            // Preserving top genetics causes simulation to get stuck
            _paths[j][k].mutate(_rng);
        }

        // Increment year count
        _year++;
    }

  public:
    ai_opt(world &w, const min::vec3<float> &start, const min::vec3<float> &dest)
        : _rng(std::uniform_real_distribution<float>(-2.0, 2.0),
               std::uniform_real_distribution<float>(-2.0, 2.0),
               std::uniform_int_distribution<unsigned>(0, _pool_size - 1)),
          _mob_index(_species, std::vector<size_t>(_species_size)),
          _paths(_species, std::vector<ai_path>(_species_size)),
          _data(_species, std::vector<path_data>(_species_size, path_data(start, dest))),
          _scores(_species, std::vector<float>(_species_size, _start_life)),
          _breed_stock(_species, std::vector<size_t>(_species_size, 0)),
          _average_top(0.0), _average_fitness(0.0), _top_score(0.0), _mutations(0), _year(0), _cataclysm(false)
    {
        // Assert we are not stupid
        static_assert(_species_size < _pool_size, "Species size can't be larger than the pool size");

        // Assert we are not stupid
        static_assert(_pool_size % _species == 0, "Speciation must perfectly divide the pool size");

        // Add a mobs to world and initialize paths
        for (size_t i = 0; i < _species; i++)
        {
            for (size_t j = 0; j < _species_size; j++)
            {
                // Create a mob in the world
                _mob_index[i][j] = w.add_mob(start);

                // Initialize ai path
                _paths[i][j].randomize(_rng);
            }
        }
    }
    inline float average_fitness() const
    {
        return _average_fitness;
    }
    void debug() const
    {
        // For all species print out scores
        for (size_t i = 0; i < _species; i++)
        {
            // Print out species average score
            std::cout << "Species " << (i + 1) << ": " << _ave[i] << std::endl;
        }

        // Print out top fitness
        std::cout << "Best species: " << (_top.first + 1) << ", fitness: " << top_fitness() << std::endl;

        // Print out average fitness
        std::cout << "Average fitness: " << _average_fitness << std::endl;

        // Print out mutation count
        std::cout << "Mutations: " << _mutations << std::endl;
    }
    inline void deserialize(const std::vector<uint8_t> &stream)
    {
        // Initialize top path
        ai_path top_path(stream);

        // Initialize all paths with the previous top path
        for (size_t i = 0; i < _species; i++)
        {
            for (size_t j = 0; j < _species_size; j++)
            {
                _paths[i][j] = top_path;
            }
        }
    }
    inline const ai_path &random_path() const
    {
        const unsigned i = _rng.random_int() % _species;
        const unsigned j = _rng.random_int() % _species_size;
        return _paths[i][j];
    }
    inline const ai_path &top_path() const
    {
        return _paths[_top.first][_top.second];
    }
    inline void serialize(std::vector<uint8_t> &stream) const
    {
        // Get the top path float data
        _paths[_top.first][_top.second].serialize(stream);
    }
    inline float top_fitness() const
    {
        // Return top fitness
        return _scores[_top.first][_top.second];
    }
    inline void update_path(const min::vec3<float> &start, const min::vec3<float> &dest)
    {
        for (size_t i = 0; i < _species; i++)
        {
            for (size_t j = 0; j < _species_size; j++)
            {
                _data[i][j] = path_data(start, dest);
            }
        }
    }
    float evolve(world &w)
    {
        // Create working function
        const auto work = [this, &w](const size_t i) {
            for (size_t j = 0; j < _species_size; j++)
            {
                // Get the mob position
                const min::vec3<float> &mp = w.mob_position(_mob_index[i][j]);

                // Update path position
                this->_data[i][j].update(mp);

                // Run fitness model, this caches step direction
                this->_scores[i][j] = this->_paths[i][j].fitness(w.get_grid(), this->_data[i][j]);

                // Update mob position with cached step
                w.mob_path(this->_paths[i][j], this->_data[i][j], this->_mob_index[i][j]);
            }
        };

        // Launch the thread pool
        _pool.launch();

        // Run the job in parallel
        _pool.run(work, 0, _species);

        // Evolve the pool
        evolve();

        // return average fitness
        return _average_fitness;
    }
    bool update_goal(world &w, goal_seek &gs)
    {
        // Goal seek for all mobs
        const bool found = top_fitness() > 100.0;
        if (found)
        {
            // Update new goal
            gs.seek_next(w);

            // Update path, start and dest in optimizer
            update_path(gs.get_start(), gs.get_goal());

            // warp all mobs to new start location
            for (size_t i = 0; i < _species; i++)
            {
                for (size_t j = 0; j < _species_size; j++)
                {
                    const min::vec3<float> &s = gs.get_start();
                    w.mob_warp(s, _mob_index[i][j]);
                }
            }
        }

        return found;
    }
};
}

#endif
