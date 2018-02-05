/* Copyright [2013-2018] [Aaron Springstroh, Minimal Graphics Library]

This file is part of the Beyond Dying Skies.

Beyond Dying Skies is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Beyond Dying Skies is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Beyond Dying Skies.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __HEIGHT_MAP__
#define __HEIGHT_MAP__

#include <chrono>
#include <cmath>
#include <random>
#include <stdexcept>
#include <vector>

namespace game
{

template <typename T, typename K>
class height_map
{
  private:
    size_t _size;
    T _lower;
    T _upper;
    std::vector<T> _map;
    std::uniform_real_distribution<K> _dist;
    std::mt19937 _gen;

    inline void blur()
    {
        // Gaussian blur kernel, 5x5
        // Sigma = 1
        // exp(-(x * x + y * y) / 2), normalized on [-2, 2] for x and y
        const T kernel[5] = {0.05449, 0.24420, 0.40262, 0.24420, 0.05449};

        // Image length and width
        const size_t end2 = _size - 2;

        // Create a copy of the image to populate map edges
        std::vector<T> copy(_map.size(), 0.0);

        // X Dimensional blur
        {
            for (size_t i = 0; i < 2; i++)
            {
                for (size_t j = 0; j < _size; j++)
                {
                    // Summing convolution
                    for (size_t k = 0; k < 5; k++)
                    {
                        const size_t index = key(i + k, j);
                        copy[key(i, j)] += _map[index] * kernel[k];
                    }
                }
            }

            for (size_t i = 2; i < end2; i++)
            {
                // Starting position
                const size_t i2 = i - 2;
                for (size_t j = 0; j < _size; j++)
                {
                    // Summing convolution
                    for (size_t k = 0; k < 5; k++)
                    {
                        const size_t index = key(i2 + k, j);
                        copy[key(i, j)] += _map[index] * kernel[k];
                    }
                }
            }

            for (size_t i = end2; i < _size; i++)
            {
                // Starting position
                const size_t i4 = i - 4;
                for (size_t j = 0; j < _size; j++)
                {
                    // Summing convolution
                    for (size_t k = 0; k < 5; k++)
                    {
                        const size_t index = key(i4 + k, j);
                        copy[key(i, j)] += _map[index] * kernel[k];
                    }
                }
            }
        }

        // Y Dimensional blur
        {
            for (size_t i = 0; i < _size; i++)
            {
                for (size_t j = 0; j < 2; j++)
                {
                    // Reset the map value
                    _map[key(i, j)] = 0.0;

                    // Summing convolution
                    for (size_t k = 0; k < 5; k++)
                    {
                        const size_t index = key(i, j + k);
                        _map[key(i, j)] += copy[index] * kernel[k];
                    }
                }
                for (size_t j = end2; j < _size; j++)
                {
                    // Starting position
                    const size_t j4 = j - 4;

                    // Reset the map value
                    _map[key(i, j)] = 0.0;

                    // Summing convolution
                    for (size_t k = 0; k < 5; k++)
                    {
                        const size_t index = key(i, j4 + k);
                        _map[key(i, j)] += copy[index] * kernel[k];
                    }
                }
            }

            for (size_t i = 0; i < _size; i++)
            {
                for (size_t j = 2; j < end2; j++)
                {
                    // Starting position
                    const size_t j2 = j - 2;

                    // Reset the map value
                    _map[key(i, j)] = 0.0;

                    // Summing convolution
                    for (size_t k = 0; k < 5; k++)
                    {
                        const size_t index = key(i, j2 + k);
                        _map[key(i, j)] += copy[index] * kernel[k];
                    }
                }
            }
        }
    }
    inline void generate()
    {
        // Generate start indexes
        const size_t end = _size - 1;

        // Generate corner keys
        const size_t ll = key(0, 0);
        const size_t lr = key(end, 0);
        const size_t ul = key(0, end);
        const size_t ur = key(end, end);

        // Generate random values at corners
        _map[ll] = _dist(_gen);
        _map[lr] = _dist(_gen);
        _map[ul] = _dist(_gen);
        _map[ur] = _dist(_gen);

        // Recursively diamond square
        const size_t mid_point = end / 2;
        diamond_square(mid_point, mid_point, mid_point, 1);
    }
    inline size_t key(const size_t x, const size_t y) const
    {
        return _size * x + y;
    }
    inline void diamond_square(const size_t x, const size_t y, const size_t length, const size_t level)
    {
        // Generate diamond keys
        const size_t ll = key(x - length, y - length);
        const size_t lr = key(x + length, y - length);
        const size_t ul = key(x - length, y + length);
        const size_t ur = key(x + length, y + length);

        // Generate average value at center
        const size_t center = key(x, y);
        _map[center] = _dist(_gen) + (_map[ll] + _map[ul] + _map[lr] + _map[ur]) / 4;

        // Generate square keys
        const size_t l = key(x - length, y);
        const size_t r = key(x + length, y);
        const size_t d = key(x, y - length);
        const size_t u = key(x, y + length);

        // Generate random values at corners
        _map[l] = _dist(_gen) + (_map[ll] + _map[ul] + _map[center]) / 3;
        _map[r] = _dist(_gen) + (_map[lr] + _map[ur] + _map[center]) / 3;
        _map[d] = _dist(_gen) + (_map[ll] + _map[lr] + _map[center]) / 3;
        _map[u] = _dist(_gen) + (_map[ul] + _map[ur] + _map[center]) / 3;

        // Recursively call this function
        const size_t half = length / 2;
        if (half > 0)
        {
            _dist = std::uniform_real_distribution<K>(_lower / level, _upper / level);
            diamond_square(x - half, y - half, half, level + 1);
            diamond_square(x + half, y - half, half, level + 1);
            diamond_square(x - half, y + half, half, level + 1);
            diamond_square(x + half, y + half, half, level + 1);
        }
    }
    inline static size_t pow2(const size_t level)
    {
        return 1 << level;
    }

  public:
    height_map(const size_t level, const T lower, const T upper)
        : _size(pow2(level) + 1), _lower(lower), _upper(upper),
          _map(_size * _size), _dist(_lower, _upper),
          _gen(std::chrono::high_resolution_clock::now().time_since_epoch().count())
    {
        // Map size must be odd, and greater than one
        if (level == 0)
        {
            throw std::runtime_error("height_map: level must greater than zero");
        }

        // Generate the random height map
        generate();

        // Use a gaussian blur on the height map
        blur();
    }
    inline const T get(const size_t x, const size_t y) const
    {
        return _map[key(x, y)];
    }
};
}

#endif
