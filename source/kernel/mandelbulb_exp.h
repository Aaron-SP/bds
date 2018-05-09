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
#ifndef __MANDELBULB_EXP__
#define __MANDELBULB_EXP__

#include <functional>
#include <game/thread_pool.h>
#include <min/vec3.h>

namespace kernel
{

class mandelbulb_exp
{
  private:
    int _a;
    int _b;
    int _c;
    int _d;
    inline static float pow9(const float x)
    {
        return x * x * x * x * x * x * x * x * x;
    }
    inline static float pow7(const float x)
    {
        return x * x * x * x * x * x * x;
    }
    inline static float pow5(const float x)
    {
        return x * x * x * x * x;
    }
    inline static float pow3(const float x)
    {
        return x * x * x;
    }
    inline int8_t do_mandelbulb(const min::vec3<float> &p, const size_t size)
    {
        // Copy point
        float x0, x1;
        float y0, y1;
        float z0, z1;

        // Set start point
        const size_t d = static_cast<size_t>(size * 0.6667);
        x0 = p.x() / d;
        y0 = p.y() / d;
        z0 = p.z() / d;

        // Convergence flag
        bool converged = false;
        size_t iterations = 0;
        for (size_t i = 0; i < 32; i++)
        {
            // X coordinate
            const float dx = std::exp((y0 * y0 + z0 * z0) * -1.0);
            const float dx2 = dx * dx;
            const float dx3 = dx2 * dx;
            const float dx4 = dx3 * dx;
            x1 = pow9(x0) - _a * pow7(x0) * dx + _b * pow5(x0) * dx2 - _c * pow3(x0) * dx3 + _d * x0 * dx4 + x0;

            // Y coordinate
            const float dy = std::exp((z0 * z0 + x0 * x0) * -1.0);
            const float dy2 = dy * dy;
            const float dy3 = dy2 * dy;
            const float dy4 = dy3 * dy;
            y1 = pow9(y0) - _a * pow7(y0) * dy + _b * pow5(y0) * dy2 - _c * pow3(y0) * dy3 + _d * y0 * dy4 + y0;

            // Z coordinate
            const float dz = std::exp((x0 * x0 + y0 * y0) * -1.0);
            const float dz2 = dz * dz;
            const float dz3 = dz2 * dz;
            const float dz4 = dz3 * dz;
            z1 = pow9(z0) - _a * pow7(z0) * dz + _b * pow5(z0) * dz2 - _c * pow3(z0) * dz3 + _d * z0 * dz4 + z0;

            if (std::abs(x1 - x0) < 1E-3 && std::abs(y1 - y0) < 1E-3 && std::abs(z1 - z0) < 1E-3)
            {
                converged = true;
                iterations = i;
                break;
            }

            // Prime next loop
            x0 = x1;
            y0 = y1;
            z0 = z1;
        }

        // If we converged return atlas
        if (converged)
        {
            return iterations % 24;
        }

        return -1;
    }

  public:
    mandelbulb_exp(std::mt19937 &rng)
    {
        // Seed the random number generator
        const int seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        rng.seed(seed);

        // Generate random range
        std::uniform_int_distribution<int> range(1, 15);

        // Generate coefficients between range
        std::uniform_int_distribution<int> coeff(1, range(rng));

        // Generate the values
        _a = coeff(rng);
        _b = coeff(rng);
        _c = coeff(rng);
        _d = coeff(rng);

        std::cout << "exp mandelbulb fractal: " << std::endl;
        std::cout << "seed: " << seed << std::endl;
        std::cout << "A: " << _a << std::endl;
        std::cout << "B: " << _b << std::endl;
        std::cout << "C: " << _c << std::endl;
        std::cout << "D: " << _d << std::endl;
    }
    inline void generate(game::thread_pool &pool, std::vector<int8_t> &grid, const size_t gsize, const std::function<min::vec3<float>(const size_t)> &f)
    {
        // Create working function
        const auto work = [this, &grid, gsize, &f](std::mt19937 &gen, const size_t i) {
            // Do mandelbulb on this cell if empty
            if (grid[i] == -1)
            {
                grid[i] = do_mandelbulb(f(i), gsize);
            }
        };

        // Run the job in parallel
        pool.run(work, 0, grid.size());
    }
};
}

#endif
