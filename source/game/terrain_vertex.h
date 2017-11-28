#ifndef __TERRAIN_VERTEX__
#define __TERRAIN_VERTEX__

#include <cstring>
#include <min/mesh.h>
#include <min/vec4.h>
#include <min/window.h>

namespace game
{

#ifdef USE_GS_RENDER

template <typename T, typename K, GLenum FLOAT_TYPE>
class terrain_vertex
{
  private:
    // These are the struct member sizes
    static constexpr size_t vertex_size = sizeof(min::vec4<T>);

    // Compute the size of struct in bytes
    static constexpr size_t width_bytes = vertex_size;

    // Compute the size of struct in floats
    static constexpr size_t width_size = width_bytes / sizeof(T);

  public:
    inline static void create()
    {
        // Specify the vertex attributes in location = 0, no offset
        glVertexAttribPointer(0, 4, FLOAT_TYPE, GL_FALSE, width_bytes, nullptr);
        glEnableVertexAttribArray(0);
    }
    inline static void check(const min::mesh<T, K> &m)
    {
        // Do nothing since only vertex data is valid
    }
    inline static void destroy()
    {
        // Disable the vertex attributes
        glDisableVertexAttribArray(0);
    }
    inline static void copy(const min::mesh<T, K> &m, std::vector<T> &data, const size_t data_offset, size_t i)
    {
        // Copy the vertex data, 4 floats
        std::memcpy(&data[data_offset], &m.vertex[i], vertex_size);
    }
    inline static constexpr size_t width()
    {
        return width_size;
    }
    inline static constexpr GLenum buffer_type()
    {
        return GL_DYNAMIC_DRAW;
    }
};

#else

template <typename T, typename K, GLenum FLOAT_TYPE>
class terrain_vertex
{
  private:
    // Turn the Struct of Array (SoA) data into Array of Structs (AoS)

    // These are the struct member sizes
    static constexpr size_t vertex_size = sizeof(min::vec4<T>);
    static constexpr size_t uv_size = sizeof(min::vec2<T>);
    static constexpr size_t normal_size = sizeof(min::vec3<T>);

    // These are the struct member offsets in floats, not bytes
    static constexpr size_t uv_off = vertex_size / sizeof(T);
    static constexpr size_t normal_off = uv_off + (uv_size / sizeof(T));

    // Compute the size of struct in bytes
    static constexpr size_t width_bytes = vertex_size + uv_size + normal_size;

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

        // Specify the normal attributes in location = 2, offset is in bytes
        glVertexAttribPointer(2, 3, FLOAT_TYPE, GL_FALSE, width_bytes, (GLvoid *)(normal_off * sizeof(T)));
        glEnableVertexAttribArray(2);
    }
    inline static void check(const min::mesh<T, K> &m)
    {
        // Verify normal, tangent and bitangent sizes
        const auto attr_size = m.vertex.size();
        if (m.uv.size() != attr_size || m.normal.size() != attr_size)
        {
            throw std::runtime_error("terrain_vertex: uv, normals or tangents invalid length");
        }
    }
    inline static void destroy()
    {
        // Disable the vertex attributes
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
    }
    inline static void copy(const min::mesh<T, K> &m, std::vector<T> &data, const size_t data_offset, size_t i)
    {
        // Copy the vertex data, 4 floats
        std::memcpy(&data[data_offset], &m.vertex[i], vertex_size);

        // Copy the uv data, 2 floats, offset is in number of floats
        std::memcpy(&data[data_offset + uv_off], &m.uv[i], uv_size);

        // Copy the normal data, 3 floats, offset is in number of floats
        std::memcpy(&data[data_offset + normal_off], &m.normal[i], normal_size);
    }
    inline static constexpr size_t width()
    {
        return width_size;
    }
    inline static constexpr GLenum buffer_type()
    {
        return GL_DYNAMIC_DRAW;
    }
};
#endif
}

#endif
