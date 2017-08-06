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
#include <min/physics.h>
#include <min/program.h>
#include <min/ray.h>
#include <min/shader.h>
#include <min/static_vertex.h>
#include <min/texture_buffer.h>
#include <min/tree.h>
#include <min/uniform_buffer.h>
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
    min::vertex_buffer<float, uint32_t, min::static_vertex, GL_FLOAT, GL_UNSIGNED_INT> _pb;
    min::vertex_buffer<float, uint32_t, min::static_vertex, GL_FLOAT, GL_UNSIGNED_INT> _gb;
    min::texture_buffer _tbuffer;
    GLuint _bmp_id;
    min::bmp _bmp;

    // User stuff
    int8_t _atlas_id;
    min::vec3<unsigned> _scale;

    // Grid stuff
    uint32_t _gsize;
    uint32_t _gsize3;
    std::vector<int8_t> _grid;
    min::aabbox<float, min::vec3> _world;

    // Physics stuff
    min::vec3<float> _gravity;
    min::physics<float, uint16_t, uint32_t, min::vec3, min::aabbox, min::aabbox, min::tree> _simulation;
    size_t _char_id;
    size_t _block_count;

    // Removes the preview shape from the grid
    void remove_geometry(const min::vec3<float> &center, const min::vec3<unsigned> &scale)
    {
        // Snap center to grid and generate grid cells
        const min::vec3<float> snapped = snap(center);
        min::vec3<float> p = snapped;

        // x axis
        for (size_t i = 0; i < scale.x(); i++)
        {
            // y axis
            p.y(snapped.y());
            for (size_t j = 0; j < scale.y(); j++)
            {
                // z axis
                p.z(snapped.z());
                for (size_t k = 0; k < scale.z(); k++)
                {
                    // Calculate grid key index
                    const uint32_t key = grid_key(p);

                    // Check if this is an existing box to remove from the simulation
                    if (_grid[key] != -1)
                    {
                        _block_count--;

                        // Reset the cell atlas id
                        _grid[key] = -1;
                    }

                    // Increment z axis
                    p.z(p.z() + 1.0);
                }

                // Increment y axis
                p.y(p.y() + 1.0);
            }

            // Increment x axis
            p.x(p.x() + 1.0);
        }
    }

    // Adds the preview shape to the grid
    void add_geometry(const min::vec3<float> &center, const min::vec3<unsigned> &scale, const size_t atlas_id)
    {
        // Snap center to grid and generate grid cells
        const min::vec3<float> snapped = snap(center);
        min::vec3<float> p = snapped;

        // x axis
        for (size_t i = 0; i < scale.x(); i++)
        {
            // y axis
            p.y(snapped.y());
            for (size_t j = 0; j < scale.y(); j++)
            {
                // z axis
                p.z(snapped.z());
                for (size_t k = 0; k < scale.z(); k++)
                {
                    // Calculate grid key index
                    const uint32_t key = grid_key(p);

                    // Check if this is a new box to add to simulation
                    if (_grid[key] == -1)
                    {
                        _block_count++;
                    }

                    // Set grid atlas
                    _grid[key] = atlas_id;

                    // Increment z axis
                    p.z(p.z() + 1.0);
                }

                // Increment y axis
                p.y(p.y() + 1.0);
            }

            // Increment x axis
            p.x(p.x() + 1.0);
        }
    }
    static min::mesh<float, uint32_t> create_box_mesh(const min::aabbox<float, min::vec3> &box, const int8_t atlas_id)
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
        const min::vec3<float> min = center - min::vec3<float>(0.5, 0.5, 0.5);
        const min::vec3<float> max = center + min::vec3<float>(0.5, 0.5, 0.5);

        // return the box
        return min::aabbox<float, min::vec3>(min, max);
    }
    std::vector<min::aabbox<float, min::vec3>> create_collision_cells(const min::vec3<float> &center) const
    {
        // Surrounding cells
        std::vector<min::aabbox<float, min::vec3>> out;
        out.reserve(36);

        // Snap center to grid and generate grid cells
        const min::vec3<float> snapped = snap(center);
        min::vec3<float> p = snapped - min::vec3<float>(1.0, 1.5, 1.0);

        // x axis
        for (size_t i = 0; i < 3; i++)
        {
            // y axis
            p.y(snapped.y() - 1.5);
            for (size_t j = 0; j < 4; j++)
            {
                // z axis
                p.z(snapped.z() - 1.0);
                for (size_t k = 0; k < 3; k++)
                {
                    // Calculate grid key index
                    const uint32_t key = grid_key(p);

                    // Check if this is a new box to add to simulation
                    if (_grid[key] != -1)
                    {
                        // Create box at this point
                        out.push_back(create_box(p));
                    }

                    // Increment z axis
                    p.z(p.z() + 1.0);
                }

                // Increment y axis
                p.y(p.y() + 1.0);
            }

            // Increment x axis
            p.x(p.x() + 1.0);
        }

        return out;
    }
    void draw_placemark() const
    {
        // Bind VAO
        _pb.bind();

        // Draw placemarker
        _pb.draw_all(GL_TRIANGLES);
    }
    void draw_terrain() const
    {
        // Bind VAO
        _gb.bind();

        // Draw graph-mesh
        _gb.draw_all(GL_TRIANGLES);
    }
    // Generate all geometry in grid and adds it to geometry buffer
    void generate_gb()
    {
        // Reset the buffer
        _gb.clear();

        // For all grid cells generate block
        const size_t size = _grid.size();
        for (size_t i = 0; i < size; i++)
        {
            const int8_t atlas = _grid[i];
            if (atlas != -1)
            {
                // Get center from grid position
                const min::vec3<float> p = grid_center(i);
                const min::aabbox<float, min::vec3> box = create_box(p);

                // Create mesh from box
                const min::mesh<float, uint32_t> box_mesh = create_box_mesh(box, atlas);

                // Add mesh to geometry buffer for drawing
                _gb.add_mesh(box_mesh);
            }
        }

        // Bind the gb VAO
        _gb.bind();

        // Upload contents to the vertex buffer
        _gb.upload();
    }
    // Generates the preview geometry and adds it to preview buffer
    void generate_pb()
    {
        // Reset the buffer
        _pb.clear();

        // Store start point => (0,0,0)
        min::vec3<float> p;

        // x axis
        for (size_t i = 0; i < _scale.x(); i++)
        {
            // y axis
            p.y(0.0);
            for (size_t j = 0; j < _scale.y(); j++)
            {
                // z axis
                p.z(0.0);
                for (size_t k = 0; k < _scale.z(); k++)
                {
                    const min::aabbox<float, min::vec3> box = create_box(p);
                    const min::mesh<float, uint32_t> box_mesh = create_box_mesh(box, _atlas_id);
                    _pb.add_mesh(box_mesh);
                    p.z(p.z() + 1.0);
                }
                p.y(p.y() + 1.0);
            }
            p.x(p.x() + 1.0);
        }

        //Bind the pb VAO
        _pb.bind();

        // Upload contents to the vertex buffer
        _pb.upload();
    }
    inline min::vec3<float> grid_center(const uint32_t index) const
    {
        if (index >= _gsize3)
        {
            throw std::runtime_error("world_mesh: index is not inside the world cell");
        }

        // Precalculate the square scale
        const uint32_t scale2 = _gsize * _gsize;

        // Calculate row, col and height
        const uint32_t row = index / scale2;
        const uint32_t col = (index - row * scale2) / _gsize;
        const uint32_t hei = index - row * scale2 - col * _gsize;

        // Calculate the center point of the box cell
        const float x = row + _world.get_min().x() + 0.5;
        const float y = col + _world.get_min().y() + 0.5;
        const float z = hei + _world.get_min().z() + 0.5;

        return min::vec3<float>(x, y, z);
    }
    inline uint32_t grid_key(const min::vec3<float> &point) const
    {
        if (!_world.point_inside(point))
        {
            throw std::runtime_error("world_mesh: point is not inside the world cell");
        }

        // Compute the grid index from point
        const min::vec3<float> cell_extent(1.0, 1.0, 1.0);
        return min::vec3<float>::grid_key(_world.get_min(), cell_extent, _gsize, point);
    }
    void load_character(const min::vec3<float> &p)
    {
        // Create a box for camera movement and add to simulation
        const min::vec3<float> half_extent(0.45, 0.95, 0.45);
        const min::aabbox<float, min::vec3> box(p - half_extent, p + half_extent);
        _char_id = _simulation.add_body(box, 10.0, 1);

        // Get the physics body for editing
        min::body<float, min::vec3> &body = _simulation.get_body(_char_id);

        // Set this body to be unrotatable
        body.set_no_rotate();
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
    min::vec3<float> ray_trace_after(const min::ray<float, min::vec3> &r)
    {
        const min::vec3<float> cell_extent(1.0, 1.0, 1.0);

        // Calculate the ray trajectory for tracing in grid
        auto grid_ray = min::vec3<float>::grid_ray(cell_extent, r.get_origin(), r.get_direction(), r.get_inverse());

        // Calculate start point in grid index format
        auto index = min::vec3<float>::grid_index(_world.get_min(), cell_extent, r.get_origin());

        // Trace a ray from origin and stop at first populated cell
        size_t next_key = grid_key(r.get_origin());
        bool bad_flag = false;
        unsigned count = 0;
        while (_grid[next_key] == -1 && !bad_flag && count < 5)
        {
            next_key = min::vec3<float>::grid_ray_next(index, grid_ray, bad_flag, _gsize);
            count++;
        }

        // return the point to add geometry
        return grid_center(next_key);
    }
    min::vec3<float> ray_trace_before(const min::ray<float, min::vec3> &r)
    {
        const min::vec3<float> cell_extent(1.0, 1.0, 1.0);

        // Calculate the ray trajectory for tracing in grid
        auto grid_ray = min::vec3<float>::grid_ray(cell_extent, r.get_origin(), r.get_direction(), r.get_inverse());

        // Calculate start point in grid index format
        auto index = min::vec3<float>::grid_index(_world.get_min(), cell_extent, r.get_origin());

        // Trace a ray from origin and stop at before populated cell
        size_t next_key = grid_key(r.get_origin());
        size_t before_key = next_key;
        bool bad_flag = false;
        unsigned count = 0;
        while (_grid[next_key] == -1 && !bad_flag && count < 4)
        {
            before_key = next_key;
            next_key = min::vec3<float>::grid_ray_next(index, grid_ray, bad_flag, _gsize);
            count++;
        }

        // return the point to add geometry
        return grid_center(before_key);
    }
    void update_uniform(min::camera<float> &cam)
    {
        // Calculate new placemark point and snap to grid
        const min::vec3<float> dest = snap(cam.project_point(3.0));

        // Create a ray from camera to destination
        const min::ray<float, min::vec3> r(cam.get_position(), dest);

        // Trace a ray to the destination point to find placement position
        const min::vec3<float> translate = snap(ray_trace_before(r));

        // Set camera at the character position
        const min::vec3<float> &p = character_position();
        cam.set_position(p + min::vec3<float>(0.0, 1.0, 0.0));

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
          _atlas_id(0),
          _scale(2, 1, 2),
          _gsize(2.0 * size), _gsize3(_gsize * _gsize * _gsize), _grid(_gsize3, -1),
          _world(min::vec3<float>(-(float)size, -(float)size, -(float)size), min::vec3<float>(size, size, size)),
          _gravity(0.0, -10.0, 0.0),
          _simulation(_world, _gravity),
          _block_count(0)
    {
        // Load texture buffer
        _bmp_id = _tbuffer.add_bmp_texture(_bmp);

        // Load uniform buffers
        load_uniform();

        // Add starting blocks to simulation
        add_block(min::vec3<float>(-1.0, 0.0, -1.0));

        // Reset scale
        _scale = min::vec3<unsigned>(1, 1, 1);

        // Generate the preview buffer
        generate_pb();

        // Set the collision elasticity of the physics simulation
        _simulation.set_elasticity(0.1);

        // Reload the character
        load_character(min::vec3<float>(0.0, 2.0, 0.0));
    }

    void remove_block(const min::ray<float, min::vec3> &r)
    {
        // Trace a ray to the destination point to find placement position
        const min::vec3<float> traced = ray_trace_after(r);

        // remove from grid
        remove_geometry(traced, _scale);

        // generate new mesh
        generate_gb();
    }
    void add_block(const min::vec3<float> &center)
    {
        // Add to grid
        add_geometry(center, _scale, _atlas_id);

        // generate new mesh
        generate_gb();
    }
    void add_block(const min::ray<float, min::vec3> &r)
    {
        // Trace a ray to the destination point to find placement position
        const min::vec3<float> traced = ray_trace_before(r);

        // Add to grid
        add_geometry(traced, _scale, _atlas_id);

        // generate new mesh
        generate_gb();
    }
    void character_jump(const min::vec3<float> &vel)
    {
        min::body<float, min::vec3> &body = _simulation.get_body(_char_id);

        // If not jumping or falling, allow jump
        if (std::abs(body.get_linear_velocity().y()) < 1.0)
        {
            // Update the position
            body.add_force(vel * 4000.0 * body.get_mass());
        }
    }
    void character_move(const min::vec3<float> &vel)
    {
        min::body<float, min::vec3> &body = _simulation.get_body(_char_id);

        // Get the current position and set y movement to zero
        const min::vec3<float> dxz = min::vec3<float>(vel.x(), 0.0, vel.z()).normalize();

        // Update the position
        body.add_force(dxz * 1E2 * body.get_mass());
    }
    const min::vec3<float> &character_position() const
    {
        const min::body<float, min::vec3> &body = _simulation.get_body(_char_id);

        // Return the character position
        return body.get_position();
    }
    void draw(min::camera<float> &cam, const float dt)
    {
        // Get the player physics object
        min::body<float, min::vec3> &body = _simulation.get_body(_char_id);

        // Solve the physics simulation
        for (int i = 0; i < 10; i++)
        {
            // Get player position
            const min::vec3<float> &p = body.get_position();

            const std::vector<min::aabbox<float, min::vec3>> col_blocks = create_collision_cells(p);

            // Add friction force
            const min::vec3<float> &vel = body.get_linear_velocity();
            const min::vec3<float> xz(vel.x(), 0.0, vel.z());

            // Add friction force opposing lateral motion
            body.add_force(xz * body.get_mass() * -2.0);

            _simulation.solve_static(col_blocks, _char_id, dt / 10.0, 10.0);
        }

        // Bind this texture for drawing
        _tbuffer.bind(_bmp_id, 0);

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
    void set_atlas_id(const int8_t id)
    {
        _atlas_id = id;

        // Regenerate the preview mesh
        generate_pb();
    }
    void set_scale_x(unsigned dx)
    {
        if (_scale.x() < 5)
        {
            _scale.x(_scale.x() + dx);

            // Regenerate the preview mesh
            generate_pb();
        }
    }
    void set_scale_y(unsigned dy)
    {
        if (_scale.y() < 5)
        {
            _scale.y(_scale.y() + dy);

            // Regenerate the preview mesh
            generate_pb();
        }
    }
    void set_scale_z(unsigned dz)
    {
        if (_scale.z() < 5)
        {
            _scale.z(_scale.z() + dz);

            // Regenerate the preview mesh
            generate_pb();
        }
    }
    void reset_scale()
    {
        _scale = min::vec3<unsigned>(1, 1, 1);

        // Regenerate the preview mesh
        generate_pb();
    }
    inline min::vec3<float> snap(const min::vec3<float> &point) const
    {
        const float x = point.x();
        const float y = point.y();
        const float z = point.z();

        return min::vec3<float>(std::floor(x) + 0.5, std::floor(y) + 0.5, std::floor(z) + 0.5);
    }
};
}

#endif
