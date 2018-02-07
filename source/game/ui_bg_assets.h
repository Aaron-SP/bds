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
#ifndef _UI_BACKGROUND_ASSETS__
#define _UI_BACKGROUND_ASSETS__

#include <game/inventory.h>
#include <min/aabbox.h>
#include <min/vec2.h>
#include <stdexcept>
#include <utility>
#include <vector>

namespace game
{

class ui_bg_assets
{
  private:
    // Backgrounds
    static constexpr float _image_size = 512.0;
    static constexpr float _next_icon = 36.0 / _image_size;
    static constexpr float _x_cursor_uv = 4.0 / _image_size;
    static constexpr float _y_cursor_uv = 4.0 / _image_size;
    static constexpr float _x_tar_cursor_uv = 40.0 / _image_size;
    static constexpr float _y_tar_cursor_uv = 4.0 / _image_size;
    static constexpr float _x_black_uv = 76.0 / _image_size;
    static constexpr float _y_black_uv = 4.0 / _image_size;
    static constexpr float _x_yellow_uv = 112.0 / _image_size;
    static constexpr float _y_yellow_uv = 4.0 / _image_size;
    static constexpr float _x_red_uv = 148.0 / _image_size;
    static constexpr float _y_red_uv = 4.0 / _image_size;
    static constexpr float _x_blue_uv = 184.0 / _image_size;
    static constexpr float _y_blue_uv = 4.0 / _image_size;
    static constexpr float _x_white_uv = 220.0 / _image_size;
    static constexpr float _y_white_uv = 4.0 / _image_size;
    static constexpr float _x_light_blue_uv = 256.0 / _image_size;
    static constexpr float _y_light_blue_uv = 4.0 / _image_size;

    // Icons
    static constexpr float _x_reload_uv = 4.0 / _image_size;
    static constexpr float _y_reload_uv = 40.0 / _image_size;
    static constexpr float _x_beam_uv = 40.0 / _image_size;
    static constexpr float _y_beam_uv = 40.0 / _image_size;
    static constexpr float _x_miss_uv = 76.0 / _image_size;
    static constexpr float _y_miss_uv = 40.0 / _image_size;
    static constexpr float _x_grap_uv = 112.0 / _image_size;
    static constexpr float _y_grap_uv = 40.0 / _image_size;
    static constexpr float _x_jet_uv = 148.0 / _image_size;
    static constexpr float _y_jet_uv = 40.0 / _image_size;
    static constexpr float _x_scan_uv = 184.0 / _image_size;
    static constexpr float _y_scan_uv = 40.0 / _image_size;

    // Cubes
    static constexpr float _x_cube_uv = 4.0 / _image_size;
    static constexpr float _y_cube_uv = 76.0 / _image_size;

    // Menu text
    static constexpr float _x_dead_uv = 4.0 / _image_size;
    static constexpr float _y_dead_uv = 384.0 / _image_size;
    static constexpr float _x_pause_uv = 4.0 / _image_size;
    static constexpr float _y_pause_uv = 256.0 / _image_size;

    // Scale sizes
    static constexpr float _s_bg = 40.0;
    static constexpr float _s_fg = 32.0;
    static constexpr float _s_uv = 32.0 / _image_size;
    static constexpr float _s_red_x = 32.0;
    static constexpr float _s_red_y = 96.0;
    static constexpr float _s_blue_x = 32.0;
    static constexpr float _s_blue_y = 96.0;

    // Inventory pixel scale
    static constexpr float _s_inv = 16.0;

    // Menu sizes
    static constexpr float _s_menu_x = 504.0;
    static constexpr float _s_menu_y = 124.0;
    static constexpr float _s_menu_uv_x = 504.0 / _image_size;
    static constexpr float _s_menu_uv_y = 124.0 / _image_size;

    // Console sizes
    static constexpr float _s_console_x = 400.0;
    static constexpr float _s_console_y = 40.0;

    // Placement values
    static constexpr size_t _num_buttons = 8;
    static constexpr size_t _num_half_buttons = _num_buttons / 2;
    static constexpr float _store_height = 48.0;
    static constexpr float _tool_height = 48.0;
    static constexpr float _tool_space = 48.0;
    static constexpr float _tool_start = -_tool_space * _num_half_buttons + _tool_space / 2;
    static constexpr float _energy_start = _tool_space * _num_half_buttons + _tool_space / 2 + 4.0;
    static constexpr float _health_start = _tool_start - _tool_space - 4.0;
    static constexpr float _y_console = 100.0;

    // Number of ui elements, 3 + 2 + 16 + 16 + 24 + 24
    static constexpr size_t _base_size = 37;
    static constexpr size_t _extend_size = 85;
    static constexpr size_t _max_size = _extend_size;

    // Rect Instance stuff
    std::vector<min::mat3<float>> _v;
    std::vector<min::mat3<float>> _uv;

    // Screen properties
    uint16_t _width;
    uint16_t _height;
    uint16_t _center_w;
    uint16_t _center_h;
    float _energy;
    float _health;
    float _cursor_angle;
    bool _draw_console;
    bool _draw_ex;
    uint8_t _draw_menu;
    bool _draw_title;

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

        // Convert to screen coordinates
        const float ox = p.x() * sx - 1.0;
        const float oy = p.y() * sy - 1.0;

        // Return screen coordinates and sizes
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

  public:
    ui_bg_assets(const uint16_t width, const uint16_t height)
        : _v(_max_size), _uv(_max_size),
          _width(width), _height(height),
          _center_w(width / 2), _center_h(height / 2),
          _energy(0.0), _health(1.0), _cursor_angle(0.0),
          _draw_console(false), _draw_ex(false), _draw_menu(0), _draw_title(true) {}

    inline bool get_draw_console() const
    {
        return _draw_console;
    }
    inline bool get_draw_dead() const
    {
        return _draw_menu == 3;
    }
    inline bool get_draw_ex() const
    {
        return _draw_ex;
    }
    inline bool get_draw_menu() const
    {
        return _draw_menu >= 3;
    }
    inline bool get_draw_pause() const
    {
        return _draw_menu == 4;
    }
    inline bool get_draw_reload() const
    {
        return _draw_menu == 1;
    }
    inline bool get_draw_target() const
    {
        return _draw_menu == 2;
    }
    inline bool get_draw_title() const
    {
        return _draw_title;
    }
    inline uint16_t get_width() const
    {
        return _width;
    }
    inline uint16_t get_height() const
    {
        return _height;
    }
    inline const std::vector<min::mat3<float>> &get_scale() const
    {
        return _v;
    }
    inline const std::vector<min::mat3<float>> &get_uv() const
    {
        return _uv;
    }
    inline static min::aabbox<float, min::vec2> inv_box(const min::vec2<float> &p)
    {
        // Create a box from the screen
        const min::vec2<float> half(_s_inv, _s_inv);

        // Return the inv_box
        return min::aabbox<float, min::vec2>(p - half, p + half);
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
        set_rect_reset(2, p, scale, pause_coord);
    }
    inline void load_menu_pause()
    {
        const min::vec2<float> p(_center_w, _center_h);
        const min::vec2<float> scale(_s_menu_x, _s_menu_y);
        const min::vec4<float> pause_coord(_x_pause_uv, _y_pause_uv, _s_menu_uv_x, _s_menu_uv_y);

        // Load rect at position
        set_rect_reset(2, p, scale, pause_coord);
    }
    inline void load_cursor_aim()
    {
        const min::vec2<float> p(_center_w, _center_h);
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> aim_coord(_x_cursor_uv, _y_cursor_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect_reset(2, p, scale, aim_coord);
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
    inline void load_cursor_target()
    {
        const min::vec2<float> p(_center_w, _center_h);
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> target_coord(_x_tar_cursor_uv, _y_tar_cursor_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect_reset(2, p, scale, target_coord);
    }
    inline void load_health_meter()
    {
        const float y_height = _s_red_y * _health;
        const float y_offset = (y_height - _s_red_x) * 0.5 + _tool_height;
        const min::vec2<float> p(_center_w + _health_start, y_offset);
        const min::vec2<float> scale(_s_red_x, y_height);
        const min::vec4<float> red_coord(_x_red_uv, _y_red_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(3, p, scale, red_coord);
    }
    inline void load_energy_meter()
    {
        const float y_height = _s_blue_y * _energy;
        const float y_offset = (y_height - _s_blue_x) * 0.5 + _tool_height;
        const min::vec2<float> p(_center_w + _energy_start, y_offset);
        const min::vec2<float> scale(_s_blue_x, y_height);
        const min::vec4<float> blue_coord(_x_blue_uv, _y_blue_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(4, p, scale, blue_coord);
    }
    inline void load_bg_black(const inv_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_bg, _s_bg);
        const min::vec4<float> black_coord(_x_black_uv, _y_black_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, black_coord);
    }
    inline void load_bg_red(const inv_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_bg, _s_bg);
        const min::vec4<float> red_coord(_x_red_uv, _y_red_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, red_coord);
    }
    inline void load_bg_yellow(const inv_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_bg, _s_bg);
        const min::vec4<float> yellow_coord(_x_yellow_uv, _y_yellow_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, yellow_coord);
    }
    inline void load_bg_white(const inv_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_bg, _s_bg);
        const min::vec4<float> white_coord(_x_white_uv, _y_white_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, white_coord);
    }
    inline void load_bg_light_blue(const inv_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_bg, _s_bg);
        const min::vec4<float> white_coord(_x_light_blue_uv, _y_light_blue_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, white_coord);
    }
    inline void load_beam_icon(const inv_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> beam_coord(_x_beam_uv, _y_beam_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, beam_coord);
    }
    inline void load_cube_icon(const inv_id id, const int8_t atlas_id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_fg, _s_fg);

        // Calculate the start of cube uv in grid
        const float x = _x_cube_uv + (atlas_id % 8) * _next_icon;
        const float y = _y_cube_uv + (atlas_id / 8) * _next_icon;

        // Calculate cube uv coordinates
        const min::vec4<float> beam_coord(x, y, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, beam_coord);
    }
    inline void load_grapple_icon(const inv_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> grap_coord(_x_grap_uv, _y_grap_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, grap_coord);
    }
    inline void load_jet_icon(const inv_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> jet_coord(_x_jet_uv, _y_jet_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, jet_coord);
    }
    inline void load_missile_icon(const inv_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> miss_coord(_x_miss_uv, _y_miss_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, miss_coord);
    }
    inline void load_scan_icon(const inv_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> beam_coord(_x_scan_uv, _y_scan_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, beam_coord);
    }
    inline void set_draw_console(const bool flag)
    {
        _draw_console = flag;
    }
    inline void set_draw_dead()
    {
        _draw_menu = 3;
    }
    inline void set_draw_aim()
    {
        _draw_menu = 0;
    }
    inline void set_draw_pause()
    {
        _draw_menu = 4;
    }
    inline void set_draw_reload()
    {
        _draw_menu = 1;
    }
    inline void set_draw_target()
    {
        _draw_menu = 2;
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
    inline void set_screen(const uint16_t width, const uint16_t height)
    {
        // Update the screen dimensions
        _width = width;
        _height = height;

        // Update screen center
        _center_w = _width / 2;
        _center_h = _height / 2;
    }
    inline void toggle_draw_console()
    {
        _draw_console = !_draw_console;
    }
    inline void toggle_draw_ex()
    {
        _draw_ex = !_draw_ex;
    }
    inline min::vec2<float> store_position(const size_t row, const size_t col) const
    {
        // Calculate offset from center for this toolbar element
        const float x = (_center_w + _tool_start) + (col * _tool_space);
        const float y = _height - _store_height;

        // Return toolbar position
        return min::vec2<float>(x, y);
    }
    inline min::vec2<float> toolbar_position(const size_t row, const size_t col) const
    {
        // Calculate offset from center for this toolbar element
        const float x = (_center_w + _tool_start) + (col * _tool_space);
        const float y = _tool_height + (row * _tool_space);

        // Return toolbar position
        return min::vec2<float>(x, y);
    }
    inline static constexpr size_t opaque_extend_size()
    {
        return _extend_size - opaque_start();
    }
    inline static constexpr size_t opaque_base_size()
    {
        return ui_size() - opaque_start();
    }
    inline static constexpr size_t opaque_start()
    {
        return 3;
    }
    inline static constexpr size_t transparent_start()
    {
        return 0;
    }
    inline static constexpr size_t transparent_size()
    {
        return 3;
    }
    inline static constexpr size_t ui_size()
    {
        // 3 + 2 + 16 + 16
        return _base_size;
    }
};
}

#endif
