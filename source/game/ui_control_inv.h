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
#ifndef __UI_CONTROL_INVENTORY__
#define __UI_CONTROL_INVENTORY__

#include <game/def.h>
#include <game/id.h>
#include <game/inventory.h>
#include <game/stats.h>
#include <game/ui_bg_assets.h>
#include <game/ui_info.h>
#include <game/ui_state.h>
#include <min/aabbox.h>
#include <min/text_buffer.h>
#include <min/tree.h>
#include <min/vec2.h>
#include <utility>

namespace game
{

class ui_control_inv
{
  private:
    static constexpr size_t _border = 6;
    static constexpr size_t _text_spacing = _inv_font_size + _border;
    static constexpr size_t _button_size = stats::stat_str_size() - 1;
    inline static constexpr size_t begin_button()
    {
        return 49;
    }
    inline static constexpr size_t end_button()
    {
        return begin_button() + _button_size;
    }
    ui_bg_assets *const _assets;
    inventory *const _inv;
    stats *const _stat;
    min::text_buffer *const _text;
    ui_tree *const _tree;
    std::vector<min::aabbox<float, min::vec2>> *const _shapes;
    bool _minimized;
    const std::string _invalid_str;

    inline bool action(const ui_id ui, const uint_fast8_t mult)
    {
        // Get the UI index
        const size_t index = ui.index();

        // Choose between crafting and decaying
        const std::pair<bool, item_id> p = (index >= _inv->begin_cube() && index < _inv->end_cube())
                                               ? _inv->craft(index, mult)
                                               : _inv->decay(index, mult);

        // If decaying consumables add stat points
        if (p.first)
        {
            switch (p.second)
            {
            case item_id::CONS_EGGP:
            case item_id::CONS_GR_PEP:
            case item_id::CONS_RED_PEP:
            case item_id::CONS_TOM:
                _stat->add_exp(10.0);
                _stat->add_health(25.0);
                break;
            case item_id::CONS_BATTERY:
                _stat->add_exp(10.0);
                _stat->add_energy(50.0);
                break;
            case item_id::CONS_OXYGEN:
                _stat->add_exp(25.0);
                _stat->add_oxygen(10.0);
                break;
            default:
                _stat->add_exp(mult * 25.0);
                break;
            }
        }

        // Return decay status
        return p.first;
    }
    inline void clear_stream(std::ostringstream &stream) const
    {
        stream.clear();
        stream.str(std::string());
    }
    inline min::vec2<float> pos_button(const ui_id ui) const
    {
        // Get row and col
        const unsigned row = ui.index() - begin_button();
        const unsigned col = 0;

        // Calculate ui element position
        return _assets->button_position(row, col);
    }
    inline min::vec2<float> pos_cube(const ui_id ui) const
    {
        // Get row and col
        const unsigned row = ui.row3();
        const unsigned col = ui.col3();

        // Calculate ui element position
        return _assets->cube_position(row, col);
    }
    inline min::vec2<float> pos_ext(const ui_id ui) const
    {
        // Get row and col
        const unsigned row = ui.row8() + 1;
        const unsigned col = ui.col8();

        // Calculate ui element position
        return _assets->toolbar_position(row, col);
    }
    inline min::vec2<float> pos_key(const ui_id ui) const
    {
        // Get row and col
        const unsigned row = 0;
        const unsigned col = ui.col8();

        // Calculate ui element position
        return _assets->toolbar_position(row, col);
    }
    inline min::vec2<float> pos_store(const ui_id ui) const
    {
        // Get row and col
        const unsigned row = 0;
        const unsigned col = ui.col8();

        // Calculate ui element position
        return _assets->store_position(row, col);
    }
    inline void select_active(const ui_state &state)
    {
        // Bg key placement
        const ui_id select = state.get_select();
        const min::vec2<float> active = _assets->toolbar_position(0, select.col8());

        // Set activated index color to white
        _assets->load_bg_yellow(select.bg_key_index(), active);
    }
    inline void select_click(const ui_state &state)
    {
        // Have we clicked?
        if (state.is_clicking())
        {
            // Determine inventory type to update bg color
            const ui_id click = state.get_click();
            const ui_type type = click.type();
            switch (type)
            {
            case ui_type::button:
                return stat_select(click);
            case ui_type::cube:
                return _assets->load_bg_light_blue(click.bg_cube_index(), pos_cube(click));
            case ui_type::extend:
                return _assets->load_bg_light_blue(click.bg_ext_index(), pos_ext(click));
            case ui_type::key:
                return _assets->load_bg_light_blue(click.bg_key_index(), pos_key(click));
            case ui_type::store:
                return _assets->load_bg_light_blue(click.bg_store_index(), pos_store(click));
            default:
                break;
            }
        }
    }
    inline void unselect_click(const ui_state &state)
    {
        if (state.is_click_select())
        {
            select_active(state);
        }
        else if (state.is_hover_click())
        {
            select_hover(state);
        }
        else
        {
            // Determine inventory type to update bg color
            const ui_id click = state.get_click();
            const ui_type type = click.type();
            switch (type)
            {
            case ui_type::button:
                return stat_unselect(click);
            case ui_type::cube:
                return _assets->load_bg_black(click.bg_cube_index(), pos_cube(click));
            case ui_type::extend:
                return _assets->load_bg_black(click.bg_ext_index(), pos_ext(click));
            case ui_type::key:
                return _assets->load_bg_black(click.bg_key_index(), pos_key(click));
            case ui_type::store:
                return _assets->load_bg_black(click.bg_store_index(), pos_store(click));
            default:
                break;
            }
        }
    }
    inline void select_hover(const ui_state &state)
    {
        // Are we hovering?
        if (state.is_hovering())
        {
            // Determine inventory type to update bg color
            const ui_id hover = state.get_hover();
            const ui_type type = hover.type();
            if (state.is_clicking())
            {
                switch (type)
                {
                case ui_type::button:
                    return stat_hover(hover);
                case ui_type::cube:
                    return _assets->load_bg_white(hover.bg_cube_index(), pos_cube(hover));
                case ui_type::extend:
                    return _assets->load_bg_white(hover.bg_ext_index(), pos_ext(hover));
                case ui_type::key:
                    return _assets->load_bg_white(hover.bg_key_index(), pos_key(hover));
                case ui_type::store:
                    return _assets->load_bg_white(hover.bg_store_index(), pos_store(hover));
                default:
                    break;
                }
            }
            else
            {
                switch (type)
                {
                case ui_type::button:
                    return stat_hover(hover);
                case ui_type::cube:
                    return _assets->load_bg_yellow(hover.bg_cube_index(), pos_cube(hover));
                case ui_type::extend:
                    return _assets->load_bg_yellow(hover.bg_ext_index(), pos_ext(hover));
                case ui_type::key:
                    return _assets->load_bg_yellow(hover.bg_key_index(), pos_key(hover));
                case ui_type::store:
                    return _assets->load_bg_yellow(hover.bg_store_index(), pos_store(hover));
                default:
                    break;
                }
            }
        }
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

        // If key is not selected or clicked change color
        const bool clicked = (state.is_clicking() && state.is_click(ui));
        if (!clicked && !state.is_select(ui))
        {
            // Select hover
            select_hover(state);
        }
    }
    inline void unselect_hover(const ui_state &state)
    {
        // Don't deselect an active click or select icon
        const bool skip = (state.is_clicking() && state.is_hover_click()) || state.is_hover_select();
        if (!skip)
        {
            // Determine inventory type to update bg color
            const ui_id hover = state.get_hover();
            const ui_type type = hover.type();
            switch (type)
            {
            case ui_type::button:
                return stat_unselect(hover);
            case ui_type::cube:
                return _assets->load_bg_black(hover.bg_cube_index(), pos_cube(hover));
            case ui_type::extend:
                return _assets->load_bg_black(hover.bg_ext_index(), pos_ext(hover));
            case ui_type::key:
                return _assets->load_bg_black(hover.bg_key_index(), pos_key(hover));
            case ui_type::store:
                return _assets->load_bg_black(hover.bg_store_index(), pos_store(hover));
            default:
                break;
            }
        }
    }
    inline bool set_click_down(ui_state &state, const ui_id ui)
    {
        // If we are clicking a button
        if (ui.type() == ui_type::button)
        {
            // If clicking reset it
            if (state.is_clicking())
            {
                unselect_click(state);
            }

            // Set selected index
            state.set_click(ui);

            // Set clicking
            state.set_clicking(true);

            // Select hover
            select_click(state);

            // Set stat points
            const size_t stat_index = ui.index() - begin_button();
            if (_stat->has_stat_points() && stat_index < _stat->stat_str_size())
            {
                _stat->set_point(stat_index);
            }
            else
            {
                // Early return
                return false;
            }
        }
        else if (state.is_clicking() && state.is_click(ui))
        {
            // If we are unclicking
            unselect_click(state);

            // Unset clicking
            state.set_clicking(false);
        }
        else if (state.is_clicking())
        {
            // Unselect old click
            unselect_click(state);

            // If not a button
            if (!state.is_click_type(ui_type::button))
            {
                // Swap inventory
                const ui_id click = state.get_click();
                _inv->swap(click.index(), ui.index());

                // Disable clicking
                state.set_clicking(false);
            }
        }
        else
        {
            // Set selected index
            state.set_click(ui);

            // Set clicking
            state.set_clicking(true);

            // Select click
            select_click(state);
        }

        // A click happened
        return true;
    }
    inline void set_inventory(const ui_id ui, const item &it, const min::vec2<float> &p)
    {
        // Draw key icon overlay based on type
        const item_type type = it.type();
        switch (type)
        {
        case item_type::empty:
            return _assets->load_empty_icon(ui, p);
        case item_type::skill:
            switch (it.id())
            {
            case item_id::AUTO_BEAM:
                return _assets->load_auto_icon(ui, p);
            case item_id::BEAM:
                return _assets->load_beam_icon(ui, p);
            case item_id::CHARGE:
                return _assets->load_charge_icon(ui, p);
            case item_id::GRAPPLE:
                return _assets->load_grapple_icon(ui, p);
            case item_id::GRENADE:
                return _assets->load_grenade_icon(ui, p);
            case item_id::JET:
                return _assets->load_jet_icon(ui, p);
            case item_id::MISSILE:
                return _assets->load_missile_icon(ui, p);
            case item_id::PORTAL:
                return _assets->load_portal_icon(ui, p);
            case item_id::SCAN:
                return _assets->load_scan_icon(ui, p);
            case item_id::SCATTER:
                return _assets->load_scatter_icon(ui, p);
            case item_id::SPEED:
                return _assets->load_speed_icon(ui, p);
            default:
                return;
            }
        case item_type::block:
            return _assets->load_block_icon(ui, it.to_block_id(), p);
        case item_type::item:
            return _assets->load_item_icon(ui, it.to_item_id(), p);
        }
    }
    inline static constexpr float stat_offset(const uint_fast16_t value)
    {
        // If three, two, or one digit
        if (value > 99)
        {
            return 86;
        }
        else if (value > 9)
        {
            return 90;
        }

        return 94;
    }
    inline void stat_select(const ui_id ui)
    {
        if (_stat->has_stat_points())
        {
            return _assets->load_stat_click(ui.button_index(), pos_button(ui));
        }

        return _assets->load_stat_grey(ui.button_index(), pos_button(ui));
    }
    inline void stat_hover(const ui_id ui)
    {
        if (_stat->has_stat_points())
        {
            return _assets->load_stat_hover(ui.button_index(), pos_button(ui));
        }

        return _assets->load_stat_grey(ui.button_index(), pos_button(ui));
    }
    inline void stat_unselect(const ui_id ui)
    {
        if (_stat->has_stat_points())
        {
            return _assets->load_stat_red(ui.button_index(), pos_button(ui));
        }

        return _assets->load_stat_grey(ui.button_index(), pos_button(ui));
    }
    inline void select_key(const ui_state &state)
    {
        if (state.is_selecting())
        {
            // Bg key placement
            const ui_id select = state.get_select();
            const min::vec2<float> active = _assets->toolbar_position(0, select.col8());

            // Set activated index color to white
            _assets->load_bg_white(select.bg_key_index(), active);
        }
    }
    inline void unselect_key(const ui_state &state)
    {
        // Bg key placement
        const ui_id select = state.get_select();
        const min::vec2<float> prev = _assets->toolbar_position(0, select.col8());

        // Set previous unselected color to black
        _assets->load_bg_black(select.bg_key_index(), prev);
    }
    inline void update_inv_slot(const ui_id ui, const item &it)
    {
        // Determine inventory type to update icon
        const ui_type type = ui.type();
        switch (type)
        {
        case ui_type::cube:
            return set_inventory(ui.fg_cube_index(), it, pos_cube(ui));
        case ui_type::extend:
            return set_inventory(ui.fg_ext_index(), it, pos_ext(ui));
        case ui_type::key:
            return set_inventory(ui.fg_key_index(), it, pos_key(ui));
        case ui_type::store:
            return set_inventory(ui.fg_store_index(), it, pos_store(ui));
        default:
            break;
        }
    }
    inline void upload_text() const
    {
        // Unbind the last VAO to prevent scrambling buffers
        _text->unbind();

        // Upload the text glyphs to the GPU
        _text->upload();
    }

  public:
    ui_control_inv(ui_bg_assets &assets, inventory &inv, stats &stat, min::text_buffer &tb,
                   ui_tree &tree, std::vector<min::aabbox<float, min::vec2>> &shapes)
        : _assets(&assets), _inv(&inv), _stat(&stat),
          _text(&tb), _tree(&tree), _shapes(&shapes),
          _minimized(false), _invalid_str("Invalid") {}

    inline void reset()
    {
        _minimized = false;
    }
    inline std::pair<bool, ui_id> action_hover(const ui_state &state, const uint_fast8_t mult)
    {
        const ui_id hover = state.get_hover();
        if (state.is_hovering_not_button() && !_minimized)
        {
            // Do action on hovering index
            return std::make_pair(action(hover, mult), hover);
        }

        // No action
        return std::make_pair(false, hover);
    }
    inline std::pair<bool, ui_id> action_select(const ui_state &state, const uint_fast8_t mult)
    {
        const ui_id select = state.get_select();
        if (!state.is_select_type(ui_type::button))
        {
            // Do action on selected index
            return std::make_pair(action(select, mult), select);
        }

        // No action
        return std::make_pair(false, select);
    }
    inline size_t bg_text_size(const ui_state &state) const
    {
        // If in extended state
        if (state.get_mode() == ui_mode::INV_EXT)
        {
            return _text->size();
        }
        else if (state.get_mode() == ui_mode::INV)
        {
            // If in base state
            return _inv->end_key();
        }

        // Do not draw anything
        return 0;
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
        if (state.is_clicking() && state.is_click_type(ui_type::button))
        {
            // Deselect button on click up
            unselect_click(state);

            // Unset clicking
            state.set_clicking(false);
        }
    }
    inline bool drop(const ui_state &state)
    {
        // If we are hovering
        if (state.is_hovering_not_button())
        {
            // Drop hovering inventory
            const ui_id hover = state.get_hover();
            _inv->drop(hover.index());

            // Return that we dropped the item
            return true;
        }

        return false;
    }
    inline ui_info get_ui_info(const ui_state &state) const
    {
        const bool not_button = !state.is_hover_type(ui_type::button);
        const ui_id hover = state.get_hover();
        const item &it = (*_inv)[hover.index()];
        const std::string &name = (not_button) ? _inv->get_name(it.id()) : _invalid_str;
        const std::string &info = (not_button) ? _inv->get_info(it.id()) : _invalid_str;

        // Return ui info
        return ui_info(name, info, it);
    }
    inline void load_tree(const ui_state &state, std::ostringstream &stream, const uint_fast16_t width, const uint_fast16_t height)
    {
        // Clear the shapes buffer
        _shapes->clear();
        _text->clear();

        // Get start and end of store
        const size_t bs = _inv->begin_store();
        const size_t es = _inv->end_store();

        // Store rows
        for (size_t i = bs; i < es; i++)
        {
            // Get ui position
            const min::vec2<float> p = pos_store(ui_id(i));

            // Add shape to buffer
            _shapes->push_back(_assets->inv_box(p));

            // Clear and reset the stream
            clear_stream(stream);

            // Update item count
            stream << static_cast<int>((*_inv)[i].count());

            // Add text for each box
            _text->add_text(stream.str(), p.x() + _item_count_dx, p.y() + _item_count_dy);
        }

        // Get start and end of keys
        const size_t bk = _inv->begin_key();
        const size_t ek = _inv->end_key();

        // First row
        for (size_t i = bk; i < ek; i++)
        {
            // Get ui position
            const min::vec2<float> p = pos_key(ui_id(i));

            // Add shape to buffer
            _shapes->push_back(_assets->inv_box(p));

            // Clear and reset the stream
            clear_stream(stream);

            // Update item count
            stream << static_cast<int>((*_inv)[i].count());

            // Add text for each box
            _text->add_text(stream.str(), p.x() + _item_count_dx, p.y() + _item_count_dy);
        }

        // Get start and end of keys
        const size_t be = _inv->begin_extend();
        const size_t ee = _inv->end_extend();

        // Extended rows
        for (size_t i = be; i < ee; i++)
        {
            // Get ui extended position
            const min::vec2<float> p = pos_ext(ui_id(i));

            // Add shape to buffer
            _shapes->push_back(_assets->inv_box(p));

            // Clear and reset the stream
            clear_stream(stream);

            // Update item count
            stream << static_cast<int>((*_inv)[i].count());

            // Add text for each box
            _text->add_text(stream.str(), p.x() + _item_count_dx, p.y() + _item_count_dy);
        }

        // Get start and end of keys
        const size_t bc = _inv->begin_cube();
        const size_t ec = _inv->end_cube();

        // Extended rows
        for (size_t i = bc; i < ec; i++)
        {
            // Get ui extended position
            const min::vec2<float> p = pos_cube(ui_id(i));

            // Add shape to buffer
            _shapes->push_back(_assets->inv_box(p));

            // Clear and reset the stream
            clear_stream(stream);

            // Update item count
            stream << static_cast<int>((*_inv)[i].count());

            // Add text for each box
            _text->add_text(stream.str(), p.x() + _item_count_dx, p.y() + _item_count_dy);
        }

        // Get start and end of buttons
        const size_t bb = begin_button();
        const size_t eb = end_button();

        // Button rows
        for (size_t i = bb; i < eb; i++)
        {
            // Get ui extended position
            const min::vec2<float> p = pos_button(ui_id(i));

            // Add shape to buffer
            _shapes->push_back(_assets->stat_box(p));
        }

        // Add attr text
        const size_t attr_size = _stat->attr_str_size();
        for (size_t i = 0; i < attr_size; i++)
        {
            // Get attr text position
            const min::vec2<float> p = _assets->attr_position(i, _text_spacing);

            // Add attr string to stream
            clear_stream(stream);
            stream << _stat->attr_str(i) << ": " << _stat->attr_value(i);

            // Set attr text at offset
            _text->add_text(stream.str(), p.x(), p.y());
        }

        // Add stat text
        const size_t stat_size = _stat->stat_str_size();
        for (size_t i = 0; i < stat_size; i++)
        {
            // Get stat text position
            const min::vec2<float> p = _assets->stat_position(i, _text_spacing);

            // Add stat descriptor and value text
            _text->add_text(_stat->stat_str(i), p.x(), p.y());

            // Get stat value
            const uint_fast16_t value = _stat->stat_value(i);

            // Create string from value
            clear_stream(stream);
            stream << value;

            // Set stat text at offset
            const float dx = stat_offset(value);
            _text->add_text(stream.str(), p.x() + dx, p.y());
        }

        // Calculate the tree depth, 2^5 = 32 was 24
        const uint_fast16_t depth = 5;

        // Insert the shapes into the tree
        _tree->insert(*_shapes, depth);

        // Upload text for display
        upload_text();
    }
    inline std::pair<bool, ui_id> overlap(ui_state &state, const min::vec2<float> &p)
    {
        // Is the inventory open?
        if (state.get_mode() == ui_mode::INV_EXT && !_minimized)
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

                    // Highlight the inventory cell
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
            else if (id.type() == ui_type::button && !_stat->has_stat_points())
            {
                // If we hit a button and no stat points available, don't play sound
                return std::make_pair(false, id);
            }
            else
            {
                // Update tooltip position
                _assets->load_bg_hover(p);

                // Overlapping a UI element
                return std::make_pair(true, id);
            }
        }

        // Not overlapping a UI element
        return std::make_pair(false, 0);
    }
    inline void position_ui(const ui_state &state)
    {
        // Get start and end of store
        const size_t bs = _inv->begin_store();
        const size_t es = _inv->end_store();

        // Update all store icons
        for (size_t i = bs; i < es; i++)
        {
            // Offset this index for a ui bg store
            const ui_id ui(i);

            // Calculate icon position
            const min::vec2<float> p = pos_store(ui);

            // Update the black bg icon
            _assets->load_bg_black(ui.bg_store_index(), p);

            // Update the icon
            set_inventory(ui.fg_store_index(), (*_inv)[i], p);
        }

        // Get start and end of keys
        const size_t bk = _inv->begin_key();
        const size_t ek = _inv->end_key();

        // Update all key icons
        for (size_t i = bk; i < ek; i++)
        {
            // Offset this index for a ui bg key
            const ui_id ui(i);

            // Calculate icon position
            const min::vec2<float> p = pos_key(ui);

            // Update the black bg icon
            _assets->load_bg_black(ui.bg_key_index(), p);

            // Update the icon
            set_inventory(ui.fg_key_index(), (*_inv)[i], p);
        }

        // Get start and end of extends
        const size_t be = _inv->begin_extend();
        const size_t ee = _inv->end_extend();

        // Update all extended icons
        for (size_t i = be; i < ee; i++)
        {
            // Offset this index for a ui bg ext
            const ui_id ui(i);

            // Calculate icon position
            const min::vec2<float> p = pos_ext(ui);

            // Update the black bg icon
            _assets->load_bg_black(ui.bg_ext_index(), p);

            // Update the icon
            set_inventory(ui.fg_ext_index(), (*_inv)[i], p);
        }

        // Load stat background
        _assets->load_bg_stat();

        // Get start and end of cubes
        const size_t bc = _inv->begin_cube();
        const size_t ec = _inv->end_cube();

        // Update all cube icons
        for (size_t i = bc; i < ec; i++)
        {
            // Offset this index for a ui bg cube
            const ui_id ui(i);

            // Calculate icon position
            const min::vec2<float> p = pos_cube(ui);

            // Update the black bg icon
            _assets->load_bg_black(ui.bg_cube_index(), p);

            // Update the icon
            set_inventory(ui.fg_cube_index(), (*_inv)[i], p);
        }

        // Get start and end of buttons
        const size_t bb = begin_button();
        const size_t eb = end_button();

        // Update all button icons
        for (size_t i = bb; i < eb; i++)
        {
            stat_unselect(ui_id(i));
        }

        // Select selected key
        select_active(state);

        // Select clicked key
        select_click(state);

        // Select hovered key
        select_hover(state);

        // Select key
        select_key(state);
    }
    inline void set_key_down(ui_state &state, const size_t index)
    {
        // Unselect old key
        unselect_key(state);

        // Set selected index
        state.set_select(ui_id(index).to_key());

        // If select a clicked cell 'unclick it'
        if (state.is_clicking() && state.is_click_select())
        {
            state.set_clicking(false);
        }

        // Set that key is down
        state.set_selecting(true);

        // Activate the selected key
        select_key(state);
    }
    inline void set_key_down_fail(const ui_state &state, const size_t index)
    {
        const min::vec2<float> p = _assets->toolbar_position(0, index);

        // Convert index to ui_id
        const ui_id ui = ui_id(index).to_key();

        // Set background color
        _assets->load_bg_red(ui.bg_key_index(), p);
    }
    inline void set_key_up(ui_state &state, const size_t index)
    {
        // Calculate ui element position
        const min::vec2<float> p = _assets->toolbar_position(0, index);

        // Convert index to ui_id
        const ui_id ui = ui_id(index).to_key();

        // Set background color
        if (state.is_select(ui))
        {
            _assets->load_bg_yellow(ui.bg_key_index(), p);
        }
        else
        {
            _assets->load_bg_black(ui.bg_key_index(), p);
        }

        // Set that key is up
        state.set_selecting(false);
    }
    inline void set_minimized(const bool flag)
    {
        _minimized = flag;
    }
    inline void transition_state(ui_state &state)
    {
        // Set the extended state
        const ui_mode mode = state.get_mode();
        switch (mode)
        {
        case ui_mode::INV:
            state.set_mode(ui_mode::INV_EXT);
            break;
        case ui_mode::INV_EXT:
            state.set_mode(ui_mode::INV);

            // Deselect UI if in base state
            unselect_click(state);
            state.set_clicking(false);
            unselect_hover(state);
            state.set_hovering(false);
            break;
        default:
            break;
        }
    }
    inline void update(const ui_state &state, std::ostringstream &stream)
    {
        // Update the inventory matrices if dirty
        if (_inv->is_dirty())
        {
            // Get all updated slots
            const std::vector<ui_id> &updates = _inv->get_updates();

            // Update all slots
            for (auto ui : updates)
            {
                const item &it = (*_inv)[ui.index()];

                // Update item count text
                clear_stream(stream);
                stream << static_cast<int>(it.count());

                // Might require a shift in the future
                _text->set_text(ui.index(), stream.str());

                // Update the slot
                update_inv_slot(ui, it);
            }
        }

        // Update the stats
        if (_stat->is_dirty())
        {
            // Calculate text offset
            const size_t bs = _inv->begin_store();
            const size_t ec = _inv->end_cube();

            // Update attr text
            const size_t attr_off = ec - bs;
            const size_t attr_size = _stat->attr_str_size();
            for (size_t i = 0; i < attr_size; i++)
            {
                // Get attr text position
                const min::vec2<float> p = _assets->attr_position(i, _text_spacing);

                // Get text buffer offset
                const size_t index = attr_off + i;

                // Add attr string to stream
                clear_stream(stream);
                stream << _stat->attr_str(i) << ": " << _stat->attr_value(i);

                // Add attr descriptor and value text
                _text->set_text(index, stream.str(), p.x(), p.y());
            }

            // Update stat text
            const size_t stat_off = (ec - bs) + attr_size;
            const size_t stat_size = _stat->stat_str_size();
            for (size_t i = 0; i < stat_size; i++)
            {
                // Get stat text position
                const min::vec2<float> p = _assets->stat_position(i, _text_spacing);

                // Get text buffer offset
                const size_t index = stat_off + (i * 2);

                // Add stat descriptor and value text
                _text->set_text(index, _stat->stat_str(i), p.x(), p.y());

                // Get stat value
                const uint_fast16_t value = _stat->stat_value(i);

                // Create string from value
                clear_stream(stream);
                stream << value;

                // Set stat text at offset
                const float dx = stat_offset(value);
                _text->set_text(index + 1, stream.str(), p.x() + dx, p.y());
            }

            // Update buttons
            const size_t bb = begin_button();
            const size_t eb = end_button();

            // Update all button icons
            for (size_t i = bb; i < eb; i++)
            {
                const bool hovering = state.is_hovering();
                const ui_id hover = state.get_hover();
                if (!hovering || (hovering && hover.index() != i))
                {
                    stat_unselect(ui_id(i));
                }
            }

            // Update cached player level
            _inv->set_player_level(_stat->level());
        }

        // Upload text if updated
        if (_inv->is_dirty() || _stat->is_dirty())
        {
            // Flag inventory clean state
            _inv->clean();

            // Flag stat clean state
            _stat->clean();

            // Upload the new text
            upload_text();
        }
    }
};
}

#endif
