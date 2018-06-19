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
#include <game/ui_bg_assets.h>
#include <game/ui_info.h>
#include <game/ui_menu.h>
#include <game/ui_state.h>
#include <min/aabbox.h>
#include <min/text_buffer.h>
#include <min/tree.h>
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
    inline static constexpr size_t end_menu_base()
    {
        return begin_menu() + ui_bg_assets::max_menu_base_size();
    }
    inline static constexpr size_t end_menu_ext()
    {
        return begin_menu() + ui_bg_assets::max_menu_ext_size();
    }
    typedef min::tree<float, uint_fast8_t, uint_fast8_t, min::vec2, min::aabbox, min::aabbox> ui_tree;
    ui_bg_assets *const _assets;
    ui_menu *const _menu;
    ui_tree *const _tree;
    std::vector<min::aabbox<float, min::vec2>> *const _shapes;
    bool _minimized;
    const min::vec2<float> _bg_scale;
    const min::vec2<float> _fg_scale;
    const min::vec2<float> _bg_ext_scale;
    const min::vec2<float> _fg_ext_scale;

    inline min::vec2<float> pos_base_menu(const ui_id ui) const
    {
        // Get row and col
        const unsigned row = ui.index() - begin_menu();

        // Calculate ui element position
        return _assets->menu_base_position(row);
    }
    inline min::vec2<float> pos_ext_menu(const ui_id ui) const
    {
        // Get row and col
        const unsigned row = ui.index() / 4;
        const unsigned col = ui.index() & 3;

        // Calculate ui element position
        return _assets->menu_ext_position(row, col);
    }
    inline void select_click(const ui_state &state)
    {
        // Have we clicked?
        if (state.is_clicking())
        {
            // Update menu bg color
            const ui_id click = state.get_click();
            if (state.get_mode() == ui_mode::MENU)
            {
                _assets->load_bg_menu_light_blue(click.bg_menu_base_index(), _bg_scale, pos_base_menu(click));
            }
            else
            {
                _assets->load_bg_menu_light_blue(click.bg_menu_ext_index(), _bg_ext_scale, pos_ext_menu(click));
            }
        }
    }
    inline void select_hover(const ui_state &state)
    {
        // Are we hovering?
        if (state.is_hovering())
        {
            // Update menu bg color
            const ui_id hover = state.get_hover();
            if (state.get_mode() == ui_mode::MENU)
            {
                _assets->load_bg_menu_yellow(hover.bg_menu_base_index(), _bg_scale, pos_base_menu(hover));
            }
            else
            {
                _assets->load_bg_menu_yellow(hover.bg_menu_ext_index(), _bg_ext_scale, pos_ext_menu(hover));
            }
        }
    }
    inline void unselect_click(const ui_state &state)
    {
        const ui_id click = state.get_click();
        if (state.is_hover_click())
        {
            select_hover(state);
        }
        else if (state.get_mode() == ui_mode::MENU)
        {
            _assets->load_bg_menu_black(click.bg_menu_base_index(), _bg_scale, pos_base_menu(click));
        }
        else
        {
            _assets->load_bg_menu_black(click.bg_menu_ext_index(), _bg_ext_scale, pos_ext_menu(click));
        }
    }
    inline void unselect_hover(const ui_state &state)
    {
        const ui_id hover = state.get_hover();
        if (state.get_mode() == ui_mode::MENU)
        {
            _assets->load_bg_menu_black(hover.bg_menu_base_index(), _bg_scale, pos_base_menu(hover));
        }
        else
        {
            _assets->load_bg_menu_black(hover.bg_menu_ext_index(), _bg_ext_scale, pos_ext_menu(hover));
        }
    }
    inline bool set_click_down(ui_state &state, const ui_id ui)
    {
        // Set selected index
        state.set_click(ui);

        // Set clicking
        state.set_clicking(true);

        // Select click
        select_click(state);

        // A click happened
        return _menu->callback(ui.index());
    }
    inline void set_hover_down(ui_state &state, const ui_id ui)
    {
        // Already hovering here?
        if (state.is_hovering() && state.is_hover(ui))
        {
            return;
        }

        // Unselect hover
        unselect_hover(state);

        // Set selected index
        state.set_hover(ui);

        // Set hovering
        state.set_hovering(true);

        // Select hover
        select_hover(state);
    }

  public:
    ui_control_menu(ui_bg_assets &assets, ui_menu &menu, ui_tree &tree, std::vector<min::aabbox<float, min::vec2>> &shapes)
        : _assets(&assets), _menu(&menu), _tree(&tree), _shapes(&shapes), _minimized(false),
          _bg_scale(_s_bg_menu_x, _s_bg_menu_y), _fg_scale(_s_fg_menu_x, _s_fg_menu_y),
          _bg_ext_scale(_s_bg_menu_ext_x, _s_bg_menu_y), _fg_ext_scale(_s_fg_menu_ext_x, _s_fg_menu_y) {}

    inline void reset()
    {
        // Reset minimized flag
        _minimized = false;
    }
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
        // Click on the currently hovered icon
        if (state.is_hovering() && !_minimized)
        {
            // Click happened
            const ui_id hover = state.get_hover();
            return set_click_down(state, hover);
        }

        return false;
    }
    inline void click_up(ui_state &state)
    {
        if (state.is_clicking())
        {
            // Deselect button on click up
            unselect_click(state);

            // Unset clicking
            state.set_clicking(false);
        }
    }
    inline void load_tree(const ui_state &state, std::ostringstream &stream, const uint_fast16_t width, const uint_fast16_t height)
    {
        // Clear the shapes buffer
        _shapes->clear();

        // Select which menu mode to switch to
        if (state.get_mode() == ui_mode::MENU)
        {
            // Get start and end of menu
            const size_t bm = begin_menu();
            const size_t em = end_menu_base();

            // Store rows
            for (size_t i = bm; i < em; i++)
            {
                // Create ui_id
                const ui_id ui(i);

                // Get ui position
                const min::vec2<float> p = pos_base_menu(ui);

                // Add shape to buffer
                _shapes->push_back(_assets->menu_base_box(p));
            }
        }
        else
        {
            // Get start and end of menu
            const size_t bm = begin_menu();
            const size_t em = end_menu_ext();

            // Store rows
            for (size_t i = bm; i < em; i++)
            {
                // Create ui_id
                const ui_id ui(i);

                // Get ui position
                const min::vec2<float> p = pos_ext_menu(ui);

                // Add shape to buffer
                _shapes->push_back(_assets->menu_ext_box(p));
            }
        }

        // Calculate the tree depth, 2^5 = 32 was 24
        const uint_fast16_t depth = 5;

        // Insert the shapes into the tree
        _tree->insert(*_shapes, depth);
    }
    inline std::pair<bool, ui_id> overlap(ui_state &state, const min::vec2<float> &p)
    {
        // Is not minimzed?
        if (!_minimized)
        {
            // Bad point
            if (!_tree->inside(p))
            {
                return std::make_pair(false, 0);
            }

            // Search for overlapping cells
            const std::vector<uint_fast8_t> &map = _tree->get_index_map();
            const std::vector<uint_fast8_t> &hits = _tree->point_inside(p);

            bool hit = false;
            ui_id id(0);

            // Set hover if overlapping
            for (const auto &h : hits)
            {
                if ((*_shapes)[map[h]].point_inside(p))
                {
                    // Flag that we hit something
                    hit = true;

                    // Cache id
                    id = ui_id(map[h]);

                    // Highlight the menu cell
                    set_hover_down(state, id);
                    break;
                }
            }

            // If he didn't hit anything
            if (!hit)
            {
                // If we are hovering, unselect hover
                if (state.is_hovering())
                {
                    // Unselect hover
                    unselect_hover(state);

                    // Reset the hover index
                    state.set_hovering(false);
                }

                // Not overlapping a UI element
                return std::make_pair(false, 0);
            }
            else
            {
                // Overlapping a UI element
                return std::make_pair(true, id);
            }
        }

        // Not overlapping a UI element
        return std::make_pair(false, 0);
    }
    inline void position_ui(const ui_state &state)
    {
        // Select which menu mode to switch to
        if (state.get_mode() == ui_mode::MENU)
        {
            // Get start and end of menu
            const size_t bm = begin_menu();
            const size_t em = end_menu_base();

            // Load the pause menu
            _assets->load_splash_pause();

            // Update all menu icons
            for (size_t i = bm; i < em; i++)
            {
                // Create ui_id
                const ui_id ui(i);

                // Calculate icon position
                const min::vec2<float> p = pos_base_menu(ui);

                // Update the black bg icon
                _assets->load_bg_menu_black(ui.bg_menu_base_index(), _bg_scale, p);
                _assets->load_fg_menu_grey(ui.fg_menu_base_index(), _fg_scale, p);
            }
        }
        else
        {
            // Get start and end of menu
            const size_t bm = begin_menu();
            const size_t em = end_menu_ext();

            // Load the pause menu
            _assets->load_splash_pause();

            // Update all menu icons
            for (size_t i = bm; i < em; i++)
            {
                // Create ui_id
                const ui_id ui(i);

                // Calculate icon position
                const min::vec2<float> p = pos_ext_menu(ui);

                // Update the black bg icon
                _assets->load_bg_menu_black(ui.bg_menu_ext_index(), _bg_ext_scale, p);
                _assets->load_fg_menu_grey(ui.fg_menu_ext_index(), _fg_ext_scale, p);
            }
        }
    }
    inline void set_extended(const bool flag)
    {
        _menu->set_extended(flag);
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
