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

#include <algorithm>
#include <game/inventory.h>
#include <game/ui_config.h>
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
    static constexpr float _image_size = 1024.0;
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
    static constexpr float _x_grey_uv = 292.0 / _image_size;
    static constexpr float _y_grey_uv = 4.0 / _image_size;
    static constexpr float _x_hover_stat_uv = 328.0 / _image_size;
    static constexpr float _y_hover_stat_uv = 4.0 / _image_size;
    static constexpr float _x_click_stat_uv = 344.0 / _image_size;
    static constexpr float _y_click_stat_uv = 4.0 / _image_size;
    static constexpr float _x_grey_stat_uv = 328.0 / _image_size;
    static constexpr float _y_grey_stat_uv = 20.0 / _image_size;
    static constexpr float _x_red_stat_uv = 344.0 / _image_size;
    static constexpr float _y_red_stat_uv = 20.0 / _image_size;

    // Icons
    static constexpr float _x_reload_uv = 4.0 / _image_size;
    static constexpr float _y_reload_uv = 40.0 / _image_size;
    static constexpr float _x_auto_uv = 40.0 / _image_size;
    static constexpr float _y_auto_uv = 40.0 / _image_size;
    static constexpr float _x_beam_uv = 76.0 / _image_size;
    static constexpr float _y_beam_uv = 40.0 / _image_size;
    static constexpr float _x_charge_uv = 112.0 / _image_size;
    static constexpr float _y_charge_uv = 40.0 / _image_size;
    static constexpr float _x_grap_uv = 148.0 / _image_size;
    static constexpr float _y_grap_uv = 40.0 / _image_size;
    static constexpr float _x_grenade_uv = 184.0 / _image_size;
    static constexpr float _y_grenade_uv = 40.0 / _image_size;
    static constexpr float _x_jet_uv = 220.0 / _image_size;
    static constexpr float _y_jet_uv = 40.0 / _image_size;
    static constexpr float _x_miss_uv = 256.0 / _image_size;
    static constexpr float _y_miss_uv = 40.0 / _image_size;
    static constexpr float _x_portal_uv = 292.0 / _image_size;
    static constexpr float _y_portal_uv = 40.0 / _image_size;
    static constexpr float _x_scan_uv = 328.0 / _image_size;
    static constexpr float _y_scan_uv = 40.0 / _image_size;
    static constexpr float _x_scatter_uv = 364.0 / _image_size;
    static constexpr float _y_scatter_uv = 40.0 / _image_size;
    static constexpr float _x_speed_uv = 400.0 / _image_size;
    static constexpr float _y_speed_uv = 40.0 / _image_size;

    // Cubes
    static constexpr float _x_block_uv = 4.0 / _image_size;
    static constexpr float _y_block_uv = 76.0 / _image_size;
    static constexpr float _x_item_uv = 328.0 / _image_size;
    static constexpr float _y_item_uv = 76.0 / _image_size;

    // Menu text
    static constexpr float _x_dead_uv = 4.0 / _image_size;
    static constexpr float _y_dead_uv = 896.0 / _image_size;
    static constexpr float _x_pause_uv = 4.0 / _image_size;
    static constexpr float _y_pause_uv = 768.0 / _image_size;
    static constexpr float _x_focus_uv = 688.0 / _image_size;
    static constexpr float _y_focus_uv = 768.0 / _image_size;
    static constexpr float _x_stat_uv = 4.0 / _image_size;
    static constexpr float _y_stat_uv = 498.0 / _image_size;
    static constexpr float _x_hover_uv = 688.0 / _image_size;
    static constexpr float _y_hover_uv = 574.0 / _image_size;
    static constexpr float _x_ui_bar_uv = 4.0 / _image_size;
    static constexpr float _y_ui_bar_uv = 380.0 / _image_size;

    // Scale sizes
    static constexpr float _s_suv = 16.0 / _image_size;
    static constexpr float _s_uv = 32.0 / _image_size;
    static constexpr float _s_health_x = 48.0;
    static constexpr float _s_health_y = 96.0;
    static constexpr float _s_energy_x = 48.0;
    static constexpr float _s_energy_y = 96.0;
    static constexpr float _s_focus_uv_x = _s_focus_x / _image_size;
    static constexpr float _s_focus_uv_y = _s_focus_y / _image_size;
    static constexpr float _s_stat_x = 680.0;
    static constexpr float _s_stat_y = 266.0;
    static constexpr float _s_stat_uv_x = _s_stat_x / _image_size;
    static constexpr float _s_stat_uv_y = _s_stat_y / _image_size;
    static constexpr float _s_hover_uv_x = _s_hover_bg_x / _image_size;
    static constexpr float _s_hover_uv_y = _s_hover_bg_y / _image_size;
    static constexpr float _s_ui_bar_x = 62.0;
    static constexpr float _s_ui_bar_y = 110.0;
    static constexpr float _s_ui_bar_uv_x = 62.0 / _image_size;
    static constexpr float _s_ui_bar_uv_y = 110.0 / _image_size;

    // Menu sizes
    static constexpr float _s_menu_x = 504.0;
    static constexpr float _s_menu_y = 124.0;
    static constexpr float _s_menu_uv_x = 504.0 / _image_size;
    static constexpr float _s_menu_uv_y = 124.0 / _image_size;

    // Number of ui elements, 5 + 4 + 16 + 16 + 24 + 24 + 1 + 9 + 9 + 6 + 2 + 1
    static constexpr size_t _base = 41;
    static constexpr size_t _focus = 114;
    static constexpr size_t _ext_hover = _focus + 2;
    static constexpr size_t _max_size = _ext_hover + 1;

    // Rect Instance stuff
    std::vector<min::mat3<float>> _v;
    std::vector<min::mat3<float>> _uv;

    // Screen properties
    uint16_t _width;
    uint16_t _height;
    uint16_t _center_w;
    uint16_t _center_h;
    float _energy;
    float _exp;
    float _health;
    float _oxy;
    float _focus_bar;
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
          _energy(0.0), _exp(0.0), _health(1.0), _oxy(1.0), _focus_bar(1.0), _cursor_angle(0.0),
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
    inline bool get_focus_bar() const
    {
        return _focus_bar > 0.0;
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
    inline static min::aabbox<float, min::vec2> stat_box(const min::vec2<float> &p)
    {
        // Create a box from the screen
        const min::vec2<float> half(_s_stat, _s_stat);

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
        const min::vec2<float> p(_center_w, _console_dy);
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
    inline void load_health_bar()
    {
        const min::vec2<float> p(_center_w + _health_dx, _bar_dy + _s_ui_bar_y * 0.5);
        const min::vec2<float> scale(_s_ui_bar_x, _s_ui_bar_y);
        const min::vec4<float> bar_coord(_x_ui_bar_uv, _y_ui_bar_uv, _s_ui_bar_uv_x, _s_ui_bar_uv_y);

        // Load rect at position
        set_rect_reset(3, p, scale, bar_coord);
    }
    inline void load_energy_bar()
    {
        const min::vec2<float> p(_center_w + _energy_dx, _bar_dy + _s_ui_bar_y * 0.5);
        const min::vec2<float> scale(_s_ui_bar_x, _s_ui_bar_y);
        const min::vec4<float> bar_coord(_x_ui_bar_uv, _y_ui_bar_uv, _s_ui_bar_uv_x, _s_ui_bar_uv_y);

        // Load rect at position
        set_rect_reset(4, p, scale, bar_coord);
    }
    inline void load_health_meter()
    {
        const float health = std::min<float>(1.0, _health);
        const float y_height = _s_health_y * health;
        const float y_offset = _meter_dy + (y_height - _s_health_x) * 0.5;
        const min::vec2<float> p(_center_w + _health_dx, y_offset);
        const min::vec2<float> scale(_s_health_x, y_height);

        // Load rect at position depending on health
        if (_health > 1.0)
        {
            set_rect(5, p, scale, min::vec4<float>(_x_yellow_uv, _y_yellow_uv, _s_uv, _s_uv));
        }
        else
        {
            set_rect(5, p, scale, min::vec4<float>(_x_red_uv, _y_red_uv, _s_uv, _s_uv));
        }
    }
    inline void load_energy_meter()
    {
        const float energy = std::min<float>(1.0, _energy);
        const float y_height = _s_energy_y * energy;
        const float y_offset = _meter_dy + (y_height - _s_energy_x) * 0.5;
        const min::vec2<float> p(_center_w + _energy_dx, y_offset);
        const min::vec2<float> scale(_s_energy_x, y_height);
        const min::vec4<float> blue_coord(_x_blue_uv, _y_blue_uv, _s_uv, _s_uv);

        // Load rect at position depending on energy
        if (_energy > 1.0)
        {
            set_rect(6, p, scale, min::vec4<float>(_x_light_blue_uv, _y_light_blue_uv, _s_uv, _s_uv));
        }
        else
        {
            set_rect(6, p, scale, min::vec4<float>(_x_blue_uv, _y_blue_uv, _s_uv, _s_uv));
        }
    }
    inline void load_exp_meter()
    {
        const float x_width = _s_exp_x * _exp;
        const float x_offset = _center_w + (x_width - _s_exp_y) * 0.5 + _exp_dx;
        const min::vec2<float> p(x_offset, _exp_dy);
        const min::vec2<float> scale(x_width, _s_exp_y);

        // Offset texture to prevent blurring edges
        const float uv_off = 2.0 / _image_size;
        const float suv_off = 4.0 / _image_size;
        const float adj_uv = _s_uv - suv_off;
        const min::vec4<float> exp_coord(_x_yellow_uv + uv_off, _y_yellow_uv + uv_off, adj_uv, adj_uv);

        // Load rect at position
        set_rect(7, p, scale, exp_coord);
    }
    inline void load_oxy_meter()
    {
        const float x_width = _s_oxy_x * _oxy;
        const float x_offset = _center_w + (x_width - _s_oxy_y) * 0.5 + _oxy_dx;
        const min::vec2<float> p(x_offset, _oxy_dy);
        const min::vec2<float> scale(x_width, _s_oxy_y);

        // Offset texture to prevent blurring edges
        const float uv_off = 2.0 / _image_size;
        const float suv_off = 4.0 / _image_size;
        const float adj_uv = _s_uv - suv_off;

        // Load rect at position depending on energy
        if (_oxy > 0.25)
        {
            const min::vec4<float> oxy_coord(_x_light_blue_uv + uv_off, _y_light_blue_uv + uv_off, adj_uv, adj_uv);
            set_rect(8, p, scale, oxy_coord);
        }
        else
        {
            const min::vec4<float> oxy_coord(_x_red_uv + uv_off, _y_red_uv + uv_off, adj_uv, adj_uv);
            set_rect(8, p, scale, oxy_coord);
        }
    }
    inline void load_bg_stat()
    {
        // Load stat rect at position
        const min::vec2<float> p(_center_w + _stat_dx, _stat_dy);
        const min::vec2<float> stat_scale(_s_stat_x, _s_stat_y);
        const min::vec4<float> stat_coord(_x_stat_uv, _y_stat_uv, _s_stat_uv_x, _s_stat_uv_y);
        set_rect(89, p, stat_scale, stat_coord);
    }
    inline void load_bg_focus()
    {
        // Load focus rect at position
        const min::vec2<float> p(_center_w + _focus_dx, _height - _focus_dy);
        const min::vec2<float> focus_scale(_s_focus_x, _s_focus_y);
        const min::vec4<float> focus_coord(_x_focus_uv, _y_focus_uv, _s_focus_uv_x, _s_focus_uv_y);
        set_rect(114, p, focus_scale, focus_coord);
    }
    inline void load_focus_meter()
    {
        const float x_width = _s_focus_bar_x * _focus_bar;
        const float x_offset = _center_w + (x_width - _s_focus_bar_y) * 0.5 + _focus_bar_dx;
        const min::vec2<float> p(x_offset, _height - _focus_bar_dy);
        const min::vec2<float> scale(x_width, _s_focus_bar_y);

        // Offset texture to prevent blurring edges
        const float uv_off = 2.0 / _image_size;
        const float suv_off = 4.0 / _image_size;
        const float adj_uv = _s_uv - suv_off;

        // Load rect at position depending on focus
        if (_focus_bar > 0.5)
        {
            const min::vec4<float> meter_coord(_x_yellow_uv + uv_off, _y_yellow_uv + uv_off, adj_uv, adj_uv);
            set_rect(115, p, scale, meter_coord);
        }
        else
        {
            const min::vec4<float> meter_coord(_x_red_uv + uv_off, _y_red_uv + uv_off, adj_uv, adj_uv);
            set_rect(115, p, scale, meter_coord);
        }
    }
    inline void load_bg_hover(const min::vec2<float> &p)
    {
        // Load hover rect at position
        const min::vec2<float> scale(_s_hover_bg_x, _s_hover_bg_y);
        const min::vec4<float> coord(_x_hover_uv, _y_hover_uv, _s_hover_uv_x, _s_hover_uv_y);

        // Calculate hover y offset to avoid off screen issues
        const float hover_dy = (p.y() > _center_h) ? _s_hover_bg_y * -0.5 : _s_hover_bg_y * 0.5;

        // Offset position by half width of rect
        const min::vec2<float> off = min::vec2<float>(p.x() + _s_hover_bg_x * 0.5, p.y() + hover_dy);

        set_rect(116, off, scale, coord);
    }
    inline void load_bg_black(const ui_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_bg, _s_bg);
        const min::vec4<float> black_coord(_x_black_uv, _y_black_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, black_coord);
    }
    inline void load_bg_red(const ui_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_bg, _s_bg);
        const min::vec4<float> red_coord(_x_red_uv, _y_red_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, red_coord);
    }
    inline void load_bg_yellow(const ui_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_bg, _s_bg);
        const min::vec4<float> yellow_coord(_x_yellow_uv, _y_yellow_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, yellow_coord);
    }
    inline void load_bg_white(const ui_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_bg, _s_bg);
        const min::vec4<float> white_coord(_x_white_uv, _y_white_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, white_coord);
    }
    inline void load_bg_light_blue(const ui_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_bg, _s_bg);
        const min::vec4<float> white_coord(_x_light_blue_uv, _y_light_blue_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, white_coord);
    }
    inline void load_empty_icon(const ui_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> grey_coord(_x_grey_uv, _y_grey_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, grey_coord);
    }
    inline void load_block_icon(const ui_id id, const int8_t block_id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_fg, _s_fg);

        // Calculate the start of cube uv in grid
        const float x = _x_block_uv + (block_id & 7) * _next_icon;
        const float y = _y_block_uv + (block_id / 8) * _next_icon;

        // Calculate cube uv coordinates
        const min::vec4<float> beam_coord(x, y, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, beam_coord);
    }
    inline void load_item_icon(const ui_id id, const int8_t item_id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_fg, _s_fg);

        // Calculate the start of cube uv in grid
        const float x = _x_item_uv + (item_id & 15) * _next_icon;
        const float y = _y_item_uv + (item_id / 16) * _next_icon;

        // Calculate cube uv coordinates
        const min::vec4<float> beam_coord(x, y, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, beam_coord);
    }
    inline void load_auto_icon(const ui_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> auto_coord(_x_auto_uv, _y_auto_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, auto_coord);
    }
    inline void load_beam_icon(const ui_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> beam_coord(_x_beam_uv, _y_beam_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, beam_coord);
    }
    inline void load_charge_icon(const ui_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> charge_coord(_x_charge_uv, _y_charge_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, charge_coord);
    }
    inline void load_grapple_icon(const ui_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> grap_coord(_x_grap_uv, _y_grap_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, grap_coord);
    }
    inline void load_grenade_icon(const ui_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> gren_coord(_x_grenade_uv, _y_grenade_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, gren_coord);
    }
    inline void load_jet_icon(const ui_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> jet_coord(_x_jet_uv, _y_jet_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, jet_coord);
    }
    inline void load_missile_icon(const ui_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> miss_coord(_x_miss_uv, _y_miss_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, miss_coord);
    }
    inline void load_portal_icon(const ui_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> scan_coord(_x_portal_uv, _y_portal_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, scan_coord);
    }
    inline void load_scan_icon(const ui_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> scan_coord(_x_scan_uv, _y_scan_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, scan_coord);
    }
    inline void load_scatter_icon(const ui_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> scat_coord(_x_scatter_uv, _y_scatter_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, scat_coord);
    }
    inline void load_speed_icon(const ui_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_fg, _s_fg);
        const min::vec4<float> speed_coord(_x_speed_uv, _y_speed_uv, _s_uv, _s_uv);

        // Load rect at position
        set_rect(id.id(), p, scale, speed_coord);
    }
    inline void load_stat_click(const ui_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_sfg, _s_sfg);
        const min::vec4<float> stat_coord(_x_click_stat_uv, _y_click_stat_uv, _s_suv, _s_suv);

        // Load rect at position
        set_rect(id.id(), p, scale, stat_coord);
    }
    inline void load_stat_grey(const ui_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_sfg, _s_sfg);
        const min::vec4<float> stat_coord(_x_grey_stat_uv, _y_grey_stat_uv, _s_suv, _s_suv);

        // Load rect at position
        set_rect(id.id(), p, scale, stat_coord);
    }
    inline void load_stat_hover(const ui_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_sfg, _s_sfg);
        const min::vec4<float> stat_coord(_x_hover_stat_uv, _y_hover_stat_uv, _s_suv, _s_suv);

        // Load rect at position
        set_rect(id.id(), p, scale, stat_coord);
    }
    inline void load_stat_red(const ui_id id, const min::vec2<float> &p)
    {
        const min::vec2<float> scale(_s_sfg, _s_sfg);
        const min::vec4<float> stat_coord(_x_red_stat_uv, _y_red_stat_uv, _s_suv, _s_suv);

        // Load rect at position
        set_rect(id.id(), p, scale, stat_coord);
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

        // Set the size of the energy bar
        load_energy_meter();
    }
    inline void set_experience(const float exp)
    {
        // Set experience in percent
        _exp = exp;

        // Set the size of the exp bar
        load_exp_meter();
    }
    inline void set_focus_bar(const float bar)
    {
        _focus_bar = bar;

        // Set the size of the focus bar
        load_focus_meter();
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
    inline void set_oxygen(const float oxy)
    {
        // Set experience in percent
        _oxy = oxy;

        // Set the size of the oxygen bar
        load_oxy_meter();
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
    inline min::vec2<float> attr_position(const unsigned row, const unsigned size) const
    {
        // Calculate offset from center for this stat text element
        const float x = _center_w + _attr_text_dx;
        const float y = _attr_text_dy - (row * size);

        // Return toolbar position
        return min::vec2<float>(x, y);
    }
    inline min::vec2<float> button_position(const unsigned row, const unsigned col) const
    {
        // Calculate offset from center for this toolbar element
        const float x = (_center_w + _button_dx) + (col * _button_space);
        const float y = _button_dy - (row * _button_space);

        // Return toolbar position
        return min::vec2<float>(x, y);
    }
    inline min::vec2<float> cube_position(const unsigned row, const unsigned col) const
    {
        // Calculate offset from center for this toolbar element
        const float x = (_center_w + _cube_dx) + (col * _cube_space);
        const float y = _cube_dy + (row * _cube_space);

        // Return toolbar position
        return min::vec2<float>(x, y);
    }
    inline min::vec2<float> stat_position(const unsigned row, const unsigned size) const
    {
        // Calculate offset from center for this stat text element
        const float x = _center_w + _stat_text_dx;
        const float y = _stat_text_dy - (row * size);

        // Return toolbar position
        return min::vec2<float>(x, y);
    }
    inline min::vec2<float> store_position(const unsigned row, const unsigned col) const
    {
        // Calculate offset from center for this toolbar element
        const float x = (_center_w + _tool_start) + (col * _tool_space);
        const float y = _height - _store_dy;

        // Return toolbar position
        return min::vec2<float>(x, y);
    }
    inline min::vec2<float> toolbar_position(const unsigned row, const unsigned col) const
    {
        // Calculate offset from center for this toolbar element
        const float x = (_center_w + _tool_start) + (col * _tool_space);
        const float y = _tool_dy + (row * _tool_space);

        // Return toolbar position
        return min::vec2<float>(x, y);
    }
    inline static constexpr size_t focus_start()
    {
        return _focus;
    }
    inline static constexpr size_t focus_size()
    {
        return 1;
    }
    inline static constexpr size_t focus_bar_size()
    {
        return 2;
    }
    inline static constexpr size_t opaque_ext_size()
    {
        return _focus - opaque_start();
    }
    inline static constexpr size_t opaque_base_size()
    {
        return _base - opaque_start();
    }
    inline static constexpr size_t opaque_start()
    {
        return 5;
    }
    inline static constexpr size_t tooltip_start()
    {
        return _ext_hover;
    }
    inline static constexpr size_t tooltip_size()
    {
        return 1;
    }
    inline static constexpr size_t transparent_start()
    {
        return 0;
    }
    inline static constexpr size_t transparent_size()
    {
        return 5;
    }
    inline static constexpr size_t ui_size()
    {
        // 5 + 4 + 16 + 16
        return _base;
    }
};
}

#endif
