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
#ifndef __GEOMETRY__
#define __GEOMETRY__

#include <min/vec2.h>
#include <min/vec3.h>
#include <min/vec4.h>
#include <vector>

namespace game
{

inline void block_vertex(std::vector<min::vec4<float>> &vertex, size_t i, const min::vec3<float> &min, const min::vec3<float> &max)
{
    // Populate vector with block vertices
    vertex[i++] = min::vec4<float>(min.x(), min.y(), min.z(), 1.0);
    vertex[i++] = min::vec4<float>(max.x(), min.y(), max.z(), 1.0);
    vertex[i++] = min::vec4<float>(min.x(), min.y(), max.z(), 1.0);
    vertex[i++] = min::vec4<float>(max.x(), max.y(), max.z(), 1.0);
    vertex[i++] = min::vec4<float>(min.x(), max.y(), min.z(), 1.0);
    vertex[i++] = min::vec4<float>(min.x(), max.y(), max.z(), 1.0);
    vertex[i++] = min::vec4<float>(min.x(), max.y(), max.z(), 1.0);
    vertex[i++] = min::vec4<float>(min.x(), min.y(), min.z(), 1.0);
    vertex[i++] = min::vec4<float>(min.x(), min.y(), max.z(), 1.0);
    vertex[i++] = min::vec4<float>(min.x(), max.y(), min.z(), 1.0);
    vertex[i++] = min::vec4<float>(max.x(), min.y(), min.z(), 1.0);
    vertex[i++] = min::vec4<float>(min.x(), min.y(), min.z(), 1.0);
    vertex[i++] = min::vec4<float>(max.x(), min.y(), min.z(), 1.0);
    vertex[i++] = min::vec4<float>(max.x(), max.y(), max.z(), 1.0);
    vertex[i++] = min::vec4<float>(max.x(), min.y(), max.z(), 1.0);
    vertex[i++] = min::vec4<float>(min.x(), min.y(), max.z(), 1.0);
    vertex[i++] = min::vec4<float>(max.x(), max.y(), max.z(), 1.0);
    vertex[i++] = min::vec4<float>(min.x(), max.y(), max.z(), 1.0);
    vertex[i++] = min::vec4<float>(max.x(), min.y(), min.z(), 1.0);
    vertex[i++] = min::vec4<float>(max.x(), max.y(), min.z(), 1.0);
    vertex[i++] = min::vec4<float>(min.x(), max.y(), min.z(), 1.0);
    vertex[i++] = min::vec4<float>(max.x(), max.y(), min.z(), 1.0);
    vertex[i++] = min::vec4<float>(max.x(), max.y(), min.z(), 1.0);
    vertex[i++] = min::vec4<float>(max.x(), min.y(), max.z(), 1.0);
}

inline void block_uv(std::vector<min::vec2<float>> &uv, size_t i)
{
    uv[i++] = min::vec2<float>(1.0, 0.0);
    uv[i++] = min::vec2<float>(0.0, 1.0);
    uv[i++] = min::vec2<float>(0.0, 0.0);
    uv[i++] = min::vec2<float>(1.0, 0.0);
    uv[i++] = min::vec2<float>(0.0, 1.0);
    uv[i++] = min::vec2<float>(0.0, 0.0);
    uv[i++] = min::vec2<float>(1.0, 0.0);
    uv[i++] = min::vec2<float>(0.0, 1.0);
    uv[i++] = min::vec2<float>(0.0, 0.0);
    uv[i++] = min::vec2<float>(1.0, 0.0);
    uv[i++] = min::vec2<float>(0.0, 1.0);
    uv[i++] = min::vec2<float>(0.0, 0.0);
    uv[i++] = min::vec2<float>(0.0, 0.0);
    uv[i++] = min::vec2<float>(1.0, 1.0);
    uv[i++] = min::vec2<float>(0.0, 1.0);
    uv[i++] = min::vec2<float>(1.0, 0.0);
    uv[i++] = min::vec2<float>(0.0, 1.0);
    uv[i++] = min::vec2<float>(0.0, 0.0);
    uv[i++] = min::vec2<float>(1.0, 1.0);
    uv[i++] = min::vec2<float>(1.0, 1.0);
    uv[i++] = min::vec2<float>(1.0, 1.0);
    uv[i++] = min::vec2<float>(1.0, 1.0);
    uv[i++] = min::vec2<float>(1.0, 0.0);
    uv[i++] = min::vec2<float>(1.0, 1.0);
}

static inline void block_uv_scale(std::vector<min::vec2<float>> &uv, const size_t index, const int_fast8_t atlas_id)
{
    // Calculate grid index
    const size_t col = atlas_id % 8;
    const size_t row = atlas_id / 8;
    const float x_offset = 0.001 + 0.125 * col;
    const float y_offset = 0.001 + (1.0 - 0.125 * (row + 1));

    // Scale at uv's in place
    const size_t end = index + 24;
    for (size_t i = index; i < end; i++)
    {
        uv[i] *= 0.124;
        uv[i].x(uv[i].x() + x_offset);
        uv[i].y(uv[i].y() + y_offset);
    }
}

inline void block_normal(std::vector<min::vec3<float>> &normal, size_t i)
{
    normal[i++] = min::vec3<float>(0.0, -1.0, 0.0);
    normal[i++] = min::vec3<float>(0.0, -1.0, 0.0);
    normal[i++] = min::vec3<float>(0.0, -1.0, 0.0);
    normal[i++] = min::vec3<float>(0.0, 1.0, 0.0);
    normal[i++] = min::vec3<float>(0.0, 1.0, 0.0);
    normal[i++] = min::vec3<float>(0.0, 1.0, 0.0);
    normal[i++] = min::vec3<float>(-1.0, 0.0, 0.0);
    normal[i++] = min::vec3<float>(-1.0, 0.0, 0.0);
    normal[i++] = min::vec3<float>(-1.0, 0.0, 0.0);
    normal[i++] = min::vec3<float>(0.0, 0.0, -1.0);
    normal[i++] = min::vec3<float>(0.0, 0.0, -1.0);
    normal[i++] = min::vec3<float>(0.0, 0.0, -1.0);
    normal[i++] = min::vec3<float>(1.0, 0.0, 0.0);
    normal[i++] = min::vec3<float>(1.0, 0.0, 0.0);
    normal[i++] = min::vec3<float>(1.0, 0.0, 0.0);
    normal[i++] = min::vec3<float>(0.0, 0.0, 1.0);
    normal[i++] = min::vec3<float>(0.0, 0.0, 1.0);
    normal[i++] = min::vec3<float>(0.0, 0.0, 1.0);
    normal[i++] = min::vec3<float>(0.0, -1.0, 0.0);
    normal[i++] = min::vec3<float>(0.0, 1.0, 0.0);
    normal[i++] = min::vec3<float>(-1.0, 0.0, 0.0);
    normal[i++] = min::vec3<float>(0.0, 0.0, -1.0);
    normal[i++] = min::vec3<float>(1.0, 0.0, 0.0);
    normal[i++] = min::vec3<float>(0.0, 0.0, 1.0);
}

template <class T>
inline void block_index(std::vector<T> &index, size_t i, const T vertex_start)
{
    // Make sure index is unsigned type
    static_assert(std::is_unsigned<T>::value, "geometry: block_index(): template parameter must be unsigned");

    index[i++] = vertex_start;
    index[i++] = 1 + vertex_start;
    index[i++] = 2 + vertex_start;
    index[i++] = 3 + vertex_start;
    index[i++] = 4 + vertex_start;
    index[i++] = 5 + vertex_start;
    index[i++] = 6 + vertex_start;
    index[i++] = 7 + vertex_start;
    index[i++] = 8 + vertex_start;
    index[i++] = 9 + vertex_start;
    index[i++] = 10 + vertex_start;
    index[i++] = 11 + vertex_start;
    index[i++] = 12 + vertex_start;
    index[i++] = 13 + vertex_start;
    index[i++] = 14 + vertex_start;
    index[i++] = 15 + vertex_start;
    index[i++] = 16 + vertex_start;
    index[i++] = 17 + vertex_start;
    index[i++] = vertex_start;
    index[i++] = 18 + vertex_start;
    index[i++] = 1 + vertex_start;
    index[i++] = 3 + vertex_start;
    index[i++] = 19 + vertex_start;
    index[i++] = 4 + vertex_start;
    index[i++] = 6 + vertex_start;
    index[i++] = 20 + vertex_start;
    index[i++] = 7 + vertex_start;
    index[i++] = 9 + vertex_start;
    index[i++] = 21 + vertex_start;
    index[i++] = 10 + vertex_start;
    index[i++] = 12 + vertex_start;
    index[i++] = 22 + vertex_start;
    index[i++] = 13 + vertex_start;
    index[i++] = 15 + vertex_start;
    index[i++] = 23 + vertex_start;
    index[i++] = 16 + vertex_start;
}

static inline void face_uv_scale(std::vector<min::vec2<float>> &uv, const size_t index, const int_fast8_t atlas_id)
{
    // Calculate grid index
    const size_t col = atlas_id % 8;
    const size_t row = atlas_id / 8;
    const float x_offset = 0.001 + 0.125 * col;
    const float y_offset = 0.001 + (1.0 - 0.125 * (row + 1));

    // Scale at uv's in place
    const size_t end = index + 6;
    for (size_t i = index; i < end; i++)
    {
        uv[i] *= 0.124;
        uv[i].x(uv[i].x() + x_offset);
        uv[i].y(uv[i].y() + y_offset);
    }
}

inline void face_vertex(std::vector<min::vec4<float>> &vertex, size_t i, const min::vec3<float> &min, const min::vec3<float> &max, const int_fast8_t face_type)
{
    switch (face_type)
    {
    case 0:
        vertex[i++] = min::vec4<float>(min.x(), max.y(), max.z(), 1.0);
        vertex[i++] = min::vec4<float>(min.x(), min.y(), min.z(), 1.0);
        vertex[i++] = min::vec4<float>(min.x(), min.y(), max.z(), 1.0);
        vertex[i++] = min::vec4<float>(min.x(), max.y(), max.z(), 1.0);
        vertex[i++] = min::vec4<float>(min.x(), max.y(), min.z(), 1.0);
        vertex[i++] = min::vec4<float>(min.x(), min.y(), min.z(), 1.0);
        break;
    case 1:
        vertex[i++] = min::vec4<float>(max.x(), min.y(), min.z(), 1.0);
        vertex[i++] = min::vec4<float>(max.x(), max.y(), max.z(), 1.0);
        vertex[i++] = min::vec4<float>(max.x(), min.y(), max.z(), 1.0);
        vertex[i++] = min::vec4<float>(max.x(), min.y(), min.z(), 1.0);
        vertex[i++] = min::vec4<float>(max.x(), max.y(), min.z(), 1.0);
        vertex[i++] = min::vec4<float>(max.x(), max.y(), max.z(), 1.0);
        break;
    case 2:
        vertex[i++] = min::vec4<float>(min.x(), min.y(), min.z(), 1.0);
        vertex[i++] = min::vec4<float>(max.x(), min.y(), max.z(), 1.0);
        vertex[i++] = min::vec4<float>(min.x(), min.y(), max.z(), 1.0);
        vertex[i++] = min::vec4<float>(min.x(), min.y(), min.z(), 1.0);
        vertex[i++] = min::vec4<float>(max.x(), min.y(), min.z(), 1.0);
        vertex[i++] = min::vec4<float>(max.x(), min.y(), max.z(), 1.0);
        break;
    case 3:
        vertex[i++] = min::vec4<float>(max.x(), max.y(), max.z(), 1.0);
        vertex[i++] = min::vec4<float>(min.x(), max.y(), min.z(), 1.0);
        vertex[i++] = min::vec4<float>(min.x(), max.y(), max.z(), 1.0);
        vertex[i++] = min::vec4<float>(max.x(), max.y(), max.z(), 1.0);
        vertex[i++] = min::vec4<float>(max.x(), max.y(), min.z(), 1.0);
        vertex[i++] = min::vec4<float>(min.x(), max.y(), min.z(), 1.0);
        break;
    case 4:
        vertex[i++] = min::vec4<float>(min.x(), max.y(), min.z(), 1.0);
        vertex[i++] = min::vec4<float>(max.x(), min.y(), min.z(), 1.0);
        vertex[i++] = min::vec4<float>(min.x(), min.y(), min.z(), 1.0);
        vertex[i++] = min::vec4<float>(min.x(), max.y(), min.z(), 1.0);
        vertex[i++] = min::vec4<float>(max.x(), max.y(), min.z(), 1.0);
        vertex[i++] = min::vec4<float>(max.x(), min.y(), min.z(), 1.0);
        break;
    case 5:
        vertex[i++] = min::vec4<float>(min.x(), min.y(), max.z(), 1.0);
        vertex[i++] = min::vec4<float>(max.x(), max.y(), max.z(), 1.0);
        vertex[i++] = min::vec4<float>(min.x(), max.y(), max.z(), 1.0);
        vertex[i++] = min::vec4<float>(min.x(), min.y(), max.z(), 1.0);
        vertex[i++] = min::vec4<float>(max.x(), min.y(), max.z(), 1.0);
        vertex[i++] = min::vec4<float>(max.x(), max.y(), max.z(), 1.0);
        break;
    }
}

inline void face_uv(std::vector<min::vec2<float>> &uv, size_t i, const int_fast8_t face_type)
{
    switch (face_type)
    {
    case 0:
        uv[i++] = min::vec2<float>(1.0, 0.0);
        uv[i++] = min::vec2<float>(0.0, 1.0);
        uv[i++] = min::vec2<float>(0.0, 0.0);
        uv[i++] = min::vec2<float>(1.0, 0.0);
        uv[i++] = min::vec2<float>(1.0, 1.0);
        uv[i++] = min::vec2<float>(0.0, 1.0);
        break;
    case 1:
        uv[i++] = min::vec2<float>(0.0, 0.0);
        uv[i++] = min::vec2<float>(1.0, 1.0);
        uv[i++] = min::vec2<float>(0.0, 1.0);
        uv[i++] = min::vec2<float>(0.0, 0.0);
        uv[i++] = min::vec2<float>(1.0, 0.0);
        uv[i++] = min::vec2<float>(1.0, 1.0);
        break;
    case 2:
        uv[i++] = min::vec2<float>(1.0, 0.0);
        uv[i++] = min::vec2<float>(0.0, 1.0);
        uv[i++] = min::vec2<float>(0.0, 0.0);
        uv[i++] = min::vec2<float>(1.0, 0.0);
        uv[i++] = min::vec2<float>(1.0, 1.0);
        uv[i++] = min::vec2<float>(0.0, 1.0);
        break;
    case 3:
        uv[i++] = min::vec2<float>(1.0, 0.0);
        uv[i++] = min::vec2<float>(0.0, 1.0);
        uv[i++] = min::vec2<float>(0.0, 0.0);
        uv[i++] = min::vec2<float>(1.0, 0.0);
        uv[i++] = min::vec2<float>(1.0, 1.0);
        uv[i++] = min::vec2<float>(0.0, 1.0);
        break;
    case 4:
        uv[i++] = min::vec2<float>(1.0, 0.0);
        uv[i++] = min::vec2<float>(0.0, 1.0);
        uv[i++] = min::vec2<float>(0.0, 0.0);
        uv[i++] = min::vec2<float>(1.0, 0.0);
        uv[i++] = min::vec2<float>(1.0, 1.0);
        uv[i++] = min::vec2<float>(0.0, 1.0);
        break;
    case 5:
        uv[i++] = min::vec2<float>(1.0, 0.0);
        uv[i++] = min::vec2<float>(0.0, 1.0);
        uv[i++] = min::vec2<float>(0.0, 0.0);
        uv[i++] = min::vec2<float>(1.0, 0.0);
        uv[i++] = min::vec2<float>(1.0, 1.0);
        uv[i++] = min::vec2<float>(0.0, 1.0);
        break;
    }
}

inline void face_normal(std::vector<min::vec3<float>> &normal, size_t i, const int_fast8_t face_type)
{

    switch (face_type)
    {
    case 0:
        normal[i++] = min::vec3<float>(-1.0, 0.0, 0.0);
        normal[i++] = min::vec3<float>(-1.0, 0.0, 0.0);
        normal[i++] = min::vec3<float>(-1.0, 0.0, 0.0);
        normal[i++] = min::vec3<float>(-1.0, 0.0, 0.0);
        normal[i++] = min::vec3<float>(-1.0, 0.0, 0.0);
        normal[i++] = min::vec3<float>(-1.0, 0.0, 0.0);
        break;
    case 1:
        normal[i++] = min::vec3<float>(1.0, 0.0, 0.0);
        normal[i++] = min::vec3<float>(1.0, 0.0, 0.0);
        normal[i++] = min::vec3<float>(1.0, 0.0, 0.0);
        normal[i++] = min::vec3<float>(1.0, 0.0, 0.0);
        normal[i++] = min::vec3<float>(1.0, 0.0, 0.0);
        normal[i++] = min::vec3<float>(1.0, 0.0, 0.0);
        break;
    case 2:
        normal[i++] = min::vec3<float>(0.0, -1.0, 0.0);
        normal[i++] = min::vec3<float>(0.0, -1.0, 0.0);
        normal[i++] = min::vec3<float>(0.0, -1.0, 0.0);
        normal[i++] = min::vec3<float>(0.0, -1.0, 0.0);
        normal[i++] = min::vec3<float>(0.0, -1.0, 0.0);
        normal[i++] = min::vec3<float>(0.0, -1.0, 0.0);
        break;
    case 3:
        normal[i++] = min::vec3<float>(0.0, 1.0, 0.0);
        normal[i++] = min::vec3<float>(0.0, 1.0, 0.0);
        normal[i++] = min::vec3<float>(0.0, 1.0, 0.0);
        normal[i++] = min::vec3<float>(0.0, 1.0, 0.0);
        normal[i++] = min::vec3<float>(0.0, 1.0, 0.0);
        normal[i++] = min::vec3<float>(0.0, 1.0, 0.0);
        break;
    case 4:
        normal[i++] = min::vec3<float>(0.0, 0.0, -1.0);
        normal[i++] = min::vec3<float>(0.0, 0.0, -1.0);
        normal[i++] = min::vec3<float>(0.0, 0.0, -1.0);
        normal[i++] = min::vec3<float>(0.0, 0.0, -1.0);
        normal[i++] = min::vec3<float>(0.0, 0.0, -1.0);
        normal[i++] = min::vec3<float>(0.0, 0.0, -1.0);
        break;
    case 5:
        normal[i++] = min::vec3<float>(0.0, 0.0, 1.0);
        normal[i++] = min::vec3<float>(0.0, 0.0, 1.0);
        normal[i++] = min::vec3<float>(0.0, 0.0, 1.0);
        normal[i++] = min::vec3<float>(0.0, 0.0, 1.0);
        normal[i++] = min::vec3<float>(0.0, 0.0, 1.0);
        normal[i++] = min::vec3<float>(0.0, 0.0, 1.0);
        break;
    }
}
}

#endif
