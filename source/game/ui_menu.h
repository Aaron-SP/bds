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
#ifndef __UI_MENU__
#define __UI_MENU__

#include <array>
#include <game/callback.h>
#include <game/id.h>
#include <game/ui_bg_assets.h>
#include <string>

namespace game
{

class ui_menu
{
  private:
    static constexpr size_t _size = ui_bg_assets::max_menu_ext_size();
    const std::string _start;
    const std::string _back;
    const std::string _title;
    const std::string _quit;
    const std::string _controls;
    const std::string _menu_back;
    const std::string _empty;
    std::array<const std::string *, _size> _prefix;
    std::array<const std::string *, _size> _str;
    std::array<menu_call, _size> _callback;
    bool _extended;
    bool _dirty;

  public:
    ui_menu()
        : _start("Start Game"), _back("Back to Game"), _title("Return to Title"),
          _quit("Save and Exit Game"), _controls("Controls"), _menu_back("Back"),
          _empty(), _prefix{}, _str{}, _callback{}, _extended(false), _dirty(true)
    {
        // Set all menu string pointers to empty
        const size_t size = _size;
        for (size_t i = 0; i < size; i++)
        {
            _prefix[i] = &_empty;
            _str[i] = &_empty;
        }
    }

    inline void reset_game_menu()
    {
        // Reset all strings and callbacks
        const size_t size = _size;
        for (size_t i = 0; i < size; i++)
        {
            _prefix[i] = &_empty;
            _str[i] = &_empty;
            _callback[i] = nullptr;
        }

        // Set the default menu strings
        _str[0] = &_back;
        _str[1] = &_title;
        _str[2] = &_quit;
        _str[3] = &_controls;
        _str[4] = &_empty;

        // Set dirty flag
        _extended = false;
        _dirty = true;
    }
    inline void reset_title_menu()
    {
        // Reset all strings and callbacks
        const size_t size = _size;
        for (size_t i = 0; i < size; i++)
        {
            _prefix[i] = &_empty;
            _str[i] = &_empty;
            _callback[i] = nullptr;
        }

        // Set the default menu strings
        _str[0] = &_start;
        _str[1] = &_quit;

        // Set dirty flag
        _extended = false;
        _dirty = true;
    }
    inline bool callback(const size_t index)
    {
        if (_callback[index])
        {
            // Call the menu function
            _callback[index]();

            // A call happened
            return true;
        }

        // No call happened
        return false;
    }
    inline void clean()
    {
        _dirty = false;
    }
    inline const std::array<const std::string *, _size> &get_prefixs() const
    {
        return _prefix;
    }
    inline const std::array<const std::string *, _size> &get_strings() const
    {
        return _str;
    }
    inline bool is_dirty() const
    {
        return _dirty;
    }
    inline bool is_extended() const
    {
        return _extended;
    }
    inline void make_dirty()
    {
        _dirty = true;
    }
    inline min::vec2<float> position_text(const uint_fast16_t center_w, const size_t index) const
    {
        if (_extended)
        {
            // Get row and col
            const unsigned row = index & 7;
            const unsigned col = index / 8;
            return ui_bg_assets::menu_ext_text_position(center_w, row, col);
        }
        else
        {
            return ui_bg_assets::menu_base_text_position(center_w, index);
        }
    }
    inline void set_callback(const size_t index, const menu_call &f)
    {
        _callback[index] = f;
    }
    inline void set_extended(const bool flag)
    {
        _extended = flag;
        _dirty = true;
    }
    inline void set_prefix(const size_t index, const std::string *str)
    {
        _prefix[index] = str;
    }
    inline void set_prefix_empty(const size_t index)
    {
        _prefix[index] = &_empty;
    }
    inline void set_string(const size_t index, const std::string *str)
    {
        _str[index] = str;
    }
    inline void set_string_back(const size_t index)
    {
        _str[index] = &_menu_back;
    }
    inline void set_string_empty(const size_t index)
    {
        _str[index] = &_empty;
    }
    inline static constexpr size_t max_size()
    {
        return _size;
    }
};
}

#endif
