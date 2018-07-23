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
#ifndef _BDS_UI_VERTEX_BDS_
#define _BDS_UI_VERTEX_BDS_

#include <cstring>
#include <min/mesh.h>
#include <min/vec2.h>
#include <min/vec4.h>
#include <min/window.h>
#include <stdexcept>

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
    inline static void change_bind_buffer(const GLuint vbo)
    {
#ifdef MGL_VB43
        // No offset, standard stride, binding point 0
        glBindVertexBuffer(0, vbo, 0, width_bytes);
#else
        // Redundantly recreate the vertex attributes
        create_vertex_attributes();
#endif
    }
    inline static void create_vertex_attributes()
    {
#ifdef MGL_VB43
        // Specify the vertex attributes in location = 0, no offset
        glVertexAttribFormat(0, 4, FLOAT_TYPE, GL_FALSE, 0);
        // Specify the uv attributes in location = 1, offset is in bytes
        glVertexAttribFormat(1, 2, FLOAT_TYPE, GL_FALSE, uv_off * sizeof(T));
#else
        // Specify the vertex attributes in location = 0, no offset
        glVertexAttribPointer(0, 4, FLOAT_TYPE, GL_FALSE, width_bytes, nullptr);
        // Specify the uv attributes in location = 1, offset is in bytes
        glVertexAttribPointer(1, 2, FLOAT_TYPE, GL_FALSE, width_bytes, (GLvoid *)(uv_off * sizeof(T)));
#endif
    }
    inline static void create_buffer_binding(const GLuint vbo, const GLuint bind_point)
    {
#ifdef MGL_VB43
        //  Create the buffer binding point
        glVertexAttribBinding(0, bind_point);
        glVertexAttribBinding(1, bind_point);

        // No offset, standard stride, binding point 0
        glBindVertexBuffer(bind_point, vbo, 0, width_bytes);
#endif
    }
    inline static void create(const GLuint vbo)
    {
        // Enable the attributes
        enable_attributes();

        // Create the vertex attributes
        create_vertex_attributes();

#ifdef MGL_VB43
        // Create the buffer binding point
        create_buffer_binding(vbo, 0);
#endif
    }
    inline static void check(const min::mesh<T, K> &m)
    {
        // Verify normal, tangent and bitangent sizes
        const auto vert_size = m.vertex.size();
        if (m.uv.size() != vert_size)
        {
            throw std::runtime_error("ui_vertex: vertex & uv invalid length");
        }
    }
    inline static void copy(std::vector<T> &data, const min::mesh<T, K> &m, const size_t mesh_offset)
    {
        const auto vert_size = m.vertex.size();
        for (size_t i = 0, j = mesh_offset; i < vert_size; i++, j += width_size)
        {
            // Copy the vertex data, 4 floats
            std::memcpy(&data[j], &m.vertex[i], vertex_size);

            // Copy the uv data, 2 floats, offset is in number of floats
            std::memcpy(&data[j + uv_off], &m.uv[i], uv_size);
        }
    }
    inline static void destroy()
    {
        // Disable the vertex attributes before destruction
        disable_attributes();
    }
    inline static void disable_attributes()
    {
        // Disable the vertex attributes
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }
    inline static void enable_attributes()
    {
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
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
