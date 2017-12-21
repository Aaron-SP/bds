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
    // Backgrounds
    static constexpr float _x_cursor_uv = 4.0 / 512.0;
    static constexpr float _y_cursor_uv = 4.0 / 512.0;
    static constexpr float _x_black_uv = 40.0 / 512.0;
    static constexpr float _y_black_uv = 4.0 / 512.0;
    static constexpr float _x_yellow_uv = 76.0 / 512.0;
    static constexpr float _y_yellow_uv = 4.0 / 512.0;
    static constexpr float _x_red_uv = 112.0 / 512.0;
    static constexpr float _y_red_uv = 4.0 / 512.0;
    static constexpr float _x_blue_uv = 148.0 / 512.0;
    static constexpr float _y_blue_uv = 4.0 / 512.0;

    // Icons
    static constexpr float _x_jet_uv = 4.0 / 512.0;
    static constexpr float _y_jet_uv = 40.0 / 512.0;
    static constexpr float _x_ex_uv = 40.0 / 512.0;
    static constexpr float _y_ex_uv = 40.0 / 512.0;
    static constexpr float _x_miss_uv = 76.0 / 512.0;
    static constexpr float _y_miss_uv = 40.0 / 512.0;
    static constexpr float _x_up_uv = 112.0 / 512.0;
    static constexpr float _y_up_uv = 40.0 / 512.0;

    // Scale sizes
    static constexpr float _s_bg = 40.0;
    static constexpr float _s_fg = 32.0;
    static constexpr float _s2_fg = _s_fg / 2;
    static constexpr float _s_uv = 32.0 / 512.0;
    static constexpr float _s_red_x = 32.0;
    static constexpr float _s_red_y = 96.0;
    static constexpr float _s_blue_x = 32.0;
    static constexpr float _s_blue_y = 96.0;

    // Placement values
    static constexpr size_t _num_buttons = 8;
    static constexpr size_t _num_half_buttons = _num_buttons / 2;
    static constexpr float _tool_height = 48.0;
    static constexpr float _tool_space = 48.0;
    static constexpr float _tool_start = -_tool_space * _num_half_buttons + _s_bg / 4;
    static constexpr float _energy_start = _tool_space * _num_half_buttons + _s_bg / 4;
    static constexpr float _health_start = _tool_start - _tool_space - 4.0;

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
    size_t _center_w;
    size_t _center_h;
    size_t _index;
    float _energy;
    float _health;

    inline size_t add_rect()
    {
        // Check for buffer overflow
        if (_v.size() == 20)
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
    inline void set_rect(const size_t index, const min::vec2<float> &p, const min::vec2<float> &scale, const min::vec3<float> &coord)
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
    inline min::vec2<float> toolbar_position(const size_t index)
    {
        // Calculate offset from center for this toolbar element
        const float offset = _tool_start + index * _tool_space;

        // Return toolbar position
        return min::vec2<float>(_center_w + offset, _tool_height);
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
    inline void load_background_black(const size_t index)
    {
        const min::vec2<float> p = toolbar_position(index);
        const min::vec2<float> scale = min::vec2<float>(_s_bg, _s_bg);
        const min::vec3<float> black_coord = min::vec3<float>(_x_black_uv, _y_black_uv, _s_uv);

        // Load rect at position
        set_rect(index, p, scale, black_coord);
    }
    inline void load_background_yellow(const size_t index)
    {
        const min::vec2<float> p = toolbar_position(index);
        const min::vec2<float> scale = min::vec2<float>(_s_bg, _s_bg);
        const min::vec3<float> yellow_coord = min::vec3<float>(_x_yellow_uv, _y_yellow_uv, _s_uv);

        // Load rect at position
        set_rect(index, p, scale, yellow_coord);
    }
    inline void load_fps_cursor(const min::vec2<float> &p)
    {
        const min::vec2<float> scale = min::vec2<float>(_s_fg, _s_fg);
        const min::vec3<float> fps_coord = min::vec3<float>(_x_cursor_uv, _y_cursor_uv, _s_uv);

        // Load rect at position
        set_rect(8, p, scale, fps_coord);
    }
    inline void load_energy_meter()
    {
        const float y_height = _s_blue_y * _energy;
        const float y_offset = (y_height - _s_blue_x) * 0.5 + _tool_height;
        const min::vec2<float> p = min::vec2<float>(_center_w + _energy_start, y_offset);
        const min::vec2<float> scale = min::vec2<float>(_s_blue_x, y_height);
        const min::vec3<float> blue_coord = min::vec3<float>(_x_blue_uv, _y_blue_uv, _s_uv);

        // Load rect at position
        set_rect(9, p, scale, blue_coord);
    }
    inline void load_health_meter()
    {
        const float y_height = _s_red_y * _health;
        const float y_offset = (y_height - _s_red_x) * 0.5 + _tool_height;
        const min::vec2<float> p = min::vec2<float>(_center_w + _health_start, y_offset);
        const min::vec2<float> scale = min::vec2<float>(_s_red_x, y_height);
        const min::vec3<float> red_coord = min::vec3<float>(_x_red_uv, _y_red_uv, _s_uv);

        // Load rect at position
        set_rect(10, p, scale, red_coord);
    }
    inline void load_jet_icon(const size_t index)
    {
        const min::vec2<float> p = toolbar_position(index);
        const min::vec2<float> scale = min::vec2<float>(_s_fg, _s_fg);
        const min::vec3<float> jet_coord = min::vec3<float>(_x_jet_uv, _y_jet_uv, _s_uv);

        // Load rect at position
        set_rect(11, p, scale, jet_coord);
    }
    inline void load_explode_icon(const size_t index)
    {
        const min::vec2<float> p = toolbar_position(index);
        const min::vec2<float> scale = min::vec2<float>(_s_fg, _s_fg);
        const min::vec3<float> ex_coord = min::vec3<float>(_x_ex_uv, _y_ex_uv, _s_uv);

        // Load rect at position
        set_rect(12, p, scale, ex_coord);
    }
    inline void load_missile_icon(const size_t index)
    {
        const min::vec2<float> p = toolbar_position(index);
        const min::vec2<float> scale = min::vec2<float>(_s_fg, _s_fg);
        const min::vec3<float> miss_coord = min::vec3<float>(_x_miss_uv, _y_miss_uv, _s_uv);

        // Load rect at position
        set_rect(13, p, scale, miss_coord);
    }
    inline void load_up_icon(const size_t index)
    {
        const min::vec2<float> p = toolbar_position(index);
        const min::vec2<float> scale = min::vec2<float>(_s_fg, _s_fg);
        const min::vec3<float> up_coord = min::vec3<float>(_x_up_uv, _y_up_uv, _s_uv);

        // Load rect at position
        set_rect(14, p, scale, up_coord);
    }
    inline void position_ui()
    {
        // Add 8 black rectangles along bottom
        for (size_t i = 0; i < 8; i++)
        {
            set_key_up(i);
        }

        // Add FPS cursor
        const min::vec2<float> p_fps(_center_w - _s2_fg, _center_h - _s2_fg);
        load_fps_cursor(p_fps);

        // Add Health meter
        load_energy_meter();

        // Add Health meter
        load_health_meter();
    }

  public:
    ui_overlay(const game::uniforms &uniforms, const uint16_t width, const uint16_t height)
        : _vertex("data/shader/ui.vertex", GL_VERTEX_SHADER),
          _fragment("data/shader/ui.fragment", GL_FRAGMENT_SHADER),
          _prog(_vertex, _fragment),
          _width(width), _height(height),
          _energy(0.0), _health(1.0)
    {
        // Create the instance rectangle
        load_base_rect();

        // Load texture
        load_texture();

        // Load the uniform buffer with program we will use
        uniforms.set_program_matrix_only(_prog);

        // Add 15 rectangles
        for (size_t i = 0; i < 15; i++)
        {
            add_rect();
        }

        // Reposition all ui on the screen
        position_ui();
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

        // Update screen center
        _center_w = _width / 2;
        _center_h = _height / 2;

        // Reposition all ui on the screen
        position_ui();
    }
    inline void set_energy(const float energy)
    {
        // Set energy in percent
        _energy = energy;

        // Set the size of the health bar
        load_energy_meter();
    }
    inline void set_health(const float health)
    {
        // Set health in percent
        _health = health;

        // Set the size of the health bar
        load_health_meter();
    }
    inline void set_key_down(const size_t index)
    {
        load_background_yellow(index);
    }
    inline void set_key_up(const size_t index)
    {
        load_background_black(index);

        // Draw key overlay
        switch (index)
        {
        case 0:
            return load_jet_icon(index);
        case 1:
            return load_explode_icon(index);
        case 2:
            return load_missile_icon(index);
        case 3:
            return load_up_icon(index);
        }
    }
};
}

#endif
