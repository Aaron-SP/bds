/* Copyright [2013-2018] [Aaron Springstroh, Minimal Graphics Library]

This file is part of the Fractex.

Fractex is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Fractex is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Fractex.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __MANDELBULB__
#define __MANDELBULB__

#include <functional>
#include <game/thread_map.h>
#include <min/vec3.h>

namespace game
{

class mandelbulb
{
  private:
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
        const size_t d = size / 2;
        x0 = p.x() / d;
        y0 = p.y() / d;
        z0 = p.z() / d;

        // Convergence flag
        bool converged = false;
        size_t iterations = 0;
        for (size_t i = 0; i < 32; i++)
        {
            // X coordinate
            const float dx = (y0 * y0 + z0 * z0);
            const float dx2 = dx * dx;
            const float dx3 = dx2 * dx;
            const float dx4 = dx3 * dx;
            x1 = pow9(x0) - 36.0 * pow7(x0) * dx + 126.0 * pow5(x0) * dx2 - 84.0 * pow3(x0) * dx3 + 9.0 * x0 * dx4 + x0;

            // Y coordinate
            const float dy = (z0 * z0 + x0 * x0);
            const float dy2 = dy * dy;
            const float dy3 = dy2 * dy;
            const float dy4 = dy3 * dy;
            y1 = pow9(y0) - 36.0 * pow7(y0) * dy + 126.0 * pow5(y0) * dy2 - 84.0 * pow3(y0) * dy3 + 9.0 * y0 * dy4 + y0;

            // Z coordinate
            const float dz = (x0 * x0 + y0 * y0);
            const float dz2 = dz * dz;
            const float dz3 = dz2 * dz;
            const float dz4 = dz3 * dz;
            z1 = pow9(z0) - 36.0 * pow7(z0) * dz + 126.0 * pow5(z0) * dz2 - 84.0 * pow3(z0) * dz3 + 9.0 * z0 * dz4 + z0;

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
            return iterations % 8;
        }

        return -1;
    }

  public:
    mandelbulb()
    {
    }
    void generate(std::vector<int8_t> &grid, const size_t gsize, const std::function<min::vec3<float>(const size_t)> &f)
    {
        // Create a threadpool for doing work in parallel
        thread_map map;

        // Create working function
        const auto work = [this, &grid, gsize, &f](const size_t i) {
            // Do work
            grid[i] = do_mandelbulb(f(i), gsize);
        };

        // Run the job in parallel
        map.run(work, 0, grid.size());
    }
};
}

#endif
