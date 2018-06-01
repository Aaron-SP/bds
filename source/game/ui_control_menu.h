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
#ifndef __UI_CONTROL_MENU__
#define __UI_CONTROL_MENU__

#include <game/id.h>
#include <game/inventory.h>
#include <game/stats.h>
#include <game/ui_bg_assets.h>
#include <game/ui_info.h>
#include <game/ui_state.h>
#include <min/aabbox.h>
#include <min/grid.h>
#include <min/text_buffer.h>
#include <min/vec2.h>
#include <utility>

namespace game
{

class ui_control_menu
{
  private:
    inline static constexpr size_t begin_menu()
    {
        return 0;
    }
    inline static constexpr size_t end_menu()
    {
        return begin_menu() + ui_bg_assets::max_menu_size();
    }
    typedef min::grid<float, uint_fast8_t, uint_fast8_t, min::vec2, min::aabbox, min::aabbox> ui_grid;
    ui_bg_assets *const _assets;
    inventory *const _inv;
    stats *const _stat;
    min::text_buffer *const _text;
    min::grid<float, uint_fast8_t, uint_fast8_t, min::vec2, min::aabbox, min::aabbox> *const _grid;
    std::vector<min::aabbox<float, min::vec2>> *const _shapes;
    bool _minimized;

    inline min::vec2<float> pos_menu(const ui_id ui) const
    {
        // Get row and col
        const unsigned row = ui.index() - begin_menu();
        const unsigned col = 0;

        // Calculate ui element position
        return _assets->menu_position(row, col);
    }

  public:
    ui_control_menu(ui_bg_assets &assets, inventory &inv, stats &stat, min::text_buffer &tb,
                    ui_grid &grid, std::vector<min::aabbox<float, min::vec2>> &shapes)
        : _assets(&assets), _inv(&inv), _stat(&stat),
          _text(&tb), _grid(&grid), _shapes(&shapes),
          _minimized(false) {}

    inline std::pair<bool, ui_id> action_hover(const ui_state &state, const uint_fast8_t mult)
    {
        return std::make_pair(false, 0);
    }
    inline std::pair<bool, ui_id> action_select(const ui_state &state, const uint_fast8_t mult)
    {
        return std::make_pair(false, 0);
    }
    inline bool click_down(ui_state &state)
    {
        return true;
    }
    inline void click_up(ui_state &state) {}
    inline void load_grid(std::ostringstream &stream, const uint_fast16_t width, const uint_fast16_t height) {}
    inline std::pair<bool, ui_id> overlap(ui_state &state, const min::vec2<float> &p)
    {
        return std::make_pair(false, 0);
    }
    inline void position_ui(const ui_state &state)
    {
        // Get start and end of store
        const size_t bm = begin_menu();
        const size_t em = end_menu();

        // Update all store icons
        for (size_t i = bm; i < em; i++)
        {
            // Offset this index for a ui bg store
            const ui_id ui(i);

            // Calculate icon position
            const min::vec2<float> p = pos_menu(ui);

            // Update the black bg icon
            _assets->load_bg_menu_black(ui.bg_menu_index(), p);
            _assets->load_fg_menu_grey(ui.fg_menu_index(), p);
        }
    }
    inline void set_key_down(ui_state &state, const size_t index) {}
    inline void set_key_down_fail(const ui_state &state, const size_t index) {}
    inline void set_key_up(const ui_state &state, const size_t index) {}
    inline void set_minimized(const bool flag)
    {
        _minimized = flag;
    }
    inline void transition_state(ui_state &state) {}
    inline void update(const ui_state &state, std::ostringstream &stream) {}
};
}

#endif
