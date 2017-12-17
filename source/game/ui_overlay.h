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
#ifndef _UI_OVERLAY__
#define _UI_OVERLAY__

#include <game/ui_vertex.h>
#include <game/uniforms.h>
#include <min/dds.h>
#include <min/program.h>
#include <min/shader.h>
#include <min/texture_buffer.h>
#include <min/vertex_buffer.h>
#include <vector>

namespace game
{

class ui_overlay
{
  private:
    // OpenGL stuff
    min::shader _vertex;
    min::shader _fragment;
    min::program _prog;

    // Buffer for holding ui and texture
    min::vertex_buffer<float, uint32_t, game::ui_vertex, GL_FLOAT, GL_UNSIGNED_INT> _vb;
    min::texture_buffer _tbuffer;

    // Index stuff
    std::vector<min::mat3<float>> _v;
    std::vector<min::mat3<float>> _uv;
    GLuint _dds_id;

    // Screen properties
    size_t _width;
    size_t _height;
    size_t _index;

    inline size_t add_rect()
    {
        // Check for buffer overflow
        if (_v.size() == 10)
        {
            throw std::runtime_error("ui_overlay: must change default ui count");
        }

        // Add matrix to the matrix buffer
        _v.emplace_back();

        // Add uv matrix to buffer
        _uv.emplace_back();

        // return the index
        return _v.size() - 1;
    }
    inline void set_rect(const size_t index, const min::vec2<float> &scale, const min::vec2<float> &p, const min::vec3<float> &coord)
    {
        // Calculate scale and offset
        const float sx = 2.0 / _width;
        const float sy = 2.0 / _height;

        // Calculate rect dimensions
        const float size_x = scale.x() * sx;
        const float size_y = scale.y() * sy;

        const float ox = p.x() * sx - 1.0;
        const float oy = p.y() * sy - 1.0;

        // Add matrix to the matrix buffer
        _v[index].set_scale(min::vec2<float>(size_x, size_y));
        _v[index].set_translation(min::vec2<float>(ox, oy));

        // Add uv matrix to buffer
        const float z = coord.z();
        _uv[index].set_scale(min::vec2<float>(z, z));
        _uv[index].set_translation(min::vec2<float>(coord.x(), coord.y()));
    }
    inline void load_base_rect()
    {
        // Cached parent mesh
        min::mesh<float, uint32_t> rect("ui");

        // Append vertices
        rect.vertex.insert(
            rect.vertex.end(),
            std::initializer_list<min::vec4<float>>{
                min::vec4<float>(-0.5, -0.5, 1.0, 1.0),
                min::vec4<float>(-0.5, 0.5, 1.0, 1.0),
                min::vec4<float>(0.5, -0.5, 1.0, 1.0),
                min::vec4<float>(0.5, 0.5, 1.0, 1.0)});

        //Create UV's for the box
        std::array<min::vec2<float>, 4> uvs{
            min::vec2<float>(0.0, 0.0),
            min::vec2<float>(0.0, 1.0),
            min::vec2<float>(1.0, 0.0),
            min::vec2<float>(1.0, 1.0)};

        // Append uv coordinates
        rect.uv.insert(rect.uv.end(), uvs.begin(), uvs.end());

        // Create indices
        std::array<uint32_t, 6> indices{
            0, 1, 2,
            2, 1, 3};

        // Append indices
        rect.index.insert(rect.index.end(), indices.begin(), indices.end());

        // Add rect mesh to the buffer
        _index = _vb.add_mesh(rect);

        // Upload the text glyphs to the GPU
        _vb.upload();
    }
    inline void load_texture()
    {
        // Load texture
        min::dds tex("data/texture/ui.dds");

        // Load texture into texture buffer
        _dds_id = _tbuffer.add_dds_texture(tex);
    }
    inline void reposition_ui()
    {
        // Calculate screen center
        const size_t center_x = _width / 2;
        const size_t center_y = _height / 2;

        // Add FPS cursor
        const min::vec2<float> scale(32.0, 32.0);
        min::vec2<float> p(center_x - 16, center_y - 16);
        constexpr float x_cursor_uv = 4.0 / 512.0;
        constexpr float y_cursor_uv = 4.0 / 512.0;
        constexpr float s32_uv = 32.0 / 512.0;
        min::vec3<float> coord(x_cursor_uv, y_cursor_uv, s32_uv);
        set_rect(0, scale, p, coord);

        // Add 8 black rectangles along bottom
        float offset = -184.0;
        constexpr float x_back_uv = 40.0 / 512.0;
        constexpr float y_back_uv = 4.0 / 512.0;
        coord = min::vec3<float>(x_back_uv, y_back_uv, s32_uv);
        for (size_t i = 1; i < 9; i++, offset += 48)
        {
            p = min::vec2<float>(center_x + offset, 48);
            set_rect(i, scale, p, coord);
        }
    }

  public:
    ui_overlay(const game::uniforms &uniforms, const uint16_t width, const uint16_t height)
        : _vertex("data/shader/ui.vertex", GL_VERTEX_SHADER),
          _fragment("data/shader/ui.fragment", GL_FRAGMENT_SHADER),
          _prog(_vertex, _fragment),
          _width(width), _height(height)
    {
        // Create the instance rectangle
        load_base_rect();

        // Load texture
        load_texture();

        // Load the uniform buffer with program we will use
        uniforms.set_program_matrix_only(_prog);

        // Add 9 rectangles
        for (size_t i = 0; i < 9; i++)
        {
            add_rect();
        }

        // Reposition all ui on the screen
        reposition_ui();
    }

    inline void draw(game::uniforms &uniforms) const
    {
        const size_t size = _v.size();
        if (size > 0)
        {
            // Activate the uniform buffer
            uniforms.bind();

            // Bind the text_buffer vao
            _vb.bind();

            // Bind the ui program
            _prog.use();

            // Clear depth for drawing ui
            glClear(GL_DEPTH_BUFFER_BIT);

            // Bind the ui texture for drawing
            _tbuffer.bind(_dds_id, 0);

            // Draw the ui elements
            _vb.draw_many(GL_TRIANGLES, _index, size);
        }
    }
    inline const std::vector<min::mat3<float>> &get_scale() const
    {
        return _v;
    }
    inline const std::vector<min::mat3<float>> &get_uv() const
    {
        return _uv;
    }
    inline void set_screen(const float width, const float height)
    {
        // Update the screen dimensions
        _width = width;
        _height = height;

        // Reposition all ui on the screen
        reposition_ui();
    }
};
}

#endif
