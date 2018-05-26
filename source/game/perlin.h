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
#ifndef __PERLIN_NOISE__
#define __PERLIN_NOISE__

#include <array>
#include <chrono>
#include <min/vec3.h>
#include <random>

namespace kernel
{

class perlin_noise
{
  private:
    std::array<uint_fast8_t, 512> _p;

    void calc_random_hash_table()
    {
        std::uniform_int_distribution<uint_fast8_t> idist(0, 255);
        std::mt19937 gen(std::chrono::high_resolution_clock::now().time_since_epoch().count());

        const size_t size = _p.size();
        for (size_t i = 0; i < size; i++)
        {
            _p[i] = idist(gen);
        }
    }
    inline float fade(float t) const
    {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }
    inline float lerp(const float a, const float b, const float x) const
    {
        return (b - a) * x + a;
    }
    inline float g(const uint_fast8_t index, const float x, const float y, const float z) const
    {
        // Get a random gradient dot product
        switch (index & 15)
        {
        case 0:
            return x + y;
        case 1:
            return -x + y;
        case 2:
            return x - y;
        case 3:
            return -x - y;
        case 4:
            return x + z;
        case 5:
            return -x + z;
        case 6:
            return x - z;
        case 7:
            return -x - z;
        case 8:
            return y + z;
        case 9:
            return -y + z;
        case 10:
            return y - z;
        case 11:
            return -y - z;
        case 12:
            return y + x;
        case 13:
            return -y + z;
        case 14:
            return y - x;
        case 15:
            return -y - z;
        default:
            return 0;
        }
    }

  public:
    perlin_noise()
    {
        // Calculate random numbers
        calc_random_hash_table();
    }
    inline float perlin(const float x, const float y, const float z) const
    {
        // Calculate hash table indices
        const uint_fast8_t xim = static_cast<uint_fast8_t>(x) & 255;
        const uint_fast8_t yim = static_cast<uint_fast8_t>(y) & 255;
        const uint_fast8_t zim = static_cast<uint_fast8_t>(z) & 255;

        // Unsigned overflow is well defined here!
        const uint_fast8_t xip = xim + 1;
        const uint_fast8_t yip = yim + 1;
        const uint_fast8_t zip = zim + 1;

        // Hash 8 corners on local unit cube
        const uint_fast8_t mmm = _p[_p[_p[xim] + yim] + zim];
        const uint_fast8_t mpm = _p[_p[_p[xim] + yip] + zim];
        const uint_fast8_t mmp = _p[_p[_p[xim] + yim] + zip];
        const uint_fast8_t mpp = _p[_p[_p[xim] + yip] + zip];
        const uint_fast8_t pmm = _p[_p[_p[xip] + yim] + zim];
        const uint_fast8_t ppm = _p[_p[_p[xip] + yip] + zim];
        const uint_fast8_t pmp = _p[_p[_p[xip] + yim] + zip];
        const uint_fast8_t ppp = _p[_p[_p[xip] + yip] + zip];

        // Calculate distance vector within local unit cube
        const float xp = x - static_cast<uint_fast8_t>(x);
        const float yp = y - static_cast<uint_fast8_t>(y);
        const float zp = z - static_cast<uint_fast8_t>(z);

        // Calculate the inverse distance vector
        const float xm = xp - 1.0;
        const float ym = yp - 1.0;
        const float zm = zp - 1.0;

        // Calculate interpolation constants
        const float t = fade(xp);
        const float u = fade(yp);
        const float v = fade(zp);

        // Interpolate along X
        const float x_ym_zm = lerp(g(mmm, xm, ym, zm), g(pmm, xp, ym, zm), t);
        const float x_yp_zm = lerp(g(mpm, xm, yp, zm), g(ppm, xp, yp, zm), t);
        const float x_ym_zp = lerp(g(mmp, xm, ym, zp), g(pmp, xp, ym, zp), t);
        const float x_yp_zp = lerp(g(mpp, xm, yp, zp), g(ppp, xp, yp, zp), t);

        // Interpolate along Y
        const float y_zm = lerp(x_ym_zm, x_yp_zm, u);
        const float y_zp = lerp(x_ym_zp, x_yp_zp, u);

        // Interpolate along Z, map [-2, 2] to [0, 1]
        return lerp(y_zm, y_zp, v) * 0.25 + 0.5;
    }
};
}

#endif
