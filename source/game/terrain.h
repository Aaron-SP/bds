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
#ifndef _BDS_TERRAIN_GEOMETRY_BDS_
#define _BDS_TERRAIN_GEOMETRY_BDS_

#include <game/memory_map.h>
#include <game/terrain_vertex.h>
#include <min/array_buffer.h>
#include <min/dds.h>
#include <min/program.h>
#include <min/shader.h>
#include <min/texture_buffer.h>
#include <stdexcept>

namespace game
{

class terrain
{
  private:
#ifdef MGL_GS_RENDER
    static constexpr GLenum TERRAIN_DRAW_TYPE = GL_POINTS;
    min::shader _tg;
#else
    static constexpr GLenum TERRAIN_DRAW_TYPE = GL_TRIANGLES;
#endif
    min::shader _tv;
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
        // Reserve maximum number of faces in a chunk
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
        :
#ifdef MGL_GS_RENDER
          _tg(memory_map::memory.get_file("data/shader/terrain_gs.geometry"), GL_GEOMETRY_SHADER),
          _tv(memory_map::memory.get_file("data/shader/terrain_gs.vertex"), GL_VERTEX_SHADER),
          _tf(memory_map::memory.get_file("data/shader/terrain_gs.fragment"), GL_FRAGMENT_SHADER),
          _prog({_tv.id(), _tg.id(), _tf.id()}),
#else
          _tv(memory_map::memory.get_file("data/shader/terrain.vertex"), GL_VERTEX_SHADER),
          _tf(memory_map::memory.get_file("data/shader/terrain.fragment"), GL_FRAGMENT_SHADER),
          _prog(_tv, _tf),
#endif
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
        _pb.draw_all(TERRAIN_DRAW_TYPE);
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
            _gb.draw_all(TERRAIN_DRAW_TYPE);
        }
    }
    inline void upload_geometry(const size_t index, min::mesh<float, uint32_t> &child)
    {
        // Swap buffer index for this chunk
        _gb.set_buffer(index);

        // Reset the buffer
        _gb.clear();

        // Only add if contains faces
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

        // Only add if contains faces
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
}

#endif
