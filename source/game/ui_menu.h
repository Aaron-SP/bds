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
    static constexpr size_t _size = ui_bg_assets::max_menu_size();
    const std::string _back;
    const std::string _quit;
    const std::string _title;
    const std::string _empty;
    std::array<const std::string *, _size> _menu;
    std::array<menu_call, _size> _callback;
    bool _dirty;

  public:
    ui_menu()
        : _back("Back to Game"), _quit("Save and Exit Game"), _title("Return to Title"), _empty(),
          _menu{&_back, &_title, &_quit, &_empty, &_empty}, _callback{}, _dirty(true) {}

    inline void reset()
    {
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
    inline bool is_dirty() const
    {
        return _dirty;
    }
    inline const std::array<const std::string *, _size> &get_strings() const
    {
        return _menu;
    }
    inline min::vec2<float> position(const uint_fast16_t center_w, const size_t index) const
    {
        return ui_bg_assets::menu_text_position(center_w, index);
    }
    inline void set_callback(const size_t index, const menu_call &f)
    {
        _callback[index] = f;
    }
    inline static constexpr size_t size()
    {
        return _size;
    }
};
}

#endif
