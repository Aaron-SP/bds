/* Copyright [2013-2016] [Aaron Springstroh, Minimal Graphics Library]

This file is part of the MGLCraft.

MGLCraft is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MGLCraft is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MGLCraft.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __TERRAIN_GEOMETRY__
#define __TERRAIN_GEOMETRY__

#include <game/terrain_vertex.h>
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
    min::shader _tg;
    min::shader _tf;
    min::program _program;
    min::vertex_buffer<float, uint32_t, game::terrain_vertex, GL_FLOAT, GL_UNSIGNED_INT> _pb;
    min::vertex_buffer<float, uint32_t, game::terrain_vertex, GL_FLOAT, GL_UNSIGNED_INT> _gb;
    min::texture_buffer _tbuffer;
    GLuint _dds_id;

    inline void generate_indices(min::mesh<float, uint32_t> &mesh)
    {
        // Generate indices
        mesh.index.resize(mesh.vertex.size());
        std::iota(mesh.index.begin(), mesh.index.end(), 0);
    }

  public:
    terrain()
        : _tv("data/shader/terrain_gs.vertex", GL_VERTEX_SHADER),
          _tg("data/shader/terrain_gs.geometry", GL_GEOMETRY_SHADER),
          _tf("data/shader/terrain_gs.fragment", GL_FRAGMENT_SHADER),
          _program({_tv.id(), _tg.id(), _tf.id()})
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
        _pb.draw_all(GL_POINTS);
    }
    inline void draw_terrain() const
    {
        // Bind VAO
        _gb.bind();

        // Draw graph-mesh
        _gb.draw_all(GL_POINTS);
    }
    inline const min::program &get_program() const
    {
        return _program;
    }
    inline void upload_geometry(const std::vector<size_t> &index, const std::function<min::mesh<float, uint32_t> &(const size_t)> f)
    {
        // Reset the buffer
        _gb.clear();

        // For all meshes
        for (const auto &i : index)
        {
            // Get next mesh
            min::mesh<float, uint32_t> &mesh = f(i);

            // Only add if contains cells
            if (mesh.vertex.size() > 0)
            { // Generate indices
                generate_indices(mesh);

                // Add mesh to vertex buffer
                _gb.add_mesh(mesh);
            }
        }

        // Bind the gb, this is needed!
        _gb.bind();

        // Upload terrain geometry to geometry buffer
        _gb.upload();
    }
    inline void upload_preview(min::mesh<float, uint32_t> &terrain)
    {
        // Reset the buffer
        _pb.clear();

        // Generate indices
        generate_indices(terrain);

        // Add mesh to the buffer
        _pb.add_mesh(terrain);

        //Bind the pb, this is needed!
        _pb.bind();

        // Upload the preview geometry to preview buffer
        _pb.upload();
    }
};
}

#endif
