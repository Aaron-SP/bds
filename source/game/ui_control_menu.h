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
    inline static constexpr size_t end_menu()
    {
        return begin_menu() + ui_bg_assets::max_menu_size();
    }
    typedef min::tree<float, uint_fast8_t, uint_fast8_t, min::vec2, min::aabbox, min::aabbox> ui_tree;
    ui_bg_assets *const _assets;
    ui_menu _menu;
    ui_tree *const _tree;
    std::vector<min::aabbox<float, min::vec2>> *const _shapes;
    bool _minimized;

    inline min::vec2<float> pos_menu(const ui_id ui) const
    {
        // Get row and col
        const size_t row = ui.index() - begin_menu();

        // Calculate ui element position
        return _assets->menu_position(row);
    }
    inline void select_click(const ui_state &state)
    {
        // Have we clicked?
        if (state.is_clicking())
        {
            // Update menu bg color
            const ui_id click = state.get_click();
            _assets->load_bg_menu_light_blue(click.bg_menu_index(), pos_menu(click));
        }
    }
    inline void select_hover(const ui_state &state)
    {
        // Are we hovering?
        if (state.is_hovering())
        {
            // Update menu bg color
            const ui_id hover = state.get_hover();
            _assets->load_bg_menu_yellow(hover.bg_menu_index(), pos_menu(hover));
        }
    }
    inline void unselect_click(const ui_state &state)
    {
        const ui_id click = state.get_click();
        _assets->load_bg_menu_black(click.bg_menu_index(), pos_menu(click));
    }
    inline void unselect_hover(const ui_state &state)
    {
        const ui_id hover = state.get_hover();
        _assets->load_bg_menu_black(hover.bg_menu_index(), pos_menu(hover));
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
        return _menu.callback(ui.index());
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
    ui_control_menu(ui_bg_assets &assets, ui_tree &tree, std::vector<min::aabbox<float, min::vec2>> &shapes)
        : _assets(&assets), _tree(&tree), _shapes(&shapes), _minimized(false) {}

    inline void reset()
    {
        // Reset menu
        _menu.reset();

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
    inline ui_menu &get_menu()
    {
        return _menu;
    }
    inline const ui_menu &get_menu() const
    {
        return _menu;
    }
    inline void load_tree(std::ostringstream &stream, const uint_fast16_t width, const uint_fast16_t height)
    {
        // Clear the shapes buffer
        _shapes->clear();

        // Get start and end of menu
        const size_t bm = begin_menu();
        const size_t em = end_menu();

        // Store rows
        for (size_t i = bm; i < em; i++)
        {
            // Create ui_id
            const ui_id ui(i);

            // Get ui position
            const min::vec2<float> p = pos_menu(ui);

            // Add shape to buffer
            _shapes->push_back(_assets->menu_box(p));
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
                // Unselect hover
                unselect_hover(state);

                // Reset the hover index
                state.set_hovering(false);

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
        // Get start and end of menu
        const size_t bm = begin_menu();
        const size_t em = end_menu();

        // Load the pause menu
        _assets->load_splash_pause();

        // Update all menu icons
        for (size_t i = bm; i < em; i++)
        {
            // Create ui_id
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
