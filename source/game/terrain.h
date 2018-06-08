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
#ifndef __TERRAIN_GEOMETRY__
#define __TERRAIN_GEOMETRY__

#include <game/terrain_vertex.h>

#ifndef USE_GS_RENDER
#include <game/geometry.h>
#include <game/work_queue.h>
#endif

#include <game/memory_map.h>
#include <min/array_buffer.h>
#include <min/dds.h>
#include <min/program.h>
#include <min/shader.h>
#include <min/texture_buffer.h>
#include <numeric>
#include <stdexcept>

namespace game
{

#ifdef USE_GS_RENDER

class terrain
{
  private:
    min::shader _tv;
    min::shader _tg;
    min::shader _tf;
    min::program _prog;
    min::array_buffer<float, uint32_t, terrain_vertex, GL_FLOAT> _pb;
    min::array_buffer<float, uint32_t, terrain_vertex, GL_FLOAT> _gb;
    min::texture_buffer _tbuffer;
    GLuint _dds_id;
    GLint _pre_loc;

    inline void load_texture()
    {
        // Load texture
        const min::mem_file &atlas = memory_map::memory.get_file("data/texture/atlas.dds");
        min::dds tex(atlas);

        // Load texture buffer
        _dds_id = _tbuffer.add_dds_texture(tex, true);
    }
    inline void reserve_memory(const size_t chunks, const size_t chunk_size)
    {
        // Reserve maximum number of cells in a chunk
        const size_t vertex = chunk_size * chunk_size * chunk_size;

        // Reserve vertex buffer memory for geometry
        for (size_t i = 0; i < chunks; i++)
        {
            _gb.set_buffer(i);
            _gb.reserve(vertex, 1);
        }

        // Reserve vertex buffer memory for preview
        _pb.reserve(vertex, 1);
    }

  public:
    terrain(const uniforms &uniforms, const size_t chunks, const size_t chunk_size)
        : _tv(memory_map::memory.get_file("data/shader/terrain_gs.vertex"), GL_VERTEX_SHADER),
          _tg(memory_map::memory.get_file("data/shader/terrain_gs.geometry"), GL_GEOMETRY_SHADER),
          _tf(memory_map::memory.get_file("data/shader/terrain_gs.fragment"), GL_FRAGMENT_SHADER),
          _prog({_tv.id(), _tg.id(), _tf.id()}),
          _gb(chunks)
    {
        // Load texture
        load_texture();

        // Reserve memory based on chunk scale
        reserve_memory(chunks, chunk_size);

        // Get the start_index uniform location
        _pre_loc = glGetUniformLocation(_prog.id(), "preview");
        if (_pre_loc == -1)
        {
            throw std::runtime_error("terrain: could not find uniform 'preview'");
        }

        // Load the uniform buffer with program we will use
        uniforms.set_program_lights(_prog);
        uniforms.set_program_matrix(_prog);
    }
    inline void bind() const
    {
        // Use the terrain program for drawing
        _prog.use();

        // Bind the terrain texture for drawing
        _tbuffer.bind(_dds_id, 0);
    }
    inline void draw_placemark(const uniforms &uniforms) const
    {
        // Bind VAO
        _pb.bind();

        // Update to use preview
        glUniform1i(_pre_loc, 1);

        // Draw placemarker
        _pb.draw_all(GL_POINTS);
    }
    inline void draw_terrain(const uniforms &uniforms, const std::vector<size_t> &index) const
    {
        // Update to use preview
        glUniform1i(_pre_loc, 0);

        // Bind VAO
        _gb.bind();

        // For all chunk meshes
        for (const auto &i : index)
        {
            // Bind array buffer
            _gb.bind_buffer(i);

            // Draw graph-mesh
            _gb.draw_all(GL_POINTS);
        }
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
    min::shader _tv;
    min::shader _tf;
    min::program _prog;
    min::array_buffer<float, uint32_t, terrain_vertex, GL_FLOAT> _pb;
    min::array_buffer<float, uint32_t, terrain_vertex, GL_FLOAT> _gb;
    min::texture_buffer _tbuffer;
    GLuint _dds_id;
    min::mesh<float, uint32_t> _parent;
    GLint _pre_loc;

    inline void allocate_mesh_buffer(const std::vector<min::vec4<float>> &cell_buffer)
    {
        // Resize the parent mesh from cell size
        const size_t size = cell_buffer.size();

        // Vertex sizes
        const size_t size36 = size * 36;
        _parent.vertex.resize(size36);
        _parent.uv.resize(size36);
        _parent.normal.resize(size36);
    }
    static inline min::aabbox<float, min::vec3> create_box(const min::vec3<float> &center)
    {
        // Create box at center
        const min::vec3<float> min = center - min::vec3<float>(0.5, 0.5, 0.5);
        const min::vec3<float> max = center + min::vec3<float>(0.5, 0.5, 0.5);

        // return the box
        return min::aabbox<float, min::vec3>(min, max);
    }
    inline void load_texture()
    {
        // Load texture
        const min::mem_file &atlas = memory_map::memory.get_file("data/texture/atlas.dds");
        min::dds tex(atlas);

        // Load texture buffer
        _dds_id = _tbuffer.add_dds_texture(tex, true);
    }
    inline void reserve_memory(const size_t chunks, const size_t chunk_size)
    {
        // Reserve maximum number of cells in a chunk
        const size_t cells = chunk_size * chunk_size * chunk_size;
        const size_t vertex = 36 * cells;

        // Reserve maximum size of chunk
        _parent.vertex.reserve(vertex);
        _parent.uv.reserve(vertex);
        _parent.normal.reserve(vertex);

        // Reserve vertex buffer memory for geometry
        for (size_t i = 0; i < chunks; i++)
        {
            _gb.set_buffer(i);
            _gb.reserve(vertex, 1);
        }

        // Reserve vertex buffer memory for preview
        _pb.reserve(vertex, 1);
    }
    inline void set_cell(const size_t cell, std::vector<min::vec4<float>> &cell_buffer)
    {
        // Unpack the point and the atlas
        const min::vec4<float> &unpack = cell_buffer[cell];

        // Calculate vertex start position
        const size_t vertex_start = 36 * cell;

        // Create bounding box of cell and get box dimensions
        const min::vec3<float> p = min::vec3<float>(unpack.x(), unpack.y(), unpack.z());
        const min::aabbox<float, min::vec3> b = create_box(p);
        const min::vec3<float> &min = b.get_min();
        const min::vec3<float> &max = b.get_max();

        // Extract the face type and atlas
        const int_fast8_t face_type = static_cast<int>(unpack.w()) / 255;
        const int_fast8_t atlas_id = static_cast<int>(unpack.w()) % 255;

        // Calculate block vertices
        face_vertex(_parent.vertex, vertex_start, min, max, face_type);

        // Calculate block uv's
        face_uv(_parent.uv, vertex_start, face_type);

        // Scale uv's based off atlas id
        face_uv_scale(_parent.uv, vertex_start, atlas_id);

        // Calculate block normals
        face_normal(_parent.normal, vertex_start, face_type);
    }

  public:
    terrain(const uniforms &uniforms, const size_t chunks, const size_t chunk_size)
        : _tv(memory_map::memory.get_file("data/shader/terrain.vertex"), GL_VERTEX_SHADER),
          _tf(memory_map::memory.get_file("data/shader/terrain.fragment"), GL_FRAGMENT_SHADER),
          _prog(_tv, _tf),
          _gb(chunks), _parent("parent")
    {
        // Load texture
        load_texture();

        // Reserve memory based on chunk scale
        reserve_memory(chunks, chunk_size);

        // Get the start_index uniform location
        _pre_loc = glGetUniformLocation(_prog.id(), "preview");
        if (_pre_loc == -1)
        {
            throw std::runtime_error("terrain: could not find uniform 'preview'");
        }

        // Load the uniform buffer with program we will use
        uniforms.set_program_lights(_prog);
        uniforms.set_program_matrix(_prog);
    }
    inline void bind() const
    {
        // Use the terrain program for drawing
        _prog.use();

        // Bind the terrain texture for drawing
        _tbuffer.bind(_dds_id, 0);
    }
    inline void draw_placemark(const uniforms &uniforms) const
    {
        // Bind VAO
        _pb.bind();

        // Update to use preview
        glUniform1i(_pre_loc, 1);

        // Draw placemarker
        _pb.draw_all(GL_TRIANGLES);
    }
    inline void draw_terrain(const uniforms &uniforms, const std::vector<size_t> &index) const
    {
        // Update to use preview
        glUniform1i(_pre_loc, 0);

        // Bind VAO
        _gb.bind();

        // For all chunk meshes
        for (const auto &i : index)
        {
            // Bind array buffer
            _gb.bind_buffer(i);

            // Draw graph-mesh
            _gb.draw_all(GL_TRIANGLES);
        }
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
            const auto work = [this, &child](std::mt19937 &gen, const size_t i) {
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
        const size_t size = terrain.vertex.size();
        if (size > 0)
        {
            // Reserve space in parent mesh
            allocate_mesh_buffer(terrain.vertex);

            // Convert cells to mesh
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
