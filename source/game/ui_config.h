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
#ifndef _UI_CONFIG__
#define _UI_CONFIG__

namespace game
{

// Font sizes
static constexpr size_t _text_font_size = 28;
static constexpr size_t _inv_font_size = 14;
static constexpr size_t _info_font_size = 20;

// Everything scales on this value
static constexpr float _s_sfg = 16.0;
static constexpr float _s_fg = 32.0;

// Base scale
static constexpr size_t _num_buttons = 8;
static constexpr size_t _num_half_buttons = _num_buttons / 2;
static constexpr float _s_bg = _s_fg + 8.0;
static constexpr float _s_inv_2 = _s_fg * 0.5;
static constexpr float _s_bg_menu_y = _s_bg;
static constexpr float _s_bg_menu_x = _s_bg_menu_y * 10.0;
static constexpr float _s_fg_menu_y = _s_fg;
static constexpr float _s_fg_menu_x = _s_bg_menu_x - (_s_bg - _s_fg);
static constexpr float _s_bg_menu_x_2 = _s_bg_menu_x * 0.5;
static constexpr float _s_bg_menu_y_2 = _s_bg_menu_y * 0.5;
static constexpr float _s_stat_2 = _s_sfg * 0.5;

// Inventory
static constexpr float _store_dy = _s_bg + 8.0;
static constexpr float _tool_dy = _s_bg + 8.0;
static constexpr float _tool_space = _s_bg + 8.0;
static constexpr float _tool_start = (0.5 - _num_half_buttons) * _tool_space;

// Hover
static constexpr float _s_hover_bg_x = 320.0;
static constexpr float _s_hover_bg_y = 190.0;
static constexpr float _s_hover_text_x = 280.0;
static constexpr float _s_hover_text_y = 190.0;

constexpr static float _item_count_dx = (_s_fg - _inv_font_size) * 0.5;
constexpr static float _item_count_dy = _s_fg * -0.5;

// Console and UI Text
static constexpr float _console_dy = (_tool_dy * 2.0) + 4.0;
static constexpr float _s_console_x = 400.0;
static constexpr float _s_console_y = 40.0;
static constexpr float _x_console_wrap = _s_console_x;
static constexpr float _y_console_wrap = _text_font_size;
static constexpr float _s_focus_x = 320.0;
static constexpr float _s_focus_y = 90.0;
static constexpr float _x_focus_wrap = _s_focus_x;
static constexpr float _y_focus_wrap = _text_font_size;
static constexpr float _s_focus_bar_x = _s_focus_x - 50.0;
static constexpr float _s_focus_bar_y = 8.0;
static constexpr float _y_hover_wrap = _info_font_size;
static constexpr float _y_ui_text = _console_dy + 24.0;
static constexpr float _energy_dx = (_tool_space * (_num_half_buttons + 0.5)) + 16.0;
static constexpr float _health_dx = _tool_start - _tool_space - 16.0;

// Alert Text
static constexpr float _x_alert_wrap = 600.0;
static constexpr float _y_alert_wrap = _text_font_size;
static constexpr float _alert_dy = -180.0;

// Experience Bar
static constexpr float _s_exp_x = _s_console_x;
static constexpr float _s_exp_y = 8.0;
static constexpr float _exp_dx = (_s_exp_y - _s_exp_x) * 0.5;
static constexpr float _exp_dy = _console_dy + 24.0;

// Focus Text
static constexpr float _focus_dx = 0.0;
static constexpr float _focus_dy = 120.0;
static constexpr float _focus_bar_dx = (_s_focus_bar_y - _s_focus_bar_x) * 0.5;
static constexpr float _focus_bar_dy = _focus_dy + _text_font_size * 0.5;
static constexpr float _focus_text_dy = _focus_dy - _text_font_size * 0.5;

// Health Bar Meter
static constexpr float _bar_dy = _tool_dy - 20.0;
static constexpr float _meter_dy = _bar_dy + 30.0;

// Oxygen Bar
static constexpr float _s_oxy_x = _s_console_x;
static constexpr float _s_oxy_y = 8.0;
static constexpr float _oxy_dx = (_s_oxy_y - _s_oxy_x) * 0.5;
static constexpr float _oxy_dy = _console_dy + 34.0;

// Menu Text
static constexpr float _menu_dx = 0.0;
static constexpr float _menu_dy = 400.0;
static constexpr float _menu_text_dy = _menu_dy + 2.0;
static constexpr float _x_menu_wrap = _s_bg_menu_x;
static constexpr float _y_menu_wrap = _s_bg_menu_y;

// Stream Text
static constexpr float _stream_dy = _console_dy;
static constexpr float _x_stream_wrap = _x_console_wrap;
static constexpr float _y_stream_wrap = _y_console_wrap;

// Timer Text
static constexpr float _x_timer_wrap = 600.0;
static constexpr float _y_timer_wrap = _text_font_size;
static constexpr float _timer_text_dy = _focus_text_dy;

// Extended UI
static constexpr float _attr_text_dx = -304.0;
static constexpr float _attr_text_dy = 540.0;
static constexpr float _button_dx = 250.0;
static constexpr float _button_dy = 545.0;
static constexpr float _button_space = 20.0;
static constexpr float _cube_dx = -42.0;
static constexpr float _cube_dy = 461.0;
static constexpr float _cube_space = 42.0;
static constexpr float _menu_space = _s_bg_menu_y + 8.0;
static constexpr float _splash_dy = 512.0;
static constexpr float _stat_dx = 0.0;
static constexpr float _stat_dy = 458.0;
static constexpr float _stat_text_dx = 110.0;
static constexpr float _stat_text_dy = 540.0;
}

#endif
