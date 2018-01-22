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
#ifndef _UI_BACKGROUND_ASSETS__
#define _UI_BACKGROUND_ASSETS__

#include <stdexcept>
#include <utility>
#include <vector>

namespace game
{

class ui_bg_assets
{
  private:
    // Backgrounds
    static constexpr size_t _max_size = 18;
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
    static constexpr float _x_white_uv = 184.0 / 512.0;
    static constexpr float _y_white_uv = 4.0 / 512.0;

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
    static constexpr float _x_scan_uv = 184.0 / 512.0;
    static constexpr float _y_scan_uv = 40.0 / 512.0;

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

    // Console sizes
    static constexpr float _s_console_x = 400.0;
    static constexpr float _s_console_y = 40.0;

    // Placement values
    static constexpr size_t _num_buttons = 8;
    static constexpr size_t _num_half_buttons = _num_buttons / 2;
    static constexpr float _tool_height = 48.0;
    static constexpr float _tool_space = 48.0;
    static constexpr float _tool_start = -_tool_space * _num_half_buttons + _tool_space / 2;
    static constexpr float _energy_start = _tool_space * _num_half_buttons + _tool_space / 2 + 4.0;
    static constexpr float _health_start = _tool_start - _tool_space - 4.0;
    static constexpr float _y_console = 100.0;

    // Index stuff
    std::vector<min::mat3<float>> _v;
    std::vector<min::mat3<float>> _uv;

    // Screen properties
    size_t _width;
    size_t _height;
    size_t _center_w;
    size_t _center_h;
    float _energy;
    float _health;
    float _cursor_angle;
    bool _draw_menu;
    bool _draw_console;
    bool _draw_title;

    inline size_t add_rect()
    {
        // Check for buffer overflow
        if (_v.size() == 20)
        {
            throw std::runtime_error("ui_bg_assets: must change default ui count");
        }

        // Add matrix to the matrix buffer
        _v.emplace_back();

        // Add uv matrix to buffer
        _uv.emplace_back();

        // return the index
        return _v.size() - 1;
    }
    inline void set_uv(const size_t index, const min::vec4<float> &coord, const float alpha)
    {
        // Add uv matrix to buffer
        const float sx = coord.z();
        const float sy = coord.w();
        _uv[index].set_scale(min::vec2<float>(sx, sy));
        _uv[index].set_translation(min::vec2<float>(coord.x(), coord.y()));
        _uv[index].w(alpha);
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
    inline void set_rect(const size_t index, const min::vec2<float> &p,
                         const min::vec2<float> &scale, const min::vec4<float> &coord,
                         const float alpha = 1.0)
    {
        // Calculate screen coordinates
        const auto ps = to_screen(p, scale);

        // Set matrix components
        _v[index].set_translation(ps.first);
        _v[index].set_scale(ps.second);

        // Set uv coordinates
        set_uv(index, coord, alpha);
    }
    inline void set_rect_reset(const size_t index, const min::vec2<float> &p,
                               const min::vec2<float> &scale, const min::vec4<float> &coord,
                               const float alpha = 1.0)
    {
        // Calculate screen coordinates
        const auto ps = to_screen(p, scale);

        // Add matrix to the matrix buffer
        _v[index] = min::mat3<float>(ps.first);
        _v[index].set_scale(ps.second);

        // Set uv coordinates
        set_uv(index, coord, alpha);
    }
    inline void set_rect_rot(const size_t index, const min::vec2<float> &p,
                             const min::vec2<float> &scale, const min::vec4<float> &coord,
                             const float angle, const float alpha = 1.0)
    {
        // Calculate screen coordinates
        const auto ps = to_screen(p, scale);

        // Add matrix to the matrix buffer
        _v[index] = min::mat3<float>(ps.first, min::mat2<float>(angle));
        _v[index] *= min::mat3<float>().set_scale(ps.second);

        // Set uv coordinates
        set_uv(index, coord, alpha);
    }
    inline min::vec2<float> toolbar_position(const size_t index)
    {
        // Calculate offset from center for this toolbar element
        const float offset = _tool_start + index * _tool_space;

        // Return toolbar position
        return min::vec2<float>(_center_w + offset, _tool_height);
    }
    inline void reserve_memory()
    {
        // Reserve space for number of menu items
        _v.reserve(_max_size);
        _uv.reserve(_max_size);
    }

  public:
    ui_bg_assets(const uint16_t width, const uint16_t height)
        : _width(width), _height(height),
          _center_w(width / 2), _center_h(height / 2),
          _energy(0.0), _health(1.0), _cursor_angle(0.0),
          _draw_menu(false), _draw_console(false), _draw_title(true)
    {
        // Reserve memory
        reserve_memory();

        // Add 18 ui rectangles
        for (size_t i = 0; i < _max_size; i++)
        {
            add_rect();
        }
    }
    inline bool get_draw_console() const
    {
        return _draw_console;
    }
    inline bool get_draw_menu() const
    {
        return _draw_menu;
    }
    inline bool get_draw_title() const
    {
        return _draw_title;
    }
    inline const std::vector<min::mat3<float>> &get_scale() const
    {
        return _v;
    }
    inline size_t get_title_offset() const
    {
        return 1;
    }
    inline const std::vector<min::mat3<float>> &get_uv() const
    {
        return _uv;
    }
    inline void load_title_overlay()
    {
        const min::vec2<float> p(_center_w, _center_h);
        const min::vec2<float> scale(_width, _height);
        const min::vec4<float> full_coord(0.0, 0.0, 1.0, 1.0);

        // Load rect at position
        set_rect(0, p, scale, full_coord);
    }
    inline void load_health_overlay()
    {
        const min::vec2<float> p(_center_w, _center_h);
        const min::vec2<float> scale(_width, _height);
        const min::vec4<float> red_coord(_x_red_uv, _y_red_uv, _s_uv, _s_uv);

        // Load rect at position
        const float alpha = (_health > 0.0) ? 0.85 - _health * 0.85 : 0.85;
        set_rect(0, p, scale, red_coord, alpha);
    }
    inline void load_console_bg()
    {
        const min::vec2<float> p(_center_w, _y_console);
        const min::vec2<float> scale(_s_console_x, _s_console_y);
        const min::vec4<float> black_coord(_x_black_uv, _y_black_uv, _s_uv, _s_uv);

        // Load rect at position
        const float alpha = (_draw_console) ? 0.5 : 0.0;
        set_rect(1, p, scale, black_coord, alpha);
    }
    inline void load_menu_dead()
    {
        const min::vec2<float> p(_center_w, _center_h);
        const min::vec2<float> scale(_s_menu_x, _s_menu_y);
        const min::vec4<float> pause_coord(_x_dead_uv, _y_dead_uv, _s_menu_uv_x, _s_menu_uv_y);

        // Load rect at position
        set_rect(2, p, scale, pause_coord);
    }
    inline void load_menu_pause()
    {
        const min::vec2<float> p(_center_w, _center_h);
        const min::vec2<float> scale(_s_menu_x, _s_menu_y);
        const min::vec4<float> pause_coord(_x_pause_uv, _y_pause_uv, _s_menu_uv_x, _s_menu_uv_y);

        // Load rect at position
        set_rect(2, p, scale, pause_coord);
    }
    inline void load_cursor_fps()
    {
        const min::vec2<float> p(_center_w, _center_h);
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> fps_coord(_x_cursor_uv, _y_cursor_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect_reset(2, p, scale, fps_coord);
    }
    inline void load_cursor_reload()
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
        set_rect_rot(2, p, scale, reload_coord, _cursor_angle);
    }
    inline void load_energy_meter()
    {
        const float y_height = _s_blue_y * _energy;
        const float y_offset = (y_height - _s_blue_x) * 0.5 + _tool_height;
        const min::vec2<float> p(_center_w + _energy_start, y_offset);
        const min::vec2<float> scale(_s_blue_x, y_height);
        const min::vec4<float> blue_coord(_x_blue_uv, _y_blue_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(3, p, scale, blue_coord);
    }
    inline void load_health_meter()
    {
        const float y_height = _s_red_y * _health;
        const float y_offset = (y_height - _s_red_x) * 0.5 + _tool_height;
        const min::vec2<float> p(_center_w + _health_start, y_offset);
        const min::vec2<float> scale(_s_red_x, y_height);
        const min::vec4<float> red_coord(_x_red_uv, _y_red_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(4, p, scale, red_coord);
    }
    inline void load_background_black(const size_t index)
    {
        const min::vec2<float> p = toolbar_position(index);
        const min::vec2<float> scale(_s_bg, _s_bg);
        const min::vec4<float> black_coord(_x_black_uv, _y_black_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(index + 5, p, scale, black_coord);
    }
    inline void load_background_red(const size_t index)
    {
        const min::vec2<float> p = toolbar_position(index);
        const min::vec2<float> scale(_s_bg, _s_bg);
        const min::vec4<float> red_coord(_x_red_uv, _y_red_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(index + 5, p, scale, red_coord);
    }
    inline void load_background_yellow(const size_t index)
    {
        const min::vec2<float> p = toolbar_position(index);
        const min::vec2<float> scale(_s_bg, _s_bg);
        const min::vec4<float> yellow_coord(_x_yellow_uv, _y_yellow_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(index + 5, p, scale, yellow_coord);
    }
    inline void load_background_white(const size_t index)
    {
        const min::vec2<float> p = toolbar_position(index);
        const min::vec2<float> scale(_s_bg, _s_bg);
        const min::vec4<float> white_coord(_x_white_uv, _y_white_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(index + 5, p, scale, white_coord);
    }
    inline void load_beam_icon(const size_t index)
    {
        const min::vec2<float> p = toolbar_position(index);
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> beam_coord(_x_beam_uv, _y_beam_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(13, p, scale, beam_coord);
    }
    inline void load_missile_icon(const size_t index)
    {
        const min::vec2<float> p = toolbar_position(index);
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> miss_coord(_x_miss_uv, _y_miss_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(14, p, scale, miss_coord);
    }
    inline void load_grapple_icon(const size_t index)
    {
        const min::vec2<float> p = toolbar_position(index);
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> grap_coord(_x_grap_uv, _y_grap_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(15, p, scale, grap_coord);
    }
    inline void load_jet_icon(const size_t index)
    {
        const min::vec2<float> p = toolbar_position(index);
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> jet_coord(_x_jet_uv, _y_jet_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(16, p, scale, jet_coord);
    }
    inline void load_scan_icon(const size_t index)
    {
        const min::vec2<float> p = toolbar_position(index);
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> beam_coord(_x_scan_uv, _y_scan_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(17, p, scale, beam_coord);
    }
    inline void set_draw_console(const bool flag)
    {
        _draw_console = flag;
    }
    inline void set_draw_menu(const bool flag)
    {
        _draw_menu = flag;
    }
    inline void set_draw_title(const bool flag)
    {
        _draw_title = flag;
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

        // Set the overlay alpha
        load_health_overlay();

        // Set the size of the health bar
        load_health_meter();
    }
    inline void set_screen(const float width, const float height)
    {
        // Update the screen dimensions
        _width = width;
        _height = height;

        // Update screen center
        _center_w = _width / 2;
        _center_h = _height / 2;
    }
    inline size_t size() const
    {
        return _v.size();
    }
    inline void toggle_draw_console()
    {
        _draw_console = !_draw_console;
    }
};
}

#endif
