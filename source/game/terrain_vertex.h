#ifndef __TERRAIN_VERTEX__
#define __TERRAIN_VERTEX__

#include <cstring>
#include <min/mesh.h>
#include <min/vec4.h>
#include <min/window.h>

namespace game
{

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
        return GL_STREAM_DRAW;
    }
};
}

#endif
