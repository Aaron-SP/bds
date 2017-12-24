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
#include <utility>
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
    static constexpr float _x_reload_uv = 4.0 / 512.0;
    static constexpr float _y_reload_uv = 40.0 / 512.0;
    static constexpr float _x_beam_uv = 40.0 / 512.0;
    static constexpr float _y_beam_uv = 40.0 / 512.0;
    static constexpr float _x_miss_uv = 76.0 / 512.0;
    static constexpr float _y_miss_uv = 40.0 / 512.0;
    static constexpr float _x_grap_uv = 112.0 / 512.0;
    static constexpr float _y_grap_uv = 40.0 / 512.0;
    static constexpr float _x_jet_uv = 148.0 / 512.0;
    static constexpr float _y_jet_uv = 40.0 / 512.0;

    // Menu text
    static constexpr float _x_dead_uv = 4.0 / 512.0;
    static constexpr float _y_dead_uv = 384.0 / 512.0;
    static constexpr float _x_pause_uv = 4.0 / 512.0;
    static constexpr float _y_pause_uv = 256.0 / 512.0;

    // Scale sizes
    static constexpr float _s_bg = 40.0;
    static constexpr float _s_fg = 32.0;
    static constexpr float _s_uv = 32.0 / 512.0;
    static constexpr float _s_red_x = 32.0;
    static constexpr float _s_red_y = 96.0;
    static constexpr float _s_blue_x = 32.0;
    static constexpr float _s_blue_y = 96.0;

    // Menu sizes
    static constexpr float _s_menu_x = 504.0;
    static constexpr float _s_menu_y = 124.0;
    static constexpr float _s_menu_uv_x = 504.0 / 512.0;
    static constexpr float _s_menu_uv_y = 124.0 / 512.0;

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
    size_t _selected;
    size_t _menu_offset;
    float _energy;
    float _health;
    float _cursor_angle;
    bool _draw_menu;

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
    inline void set_uv(const size_t index, const min::vec4<float> &coord)
    {
        // Add uv matrix to buffer
        const float sx = coord.z();
        const float sy = coord.w();
        _uv[index].set_scale(min::vec2<float>(sx, sy));
        _uv[index].set_translation(min::vec2<float>(coord.x(), coord.y()));
    }
    inline std::pair<min::vec2<float>, min::vec2<float>> to_screen(const min::vec2<float> &p, const min::vec2<float> &scale)
    {
        // Calculate scale and offset
        const float sx = 2.0 / _width;
        const float sy = 2.0 / _height;

        // Calculate rect dimensions
        const float size_x = scale.x() * sx;
        const float size_y = scale.y() * sy;

        const float ox = p.x() * sx - 1.0;
        const float oy = p.y() * sy - 1.0;

        return std::make_pair(min::vec2<float>(ox, oy), min::vec2<float>(size_x, size_y));
    }
    inline void set_rect(const size_t index, const min::vec2<float> &p, const min::vec2<float> &scale, const min::vec4<float> &coord)
    {
        // Calculate screen coordinates
        const auto ps = to_screen(p, scale);

        // Set matrix components
        _v[index].set_translation(ps.first);
        _v[index].set_scale(ps.second);

        // Set uv coordinates
        set_uv(index, coord);
    }
    inline void set_rect_reset(const size_t index, const min::vec2<float> &p, const min::vec2<float> &scale, const min::vec4<float> &coord)
    {
        // Calculate screen coordinates
        const auto ps = to_screen(p, scale);

        // Add matrix to the matrix buffer
        _v[index] = min::mat3<float>(ps.first);
        _v[index].set_scale(ps.second);

        // Set uv coordinates
        set_uv(index, coord);
    }
    inline void set_rect_rot(const size_t index, const min::vec2<float> &p, const min::vec2<float> &scale, const min::vec4<float> &coord, const float angle)
    {
        // Calculate screen coordinates
        const auto ps = to_screen(p, scale);

        // Add matrix to the matrix buffer
        _v[index] = min::mat3<float>(ps.first, min::mat2<float>(angle));
        _v[index] *= min::mat3<float>().set_scale(ps.second);

        // Set uv coordinates
        set_uv(index, coord);
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
        const min::vec2<float> scale(_s_bg, _s_bg);
        const min::vec4<float> black_coord(_x_black_uv, _y_black_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(index, p, scale, black_coord);
    }
    inline void load_background_red(const size_t index)
    {
        const min::vec2<float> p = toolbar_position(index);
        const min::vec2<float> scale(_s_bg, _s_bg);
        const min::vec4<float> red_coord(_x_red_uv, _y_red_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(index, p, scale, red_coord);
    }
    inline void load_background_yellow(const size_t index)
    {
        const min::vec2<float> p = toolbar_position(index);
        const min::vec2<float> scale(_s_bg, _s_bg);
        const min::vec4<float> yellow_coord(_x_yellow_uv, _y_yellow_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(index, p, scale, yellow_coord);
    }
    inline void load_fps_cursor()
    {
        const min::vec2<float> p(_center_w, _center_h);
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> fps_coord(_x_cursor_uv, _y_cursor_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect_reset(8, p, scale, fps_coord);
    }
    inline void load_reload_cursor()
    {
        const min::vec2<float> p(_center_w, _center_h);
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> reload_coord(_x_reload_uv, _y_reload_uv, _s_uv, _s_uv);

        // Rotate rect by angle
        _cursor_angle -= 4.0;
        if (_cursor_angle > 180.0)
        {
            _cursor_angle -= 180.0;
        }

        // Load rect at position
        set_rect_rot(8, p, scale, reload_coord, _cursor_angle);
    }
    inline void load_energy_meter()
    {
        const float y_height = _s_blue_y * _energy;
        const float y_offset = (y_height - _s_blue_x) * 0.5 + _tool_height;
        const min::vec2<float> p(_center_w + _energy_start, y_offset);
        const min::vec2<float> scale(_s_blue_x, y_height);
        const min::vec4<float> blue_coord(_x_blue_uv, _y_blue_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(9, p, scale, blue_coord);
    }
    inline void load_health_meter()
    {
        const float y_height = _s_red_y * _health;
        const float y_offset = (y_height - _s_red_x) * 0.5 + _tool_height;
        const min::vec2<float> p(_center_w + _health_start, y_offset);
        const min::vec2<float> scale(_s_red_x, y_height);
        const min::vec4<float> red_coord(_x_red_uv, _y_red_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(10, p, scale, red_coord);
    }
    inline void load_beam_icon(const size_t index)
    {
        const min::vec2<float> p = toolbar_position(index);
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> beam_coord(_x_beam_uv, _y_beam_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(11, p, scale, beam_coord);
    }
    inline void load_missile_icon(const size_t index)
    {
        const min::vec2<float> p = toolbar_position(index);
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> miss_coord(_x_miss_uv, _y_miss_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(12, p, scale, miss_coord);
    }
    inline void load_grapple_icon(const size_t index)
    {
        const min::vec2<float> p = toolbar_position(index);
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> grap_coord(_x_grap_uv, _y_grap_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(13, p, scale, grap_coord);
    }
    inline void load_jet_icon(const size_t index)
    {
        const min::vec2<float> p = toolbar_position(index);
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> jet_coord(_x_jet_uv, _y_jet_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(14, p, scale, jet_coord);
    }
    inline void load_menu_dead()
    {
        const min::vec2<float> p(_center_w, _center_h);
        const min::vec2<float> scale(_s_menu_x, _s_menu_y);
        const min::vec4<float> pause_coord(_x_dead_uv, _y_dead_uv, _s_menu_uv_x, _s_menu_uv_y);

        // Load rect at position
        set_rect(15, p, scale, pause_coord);
    }
    inline void load_menu_pause()
    {
        const min::vec2<float> p(_center_w, _center_h);
        const min::vec2<float> scale(_s_menu_x, _s_menu_y);
        const min::vec4<float> pause_coord(_x_pause_uv, _y_pause_uv, _s_menu_uv_x, _s_menu_uv_y);

        // Load rect at position
        set_rect(15, p, scale, pause_coord);
    }
    inline void position_ui()
    {
        // Add 8 black rectangles along bottom
        for (size_t i = 0; i < 8; i++)
        {
            set_key_up(i);
        }

        // Add FPS cursor
        load_fps_cursor();

        // Add Health meter
        load_energy_meter();

        // Add Health meter
        load_health_meter();

        // Load pause text
        load_menu_pause();
    }

  public:
    ui_overlay(const game::uniforms &uniforms, const uint16_t width, const uint16_t height)
        : _vertex("data/shader/ui.vertex", GL_VERTEX_SHADER),
          _fragment("data/shader/ui.fragment", GL_FRAGMENT_SHADER),
          _prog(_vertex, _fragment),
          _width(width), _height(height),
          _center_w(width / 2), _center_h(height / 2),
          _index(0), _selected(0), _menu_offset(15),
          _energy(0.0), _health(1.0), _cursor_angle(0.0), _draw_menu(false)
    {
        // Create the instance rectangle
        load_base_rect();

        // Load texture
        load_texture();

        // Load the uniform buffer with program we will use
        uniforms.set_program_matrix_only(_prog);

        // Add 15 ui rectangles
        for (size_t i = 0; i < _menu_offset; i++)
        {
            add_rect();
        }

        // Add 1 menu rectangle
        for (size_t i = 0; i < 1; i++)
        {
            add_rect();
        }

        // Reposition all ui on the screen
        position_ui();
    }
    inline size_t draw_size() const
    {
        if (_draw_menu)
        {
            return _v.size();
        }

        return _menu_offset;
    }
    inline void draw(game::uniforms &uniforms) const
    {
        const size_t size = draw_size();
        if (size > 0)
        {
            // Bind the text_buffer vao
            _vb.bind();

            // Bind the ui program
            _prog.use();

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
    inline void respawn()
    {
        // Reset menu
        set_menu_pause();

        // Turn off showing menu
        _draw_menu = false;
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
        // Set unselected color to black
        load_background_black(_selected);

        // Set selected index
        _selected = index;

        // Set selected color to yellow
        load_background_yellow(index);
    }
    inline void set_key_down_fail(const size_t index)
    {
        load_background_red(index);
    }
    inline void set_key_up(const size_t index)
    {
        // Set correct background if selected
        if (index == _selected)
        {
            load_background_yellow(index);
        }
        else
        {
            load_background_black(index);
        }

        // Draw key overlay
        switch (index)
        {
        case 0:
            return load_beam_icon(index);
        case 1:
            return load_missile_icon(index);
        case 2:
            return load_grapple_icon(index);
        case 3:
            return load_jet_icon(index);
        }
    }
    inline void set_menu_draw(const bool flag)
    {
        _draw_menu = flag;
    }
    inline void set_menu_dead()
    {
        load_menu_dead();
    }
    inline void set_menu_pause()
    {
        load_menu_pause();
    }
    inline void set_reload_cursor()
    {
        load_reload_cursor();
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
    inline void set_target_cursor()
    {
        load_fps_cursor();
    }
};
}

#endif
