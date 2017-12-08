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
#include <min/convert.h>
#endif

#include <min/dds.h>
#include <min/program.h>
#include <min/shader.h>
#include <min/texture_buffer.h>
#include <min/vertex_buffer.h>
#include <numeric>

namespace game
{

class terrain
{
  private:
    // Opengl stuff
    min::shader _tv;
#ifdef USE_GS_RENDER
    min::shader _tg;
#endif
    min::shader _tf;
    min::program _program;
    min::vertex_buffer<float, uint32_t, game::terrain_vertex, GL_FLOAT, GL_UNSIGNED_INT> _pb;
    min::vertex_buffer<float, uint32_t, game::terrain_vertex, GL_FLOAT, GL_UNSIGNED_INT> _gb;
    min::texture_buffer _tbuffer;
    GLuint _dds_id;

#ifndef USE_GS_RENDER
    min::mesh<float, uint32_t> _parent;

    // About 6MB vertex and 4MB index; 8*8*8*7*7*7 = 175616 cells
    static constexpr size_t vertex_reserve_size = 36 * 175616;
    static constexpr size_t uv_reserve_size = 24 * 175616;
#endif

#ifdef USE_GS_RENDER
    static constexpr GLenum RENDER_TYPE = GL_POINTS;
    inline void generate_indices(min::mesh<float, uint32_t> &mesh)
    {
        // Generate indices
        mesh.index.resize(mesh.vertex.size());
        std::iota(mesh.index.begin(), mesh.index.end(), 0);
    }
#else
    static constexpr GLenum RENDER_TYPE = GL_TRIANGLES;
    static inline void append_box(const min::aabbox<float, min::vec3> &b, min::mesh<float, uint32_t> &m, const int8_t atlas_id)
    {
        // Get box dimensions
        const min::vec3<float> &min = b.get_min();
        const min::vec3<float> &max = b.get_max();
        const size_t index_offset = m.vertex.size();

        // Append vertices
        m.vertex.insert(
            m.vertex.end(),
            std::initializer_list<min::vec4<float>>{
                min::vec4<float>(min.x(), min.y(), min.z(), 1.0),
                min::vec4<float>(max.x(), min.y(), max.z(), 1.0),
                min::vec4<float>(min.x(), min.y(), max.z(), 1.0),
                min::vec4<float>(max.x(), max.y(), max.z(), 1.0),
                min::vec4<float>(min.x(), max.y(), min.z(), 1.0),
                min::vec4<float>(min.x(), max.y(), max.z(), 1.0),
                min::vec4<float>(min.x(), max.y(), max.z(), 1.0),
                min::vec4<float>(min.x(), min.y(), min.z(), 1.0),
                min::vec4<float>(min.x(), min.y(), max.z(), 1.0),
                min::vec4<float>(min.x(), max.y(), min.z(), 1.0),
                min::vec4<float>(max.x(), min.y(), min.z(), 1.0),
                min::vec4<float>(min.x(), min.y(), min.z(), 1.0),
                min::vec4<float>(max.x(), min.y(), min.z(), 1.0),
                min::vec4<float>(max.x(), max.y(), max.z(), 1.0),
                min::vec4<float>(max.x(), min.y(), max.z(), 1.0),
                min::vec4<float>(min.x(), min.y(), max.z(), 1.0),
                min::vec4<float>(max.x(), max.y(), max.z(), 1.0),
                min::vec4<float>(min.x(), max.y(), max.z(), 1.0),
                min::vec4<float>(max.x(), min.y(), min.z(), 1.0),
                min::vec4<float>(max.x(), max.y(), min.z(), 1.0),
                min::vec4<float>(min.x(), max.y(), min.z(), 1.0),
                min::vec4<float>(max.x(), max.y(), min.z(), 1.0),
                min::vec4<float>(max.x(), max.y(), min.z(), 1.0),
                min::vec4<float>(max.x(), min.y(), max.z(), 1.0)});

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

        // grass
        if (atlas_id == 5)
        {
            for (auto &uv : uvs)
            {
                uv *= 0.248;
                uv.x(uv.x() + 0.001);
                uv.y(uv.y() + 0.751);
            }
        }
        // stone
        else if (atlas_id == 0)
        {
            for (auto &uv : uvs)
            {
                uv *= 0.248;
                uv.x(uv.x() + 0.251);
                uv.y(uv.y() + 0.751);
            }
        }
        // sand
        else if (atlas_id == 1)
        {
            for (auto &uv : uvs)
            {
                uv *= 0.248;
                uv.x(uv.x() + 0.501);
                uv.y(uv.y() + 0.751);
            }
        }
        // wood
        else if (atlas_id == 3)
        {
            for (auto &uv : uvs)
            {
                uv *= 0.248;
                uv += 0.751;
            }
        }
        // dirt
        else if (atlas_id == 4)
        {
            for (auto &uv : uvs)
            {
                uv *= 0.248;
                uv.x(uv.x() + 0.001);
                uv.y(uv.y() + 0.501);
            }
        }
        // lava
        else if (atlas_id == 2)
        {
            for (auto &uv : uvs)
            {
                uv *= 0.248;
                uv.x(uv.x() + 0.251);
                uv.y(uv.y() + 0.501);
            }
        }
        // water
        else if (atlas_id == 6)
        {
            for (auto &uv : uvs)
            {
                uv *= 0.248;
                uv += 0.501;
            }
        }
        // sulphur
        else if (atlas_id == 7)
        {
            for (auto &uv : uvs)
            {
                uv *= 0.248;
                uv.x(uv.x() + 0.751);
                uv.y(uv.y() + 0.501);
            }
        }

        // Append uv coordinates
        m.uv.insert(m.uv.end(), uvs.begin(), uvs.end());

        // Create indices
        std::array<uint32_t, 36> indices{
            0, 1, 2,
            3, 4, 5,
            6, 7, 8,
            9, 10, 11,
            12, 13, 14,
            15, 16, 17,
            0, 18, 1,
            3, 19, 4,
            6, 20, 7,
            9, 21, 10,
            12, 22, 13,
            15, 23, 16};

        // Offset the uv for each box in the mesh
        for (auto &i : indices)
        {
            i += index_offset;
        }

        // Append indices
        m.index.insert(m.index.end(), indices.begin(), indices.end());

        // Append vertices
        m.normal.insert(
            m.normal.end(),
            std::initializer_list<min::vec3<float>>{
                min::vec3<float>(0.0, -1.0, 0.0),
                min::vec3<float>(0.0, -1.0, 0.0),
                min::vec3<float>(0.0, -1.0, 0.0),
                min::vec3<float>(0.0, 1.0, 0.0),
                min::vec3<float>(0.0, 1.0, 0.0),
                min::vec3<float>(0.0, 1.0, 0.0),
                min::vec3<float>(-1.0, 0.0, 0.0),
                min::vec3<float>(-1.0, 0.0, 0.0),
                min::vec3<float>(-1.0, 0.0, 0.0),
                min::vec3<float>(0.0, 0.0, -1.0),
                min::vec3<float>(0.0, 0.0, -1.0),
                min::vec3<float>(0.0, 0.0, -1.0),
                min::vec3<float>(1.0, 0.0, 0.0),
                min::vec3<float>(1.0, 0.0, 0.0),
                min::vec3<float>(1.0, 0.0, 0.0),
                min::vec3<float>(0.0, 0.0, 1.0),
                min::vec3<float>(0.0, 0.0, 1.0),
                min::vec3<float>(0.0, 0.0, 1.0),
                min::vec3<float>(0.0, -1.0, 0.0),
                min::vec3<float>(0.0, 1.0, 0.0),
                min::vec3<float>(-1.0, 0.0, 0.0),
                min::vec3<float>(0.0, 0.0, -1.0),
                min::vec3<float>(1.0, 0.0, 0.0),
                min::vec3<float>(0.0, 0.0, 1.0)});
    }
    static inline min::aabbox<float, min::vec3> create_box(const min::vec3<float> &center)
    {
        // Create box at center
        const min::vec3<float> min = center - min::vec3<float>(0.5, 0.5, 0.5);
        const min::vec3<float> max = center + min::vec3<float>(0.5, 0.5, 0.5);

        // return the box
        return min::aabbox<float, min::vec3>(min, max);
    }
    inline void generate_mesh(min::mesh<float, uint32_t> &parent, const min::mesh<float, uint32_t> &child)
    {
        // Get number of meshes to generate
        const size_t size = child.vertex.size();

        // Generate meshes in parent for all meshes
        for (size_t i = 0; i < size; i++)
        {
            // Unpack the point and the atlas
            const min::vec4<float> &unpack = child.vertex[i];
            const min::vec3<float> p = min::vec3<float>(unpack.x(), unpack.y(), unpack.z());
            const int8_t atlas = static_cast<int8_t>(unpack.w());

            // Add box to parent mesh
            append_box(create_box(p), parent, atlas);
        }
    }
#endif

  public:
    terrain()
        :
#ifdef USE_GS_RENDER
          _tv("data/shader/terrain_gs.vertex", GL_VERTEX_SHADER),
          _tg("data/shader/terrain_gs.geometry", GL_GEOMETRY_SHADER),
          _tf("data/shader/terrain_gs.fragment", GL_FRAGMENT_SHADER),
          _program({_tv.id(), _tg.id(), _tf.id()})
#else
          _tv("data/shader/terrain.vertex", GL_VERTEX_SHADER),
          _tf("data/shader/terrain.fragment", GL_FRAGMENT_SHADER),
          _program(_tv, _tf),
          _parent("parent")
#endif
    {
        // Load texture
        min::dds tex("data/texture/atlas.dds");

        // Load texture buffer
        _dds_id = _tbuffer.add_dds_texture(tex);

// Reserve space for parent
#ifndef USE_GS_RENDER
        // Reserve space for parent mesh
        _parent.vertex.reserve(vertex_reserve_size);
        _parent.uv.reserve(uv_reserve_size);
        _parent.index.reserve(vertex_reserve_size);
        _parent.normal.reserve(vertex_reserve_size);
#endif
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
    inline void draw_terrain() const
    {
        // Bind VAO
        _gb.bind();

        // Draw graph-mesh
        _gb.draw_all(RENDER_TYPE);
    }
    inline const min::program &get_program() const
    {
        return _program;
    }
    inline void upload_geometry(const std::vector<size_t> &index, const std::function<min::mesh<float, uint32_t> &(const size_t)> f)
    {
        // Reset the buffer
        _gb.clear();

#ifdef USE_GS_RENDER
        // For all meshes
        for (const auto &i : index)
        {
            // Get next mesh
            min::mesh<float, uint32_t> &mesh = f(i);

            // Only add if contains cells
            if (mesh.vertex.size() > 0)
            {
                // Generate indices
                generate_indices(mesh);

                // Add mesh to vertex buffer
                _gb.add_mesh(mesh);
            }
        }
#else
        // Clear out the parent
        _parent.clear();

        // For all meshes
        for (const auto &i : index)
        {
            // Get next mesh
            const min::mesh<float, uint32_t> &child = f(i);

            // Only add if contains cells
            if (child.vertex.size() > 0)
            {
                // generate geometry
                generate_mesh(_parent, child);
            }
        }

        // Add mesh to vertex buffer
        if (_parent.vertex.size() > 0)
        {
            _gb.add_mesh(_parent);
        }
#endif

        // Bind the gb, this is needed!
        _gb.bind();

        // Upload terrain geometry to geometry buffer
        _gb.upload();
    }
    inline void upload_preview(min::mesh<float, uint32_t> &terrain)
    {
        // Reset the buffer
        _pb.clear();

#ifdef USE_GS_RENDER
        // Generate indices
        generate_indices(terrain);

        // Add mesh to the buffer
        _pb.add_mesh(terrain);
#else
        // Clear out the parent
        _parent.clear();

        // Only add if contains cells
        if (terrain.vertex.size() > 0)
        {
            // generate geometry
            generate_mesh(_parent, terrain);
        }

        // Add mesh to the buffer
        if (_parent.vertex.size() > 0)
        {
            _pb.add_mesh(_parent);
        }
#endif

        //Bind the pb, this is needed!
        _pb.bind();

        // Upload the preview geometry to preview buffer
        _pb.upload();
    }
};
}

#endif
