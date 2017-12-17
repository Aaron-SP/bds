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
#ifndef __UI_VERTEX__
#define __UI_VERTEX__

#include <cstring>
#include <min/mesh.h>
#include <min/vec2.h>
#include <min/vec4.h>
#include <min/window.h>

namespace game
{

template <typename T, typename K, GLenum FLOAT_TYPE>
class ui_vertex
{
  private:
    // Turn the Struct of Array (SoA) data into Array of Structs (AoS)

    // These are the struct member sizes
    static constexpr size_t vertex_size = sizeof(min::vec4<T>);
    static constexpr size_t uv_size = sizeof(min::vec2<T>);

    // These are the struct member offsets in floats, not bytes
    static constexpr size_t uv_off = vertex_size / sizeof(T);

    // Compute the size of struct in bytes
    static constexpr size_t width_bytes = vertex_size + uv_size;

    // Compute the size of struct in floats
    static constexpr size_t width_size = width_bytes / sizeof(T);

  public:
    inline static void create()
    {
        // Specify the vertex attributes in location = 0, no offset
        glVertexAttribPointer(0, 4, FLOAT_TYPE, GL_FALSE, width_bytes, nullptr);
        glEnableVertexAttribArray(0);

        // Specify the uv attributes in location = 1, offset is in bytes
        glVertexAttribPointer(1, 2, FLOAT_TYPE, GL_FALSE, width_bytes, (GLvoid *)(uv_off * sizeof(T)));
        glEnableVertexAttribArray(1);
    }
    inline static void check(const min::mesh<T, K> &m)
    {
        // Verify normal, tangent and bitangent sizes
        const auto attr_size = m.vertex.size();
        if (m.uv.size() != attr_size)
        {
            throw std::runtime_error("ui_vertex: vertex & uv invalid length");
        }
    }
    inline static void destroy()
    {
        // Disable the vertex attributes
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }
    inline static void copy(const min::mesh<T, K> &m, std::vector<T> &data, const size_t data_offset, size_t i)
    {
        // Copy the vertex data, 4 floats
        std::memcpy(&data[data_offset], &m.vertex[i], vertex_size);

        // Copy the uv data, 2 floats, offset is in number of floats
        std::memcpy(&data[data_offset + uv_off], &m.uv[i], uv_size);
    }
    inline static constexpr size_t width()
    {
        return width_size;
    }
    inline static constexpr GLenum buffer_type()
    {
        return GL_STATIC_DRAW;
    }
};
}

#endif
