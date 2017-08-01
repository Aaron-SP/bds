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
#ifndef __WORLDMESH__
#define __WORLDMESH__

#include <cmath>
#include <cstdint>
#include <min/aabbox.h>
#include <min/bmp.h>
#include <min/camera.h>
#include <min/convert.h>
#include <min/program.h>
#include <min/shader.h>
#include <min/static_vertex.h>
#include <min/texture_buffer.h>
#include <min/uniform_buffer.h>
#include <min/vec2.h>
#include <min/vec3.h>
#include <min/vec4.h>
#include <min/vertex_buffer.h>
#include <stdexcept>
#include <vector>

namespace game
{

class world_mesh
{
  private:
    // Opengl stuff
    min::shader _tv;
    min::shader _tf;
    min::program _terrain_program;
    min::uniform_buffer<float> _preview;
    min::uniform_buffer<float> _geom;
    min::vertex_buffer<float, uint32_t, min::static_vertex, GL_FLOAT, GL_UNSIGNED_INT> _buffer;
    min::texture_buffer _tbuffer;
    GLuint _bmp_id;

    // Geometry stuff
    std::vector<min::aabbox<float, min::vec3>> _boxes;
    std::vector<size_t> _atlas;
    min::bmp _bmp;
    uint8_t _atlas_id;

    void draw_placemark() const
    {
        // Draw placemarker
        _buffer.draw(GL_TRIANGLES, 0);
    }
    void draw_terrain() const
    {
        // Draw graph-mesh
        if (_boxes.size() > 0)
        {
            _buffer.draw_all_after(GL_TRIANGLES, 0);
        }
    }
    static min::mesh<float, uint32_t> create_box_mesh(const min::aabbox<float, min::vec3> &box, const uint8_t atlas_id)
    {
        min::mesh<float, uint32_t> box_mesh = min::to_mesh<float, uint32_t>(box);
        if (atlas_id == 0)
        {
            for (auto &uv : box_mesh.uv)
            {
                uv *= 0.5;
                uv.y(uv.y() + 0.5);
            }
        }
        else if (atlas_id == 1)
        {
            for (auto &uv : box_mesh.uv)
            {
                uv *= 0.5;
                uv += 0.5;
            }
        }
        else if (atlas_id == 2)
        {
            for (auto &uv : box_mesh.uv)
            {
                uv *= 0.5;
            }
        }
        else if (atlas_id == 3)
        {
            for (auto &uv : box_mesh.uv)
            {
                uv *= 0.5;
                uv.x(uv.x() + 0.5);
            }
        }

        return box_mesh;
    }
    min::aabbox<float, min::vec3> create_box(const min::vec3<float> &center) const
    {
        // Create box at center
        const min::vec3<float> min = min::vec3<float>(-0.5, -0.5, -0.5) + center;
        const min::vec3<float> max = min::vec3<float>(0.5, 0.5, 0.5) + center;

        // return the box
        return min::aabbox<float, min::vec3>(min, max);
    }
    void generate_block(const min::aabbox<float, min::vec3> &box, const size_t atlas_id)
    {
        const min::mesh<float, uint32_t> box_mesh = create_box_mesh(box, atlas_id);

        // upload meshes
        _buffer.add_mesh(box_mesh);
    }
    void load_uniform()
    {
        // Load the uniform buffer with program we will use
        _preview.set_program(_terrain_program);
        _geom.set_program(_terrain_program);

        // Change light alpha for placemark
        const min::vec4<float> col1(1.0, 1.0, 1.0, 1.0);
        const min::vec4<float> pos1(0.0, 100.0, 0.0, 1.0);
        const min::vec4<float> pow1(0.5, 1.0, 0.0, 0.5);
        _preview.add_light(min::light<float>(col1, pos1, pow1));

        // Add light to scene
        const min::vec4<float> col2(1.0, 1.0, 1.0, 1.0);
        const min::vec4<float> pos2(0.0, 100.0, 0.0, 1.0);
        const min::vec4<float> pow2(0.5, 1.0, 0.0, 1.0);
        _geom.add_light(min::light<float>(col2, pos2, pow2));

        // Load projection and view matrix into uniform buffer
        _preview.add_matrix(min::mat4<float>());
        _preview.add_matrix(min::mat4<float>());
        _preview.add_matrix(min::mat4<float>());
        _geom.add_matrix(min::mat4<float>());
        _geom.add_matrix(min::mat4<float>());
        _geom.add_matrix(min::mat4<float>());

        // Load the buffer with data
        _preview.update();
        _geom.update();
    }
    void update_uniform(min::camera<float> &cam)
    {
        // Calculate new placemark point and snap to grid
        const min::vec3<float> translate = cam.project_point(4.0);

        // Update geom matrix uniforms
        _geom.set_matrix(cam.get_pv_matrix(), 0);
        _geom.set_matrix(cam.get_v_matrix(), 1);
        _geom.set_matrix(translate, 2);
        _geom.update_matrix();

        // Update preview matrix uniforms
        _preview.set_matrix(cam.get_pv_matrix(), 0);
        _preview.set_matrix(cam.get_v_matrix(), 1);
        _preview.set_matrix(translate, 2);
        _preview.update_matrix();
    }

  public:
    world_mesh(const std::string &texture_file, const uint32_t size)
        : _tv("data/shader/terrain.vertex", GL_VERTEX_SHADER),
          _tf("data/shader/terrain.fragment", GL_FRAGMENT_SHADER),
          _terrain_program(_tv, _tf),
          _preview(1, 3),
          _geom(1, 3),
          _bmp(texture_file),
          _atlas_id(0)
    {
        // Load texture buffer
        _bmp_id = _tbuffer.add_bmp_texture(_bmp);

        // Load uniform buffers
        load_uniform();

        // Generate the buffer
        generate();
    }

    void add_block(const min::vec3<float> &center)
    {
        // Add box to queue
        const min::aabbox<float, min::vec3> box = create_box(center);
        _boxes.push_back(box);

        // Record current atlas
        _atlas.push_back(_atlas_id);
    }
    void draw(min::camera<float> &cam)
    {
        // Bind this texture for drawing
        _tbuffer.bind(_bmp_id, 0);

        // Bind VAO
        _buffer.bind();

        // update camera matrices
        update_uniform(cam);

        // Use the terrain program for drawing
        _terrain_program.use();

        // Activate the uniform buffer
        _geom.bind();

        // Draw the world geometry
        draw_terrain();

        // Activate the uniform buffer
        _preview.bind();

        // Draw the placemark
        draw_placemark();
    }
    void generate()
    {
        // Reset the buffer
        _buffer.clear();

        // Create the placemark
        min::aabbox<float, min::vec3> box = create_box(min::vec3<float>());
        generate_block(box, _atlas_id);

        // Generate all boxes
        const size_t size = _boxes.size();
        for (size_t i = 0; i < size; i++)
        {
            const min::aabbox<float, min::vec3> &b = _boxes[i];
            generate_block(b, _atlas[i]);
        }

        // Upload contents to the vertex buffer
        _buffer.upload();
    }
    void set_atlas_id(const uint8_t id)
    {
        _atlas_id = id;

        // Regenerate the mesh
        generate();
    }
};
}

#endif
