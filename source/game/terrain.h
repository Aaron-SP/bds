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
#ifndef __TERRAIN_GEOMETRY__
#define __TERRAIN_GEOMETRY__

#include <game/terrain_vertex.h>

#ifndef USE_GS_RENDER
#include <game/work_queue.h>
#endif

#include <min/dds.h>
#include <min/program.h>
#include <min/shader.h>
#include <min/texture_buffer.h>
#include <min/vertex_buffer.h>
#include <numeric>

namespace game
{

#ifdef USE_GS_RENDER

class terrain
{
  private:
    // Opengl stuff
    min::shader _tv;
    min::shader _tg;
    min::shader _tf;
    min::program _program;
    min::vertex_buffer<float, uint32_t, game::terrain_vertex, GL_FLOAT, GL_UNSIGNED_INT> _pb;
    min::vertex_buffer<float, uint32_t, game::terrain_vertex, GL_FLOAT, GL_UNSIGNED_INT> _gb;
    min::texture_buffer _tbuffer;
    GLuint _dds_id;

    static constexpr GLenum RENDER_TYPE = GL_POINTS;
    inline void generate_indices(min::mesh<float, uint32_t> &mesh)
    {
        // Generate indices
        mesh.index.resize(mesh.vertex.size());
        std::iota(mesh.index.begin(), mesh.index.end(), 0);
    }

  public:
    terrain(const size_t chunks)
        : _tv("data/shader/terrain_gs.vertex", GL_VERTEX_SHADER),
          _tg("data/shader/terrain_gs.geometry", GL_GEOMETRY_SHADER),
          _tf("data/shader/terrain_gs.fragment", GL_FRAGMENT_SHADER),
          _program({_tv.id(), _tg.id(), _tf.id()}),
          _gb(chunks)
    {
        // Load texture
        min::dds tex("data/texture/atlas.dds");

        // Load texture buffer
        _dds_id = _tbuffer.add_dds_texture(tex);
    }
    inline void bind() const
    {
        // Bind the terrain texture for drawing
        _tbuffer.bind(_dds_id, 0);

        // Use the terrain program for drawing
        _program.use();
    }
    inline void draw_placemark() const
    {
        // Bind VAO
        _pb.bind();

        // Draw placemarker
        _pb.draw_all(RENDER_TYPE);
    }
    inline void draw_terrain(const std::vector<size_t> &index) const
    {
        // For all chunk meshes
        for (const auto &i : index)
        {
            // Bind VAO
            _gb.bind_buffer(i);

            // Draw graph-mesh
            _gb.draw_all(RENDER_TYPE);
        }
    }
    inline const min::program &get_program() const
    {
        return _program;
    }
    inline void upload_geometry(const size_t index, min::mesh<float, uint32_t> &child)
    {
        // Swap buffer index for this chunk
        _gb.set_buffer(index);

        // Reset the buffer
        _gb.clear();

        // Only add if contains cells
        if (child.vertex.size() > 0)
        {
            // Generate indices
            generate_indices(child);

            // Add mesh to vertex buffer
            _gb.add_mesh(child);

            // Unbind the last VAO to prevent scrambling buffers
            _gb.unbind();

            // Upload terrain geometry to geometry buffer
            _gb.upload();
        }
    }
    inline void upload_preview(min::mesh<float, uint32_t> &terrain)
    {
        // Reset the buffer
        _pb.clear();

        // Only add if contains cells
        if (terrain.vertex.size() > 0)
        {
            // Generate indices
            generate_indices(terrain);

            // Add mesh to the buffer
            _pb.add_mesh(terrain);

            // Unbind the last VAO to prevent scrambling buffers
            _pb.unbind();

            // Upload the preview geometry to preview buffer
            _pb.upload();
        }
    }
};

#else

class terrain
{
  private:
    // About 6MB vertex and 4MB index; 8*8*8*7*7*7 = 175616 cells
    static constexpr size_t _cell_size = 175616;
    static constexpr size_t _reserve_size = 24 * _cell_size;
    static constexpr size_t _index_reserve_size = 36 * _cell_size;

    // Opengl stuff
    min::shader _tv;
    min::shader _tf;
    min::program _program;
    min::vertex_buffer<float, uint32_t, game::terrain_vertex, GL_FLOAT, GL_UNSIGNED_INT> _pb;
    min::vertex_buffer<float, uint32_t, game::terrain_vertex, GL_FLOAT, GL_UNSIGNED_INT> _gb;
    min::texture_buffer _tbuffer;
    GLuint _dds_id;
    min::mesh<float, uint32_t> _parent;

    static constexpr GLenum RENDER_TYPE = GL_TRIANGLES;
    static inline std::array<min::vec2<float>, 24> create_uvs(const int8_t atlas_id)
    {
        //Create UV's for the box
        std::array<min::vec2<float>, 24> uvs{
            min::vec2<float>(1.0, 0.0),
            min::vec2<float>(0.0, 1.0),
            min::vec2<float>(0.0, 0.0),
            min::vec2<float>(1.0, 0.0),
            min::vec2<float>(0.0, 1.0),
            min::vec2<float>(0.0, 0.0),
            min::vec2<float>(1.0, 0.0),
            min::vec2<float>(0.0, 1.0),
            min::vec2<float>(0.0, 0.0),
            min::vec2<float>(1.0, 0.0),
            min::vec2<float>(0.0, 1.0),
            min::vec2<float>(0.0, 0.0),
            min::vec2<float>(0.0, 0.0),
            min::vec2<float>(1.0, 1.0),
            min::vec2<float>(0.0, 1.0),
            min::vec2<float>(1.0, 0.0),
            min::vec2<float>(0.0, 1.0),
            min::vec2<float>(0.0, 0.0),
            min::vec2<float>(1.0, 1.0),
            min::vec2<float>(1.0, 1.0),
            min::vec2<float>(1.0, 1.0),
            min::vec2<float>(1.0, 1.0),
            min::vec2<float>(1.0, 0.0),
            min::vec2<float>(1.0, 1.0)};

        // Select texture from atlas
        switch (atlas_id)
        {
        case 0:
            for (auto &uv : uvs)
            {
                uv *= 0.124;
                uv.x(uv.x() + 0.001);
                uv.y(uv.y() + 0.876);
            }
            break;
        case 1:
            for (auto &uv : uvs)
            {
                uv *= 0.124;
                uv.x(uv.x() + 0.126);
                uv.y(uv.y() + 0.876);
            }
            break;
        case 2:
            for (auto &uv : uvs)
            {
                uv *= 0.124;
                uv.x(uv.x() + 0.251);
                uv.y(uv.y() + 0.876);
            }
            break;
        case 3:
            for (auto &uv : uvs)
            {
                uv *= 0.124;
                uv.x(uv.x() + 0.376);
                uv.y(uv.y() + 0.876);
            }
            break;
        case 4:
            for (auto &uv : uvs)
            {
                uv *= 0.124;
                uv.x(uv.x() + 0.501);
                uv.y(uv.y() + 0.876);
            }
            break;
        case 5:
            for (auto &uv : uvs)
            {
                uv *= 0.124;
                uv.x(uv.x() + 0.626);
                uv.y(uv.y() + 0.876);
            }
            break;
        case 6:
            for (auto &uv : uvs)
            {
                uv *= 0.124;
                uv.x(uv.x() + 0.751);
                uv.y(uv.y() + 0.876);
            }
            break;
        case 7:
            for (auto &uv : uvs)
            {
                uv *= 0.124;
                uv.x(uv.x() + 0.876);
                uv.y(uv.y() + 0.876);
            }
            break;
        case 8:
            for (auto &uv : uvs)
            {
                uv *= 0.124;
                uv.x(uv.x() + 0.001);
                uv.y(uv.y() + 0.751);
            }
            break;
        case 9:
            for (auto &uv : uvs)
            {
                uv *= 0.124;
                uv.x(uv.x() + 0.126);
                uv.y(uv.y() + 0.751);
            }
            break;
        case 10:
            for (auto &uv : uvs)
            {
                uv *= 0.124;
                uv.x(uv.x() + 0.251);
                uv.y(uv.y() + 0.751);
            }
            break;
        case 11:
            for (auto &uv : uvs)
            {
                uv *= 0.124;
                uv.x(uv.x() + 0.376);
                uv.y(uv.y() + 0.751);
            }
            break;
        case 12:
            for (auto &uv : uvs)
            {
                uv *= 0.124;
                uv.x(uv.x() + 0.501);
                uv.y(uv.y() + 0.751);
            }
            break;
        case 13:
            for (auto &uv : uvs)
            {
                uv *= 0.124;
                uv.x(uv.x() + 0.626);
                uv.y(uv.y() + 0.751);
            }
            break;
        case 14:
            for (auto &uv : uvs)
            {
                uv *= 0.124;
                uv.x(uv.x() + 0.751);
                uv.y(uv.y() + 0.751);
            }
            break;
        case 15:
            for (auto &uv : uvs)
            {
                uv *= 0.124;
                uv.x(uv.x() + 0.876);
                uv.y(uv.y() + 0.751);
            }
            break;
        }

        return uvs;
    }
    static inline min::aabbox<float, min::vec3> create_box(const min::vec3<float> &center)
    {
        // Create box at center
        const min::vec3<float> min = center - min::vec3<float>(0.5, 0.5, 0.5);
        const min::vec3<float> max = center + min::vec3<float>(0.5, 0.5, 0.5);

        // return the box
        return min::aabbox<float, min::vec3>(min, max);
    }
    inline void set_cell(const size_t cell, std::vector<min::vec4<float>> &cell_buffer)
    {
        // Unpack the point and the atlas
        const min::vec4<float> &unpack = cell_buffer[cell];

        // Calculate vertex start position
        const size_t vertex_start = 24 * cell;
        const size_t index_start = 36 * cell;

        // Append vertices at index location
        {
            // Create bounding box of cell and get box dimensions
            const min::vec3<float> p = min::vec3<float>(unpack.x(), unpack.y(), unpack.z());
            const min::aabbox<float, min::vec3> b = create_box(p);
            const min::vec3<float> &min = b.get_min();
            const min::vec3<float> &max = b.get_max();

            // Calculate vertex start position
            size_t v = vertex_start;
            _parent.vertex[v++] = min::vec4<float>(min.x(), min.y(), min.z(), 1.0);
            _parent.vertex[v++] = min::vec4<float>(max.x(), min.y(), max.z(), 1.0);
            _parent.vertex[v++] = min::vec4<float>(min.x(), min.y(), max.z(), 1.0);
            _parent.vertex[v++] = min::vec4<float>(max.x(), max.y(), max.z(), 1.0);
            _parent.vertex[v++] = min::vec4<float>(min.x(), max.y(), min.z(), 1.0);
            _parent.vertex[v++] = min::vec4<float>(min.x(), max.y(), max.z(), 1.0);
            _parent.vertex[v++] = min::vec4<float>(min.x(), max.y(), max.z(), 1.0);
            _parent.vertex[v++] = min::vec4<float>(min.x(), min.y(), min.z(), 1.0);
            _parent.vertex[v++] = min::vec4<float>(min.x(), min.y(), max.z(), 1.0);
            _parent.vertex[v++] = min::vec4<float>(min.x(), max.y(), min.z(), 1.0);
            _parent.vertex[v++] = min::vec4<float>(max.x(), min.y(), min.z(), 1.0);
            _parent.vertex[v++] = min::vec4<float>(min.x(), min.y(), min.z(), 1.0);
            _parent.vertex[v++] = min::vec4<float>(max.x(), min.y(), min.z(), 1.0);
            _parent.vertex[v++] = min::vec4<float>(max.x(), max.y(), max.z(), 1.0);
            _parent.vertex[v++] = min::vec4<float>(max.x(), min.y(), max.z(), 1.0);
            _parent.vertex[v++] = min::vec4<float>(min.x(), min.y(), max.z(), 1.0);
            _parent.vertex[v++] = min::vec4<float>(max.x(), max.y(), max.z(), 1.0);
            _parent.vertex[v++] = min::vec4<float>(min.x(), max.y(), max.z(), 1.0);
            _parent.vertex[v++] = min::vec4<float>(max.x(), min.y(), min.z(), 1.0);
            _parent.vertex[v++] = min::vec4<float>(max.x(), max.y(), min.z(), 1.0);
            _parent.vertex[v++] = min::vec4<float>(min.x(), max.y(), min.z(), 1.0);
            _parent.vertex[v++] = min::vec4<float>(max.x(), max.y(), min.z(), 1.0);
            _parent.vertex[v++] = min::vec4<float>(max.x(), max.y(), min.z(), 1.0);
            _parent.vertex[v++] = min::vec4<float>(max.x(), min.y(), max.z(), 1.0);
        }

        // Append uv coordinates at index location
        {
            const int8_t atlas_id = static_cast<int8_t>(unpack.w());
            const std::array<min::vec2<float>, 24> uvs = create_uvs(atlas_id);
            for (size_t i = 0; i < 24; i++)
            {
                _parent.uv[i + vertex_start] = uvs[i];
            }
        }

        // Append indices at index location
        {
            size_t in = index_start;
            _parent.index[in++] = vertex_start;
            _parent.index[in++] = 1 + vertex_start;
            _parent.index[in++] = 2 + vertex_start;
            _parent.index[in++] = 3 + vertex_start;
            _parent.index[in++] = 4 + vertex_start;
            _parent.index[in++] = 5 + vertex_start;
            _parent.index[in++] = 6 + vertex_start;
            _parent.index[in++] = 7 + vertex_start;
            _parent.index[in++] = 8 + vertex_start;
            _parent.index[in++] = 9 + vertex_start;
            _parent.index[in++] = 10 + vertex_start;
            _parent.index[in++] = 11 + vertex_start;
            _parent.index[in++] = 12 + vertex_start;
            _parent.index[in++] = 13 + vertex_start;
            _parent.index[in++] = 14 + vertex_start;
            _parent.index[in++] = 15 + vertex_start;
            _parent.index[in++] = 16 + vertex_start;
            _parent.index[in++] = 17 + vertex_start;
            _parent.index[in++] = vertex_start;
            _parent.index[in++] = 18 + vertex_start;
            _parent.index[in++] = 1 + vertex_start;
            _parent.index[in++] = 3 + vertex_start;
            _parent.index[in++] = 19 + vertex_start;
            _parent.index[in++] = 4 + vertex_start;
            _parent.index[in++] = 6 + vertex_start;
            _parent.index[in++] = 20 + vertex_start;
            _parent.index[in++] = 7 + vertex_start;
            _parent.index[in++] = 9 + vertex_start;
            _parent.index[in++] = 21 + vertex_start;
            _parent.index[in++] = 10 + vertex_start;
            _parent.index[in++] = 12 + vertex_start;
            _parent.index[in++] = 22 + vertex_start;
            _parent.index[in++] = 13 + vertex_start;
            _parent.index[in++] = 15 + vertex_start;
            _parent.index[in++] = 23 + vertex_start;
            _parent.index[in++] = 16 + vertex_start;
        }

        // Append normals at index location
        {
            size_t n = vertex_start;
            _parent.normal[n++] = min::vec3<float>(0.0, -1.0, 0.0);
            _parent.normal[n++] = min::vec3<float>(0.0, -1.0, 0.0);
            _parent.normal[n++] = min::vec3<float>(0.0, -1.0, 0.0);
            _parent.normal[n++] = min::vec3<float>(0.0, 1.0, 0.0);
            _parent.normal[n++] = min::vec3<float>(0.0, 1.0, 0.0);
            _parent.normal[n++] = min::vec3<float>(0.0, 1.0, 0.0);
            _parent.normal[n++] = min::vec3<float>(-1.0, 0.0, 0.0);
            _parent.normal[n++] = min::vec3<float>(-1.0, 0.0, 0.0);
            _parent.normal[n++] = min::vec3<float>(-1.0, 0.0, 0.0);
            _parent.normal[n++] = min::vec3<float>(0.0, 0.0, -1.0);
            _parent.normal[n++] = min::vec3<float>(0.0, 0.0, -1.0);
            _parent.normal[n++] = min::vec3<float>(0.0, 0.0, -1.0);
            _parent.normal[n++] = min::vec3<float>(1.0, 0.0, 0.0);
            _parent.normal[n++] = min::vec3<float>(1.0, 0.0, 0.0);
            _parent.normal[n++] = min::vec3<float>(1.0, 0.0, 0.0);
            _parent.normal[n++] = min::vec3<float>(0.0, 0.0, 1.0);
            _parent.normal[n++] = min::vec3<float>(0.0, 0.0, 1.0);
            _parent.normal[n++] = min::vec3<float>(0.0, 0.0, 1.0);
            _parent.normal[n++] = min::vec3<float>(0.0, -1.0, 0.0);
            _parent.normal[n++] = min::vec3<float>(0.0, 1.0, 0.0);
            _parent.normal[n++] = min::vec3<float>(-1.0, 0.0, 0.0);
            _parent.normal[n++] = min::vec3<float>(0.0, 0.0, -1.0);
            _parent.normal[n++] = min::vec3<float>(1.0, 0.0, 0.0);
            _parent.normal[n++] = min::vec3<float>(0.0, 0.0, 1.0);
        }
    }
    inline void allocate_mesh_buffer(const std::vector<min::vec4<float>> &cell_buffer)
    {
        // Resize the parent mesh from cell size
        const size_t size = cell_buffer.size();

        // Vertex sizes
        const size_t size24 = size * 24;
        _parent.vertex.resize(size24);
        _parent.uv.resize(size24);
        _parent.normal.resize(size24);

        // Index sizes
        const size_t size36 = size * 36;
        _parent.index.resize(size36);
    }

  public:
    terrain(const size_t chunks)
        : _tv("data/shader/terrain.vertex", GL_VERTEX_SHADER),
          _tf("data/shader/terrain.fragment", GL_FRAGMENT_SHADER),
          _program(_tv, _tf),
          _gb(chunks), _parent("parent")
    {
        // Load texture
        min::dds tex("data/texture/atlas.dds");

        // Load texture buffer
        _dds_id = _tbuffer.add_dds_texture(tex);

        // Reserve space for merging
        _parent.vertex.reserve(_reserve_size);
        _parent.index.reserve(_index_reserve_size);
        _parent.uv.reserve(_reserve_size);
        _parent.normal.reserve(_reserve_size);
    }
    inline void bind() const
    {
        // Bind the terrain texture for drawing
        _tbuffer.bind(_dds_id, 0);

        // Use the terrain program for drawing
        _program.use();
    }
    inline void draw_placemark() const
    {
        // Bind VAO
        _pb.bind();

        // Draw placemarker
        _pb.draw_all(RENDER_TYPE);
    }
    inline void draw_terrain(const std::vector<size_t> &index) const
    {
        // For all chunk meshes
        for (const auto &i : index)
        {
            // Bind VAO
            _gb.bind_buffer(i);

            // Draw graph-mesh
            _gb.draw_all(RENDER_TYPE);
        }
    }
    inline const min::program &get_program() const
    {
        return _program;
    }
    inline void upload_geometry(const size_t index, min::mesh<float, uint32_t> &child)
    {
        // Swap buffer index for this chunk
        _gb.set_buffer(index);

        // Reset the buffer
        _gb.clear();

        // Convert cells to mesh in parallel
        const size_t size = child.vertex.size();
        if (size > 0)
        {
            // Reserve space in parent mesh
            allocate_mesh_buffer(child.vertex);

            // Parallelize on generating cells
            const auto work = [this, &child](const size_t i) {
                set_cell(i, child.vertex);
            };

            // Convert cells to mesh in parallel
            work_queue::worker.run(work, 0, size);

            // Add mesh to vertex buffer
            _gb.add_mesh(_parent);

            // Unbind the last VAO to prevent scrambling buffers
            _gb.unbind();

            // Upload terrain geometry to geometry buffer
            _gb.upload();
        }
    }
    inline void upload_preview(min::mesh<float, uint32_t> &terrain)
    {
        // Reset the buffer
        _pb.clear();

        // Only add if contains cells
        if (terrain.vertex.size() > 0)
        {
            // Reserve space in parent mesh
            allocate_mesh_buffer(terrain.vertex);

            // Convert cells to mesh in parallel
            const size_t size = terrain.vertex.size();
            for (size_t i = 0; i < size; i++)
            {
                set_cell(i, terrain.vertex);
            }

            _pb.add_mesh(_parent);

            // Unbind the last VAO to prevent scrambling buffers
            _pb.unbind();

            // Upload the preview geometry to preview buffer
            _pb.upload();
        }
    }
};
#endif
}

#endif
