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
#ifndef __UI_STATE__
#define __UI_STATE__

#include <game/id.h>

namespace game
{

enum class ui_mode : int_fast8_t
{
    TITLE = 0,
    BASE = 1,
    EXTEND = 2,
    MENU = 3
};

class ui_state
{
  private:
    ui_mode _mode;
    bool _clicking;
    ui_id _click;
    bool _hovering;
    ui_id _hover;
    bool _selecting;
    ui_id _select;

  public:
    ui_state(const ui_id select)
        : _mode(ui_mode::TITLE),
          _clicking(false), _click(0),
          _hovering(false), _hover(0),
          _selecting(false), _select(select) {}

    inline ui_id get_click() const
    {
        return _click;
    }
    inline ui_id get_hover() const
    {
        return _hover;
    }
    inline ui_mode get_mode() const
    {
        return _mode;
    }
    inline ui_id get_select() const
    {
        return _select;
    }
    inline bool is_clicking() const
    {
        return _clicking;
    }
    inline bool is_click(const ui_id ui) const
    {
        return _click == ui;
    }
    inline bool is_click_select() const
    {
        return _click == _select;
    }
    inline bool is_click_type(const ui_type type) const
    {
        return _click.type() == type;
    }
    inline bool is_hovering() const
    {
        return _hovering;
    }
    inline bool is_hovering_not_button() const
    {
        return _hovering && !is_hover_type(ui_type::button);
    }
    inline bool is_hover(const ui_id ui) const
    {
        return _hover == ui;
    }
    inline bool is_hover_click() const
    {
        return _hover == _click;
    }
    inline bool is_hover_select() const
    {
        return _hover == _select;
    }
    inline bool is_hover_type(const ui_type type) const
    {
        return _hover.type() == type;
    }
    inline bool is_inv_mode() const
    {
        return _mode == ui_mode::BASE || _mode == ui_mode::EXTEND;
    }
    inline bool is_menu_mode() const
    {
        return _mode == ui_mode::MENU;
    }
    inline bool is_selecting() const
    {
        return _selecting;
    }
    inline bool is_select(const ui_id ui) const
    {
        return _select == ui;
    }
    inline bool is_select_type(const ui_type type) const
    {
        return _select.type() == type;
    }
    inline bool is_title_mode() const
    {
        return _mode == ui_mode::TITLE;
    }
    inline void set_click(const ui_id id)
    {
        _click = id;
    }
    inline void set_clicking(const bool flag)
    {
        _clicking = flag;
    }
    inline void set_mode(const ui_mode mode)
    {
        _mode = mode;
    }
    inline void set_hover(const ui_id id)
    {
        _hover = id;
    }
    inline void set_hovering(const bool flag)
    {
        _hovering = flag;
    }
    inline void set_select(const ui_id id)
    {
        _select = id;
    }
    inline void set_selecting(const bool flag)
    {
        _selecting = flag;
    }
};
}

#endif
