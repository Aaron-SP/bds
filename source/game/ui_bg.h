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
#ifndef _UI_BACKGROUND__
#define _UI_BACKGROUND__

#include <game/id.h>
#include <game/inventory.h>
#include <game/memory_map.h>
#include <game/stats.h>
#include <game/ui_bg_assets.h>
#include <game/ui_vertex.h>
#include <game/uniforms.h>
#include <min/aabbox.h>
#include <min/dds.h>
#include <min/grid.h>
#include <min/program.h>
#include <min/shader.h>
#include <min/text_buffer.h>
#include <min/texture_buffer.h>
#include <min/vec2.h>
#include <min/vertex_buffer.h>
#include <sstream>
#include <utility>

namespace game
{

class ui_bg
{
  private:
    static constexpr size_t _border = 6;
    static constexpr size_t _text_spacing = _ui_font_size + _border;
    static constexpr size_t _button_size = stats::stat_str_size() - 1;
    inline static constexpr size_t begin_button()
    {
        return 49;
    }
    inline static constexpr size_t end_button()
    {
        return begin_button() + _button_size;
    }

    // OpenGL stuff
    min::shader _vertex;
    min::shader _fragment;
    min::program _prog;
    const GLint _index_location;

    // Instance buffer stuff
    min::vertex_buffer<float, uint32_t, ui_vertex, GL_FLOAT, GL_UNSIGNED_INT> _vb;
    size_t _mesh_id;

    // Texture stuff
    min::texture_buffer _tbuffer;
    GLuint _title_id;
    GLuint _ui_id;

    // Misc
    bool _clicking;
    ui_id _click;
    bool _focus;
    bool _hovering;
    bool _minimized;
    ui_id _hover;
    ui_id _select;

    // Background assets
    ui_bg_assets _assets;
    inventory *const _inv;
    stats *const _stat;
    min::text_buffer *const _text;
    const std::string _invalid_str;

    // Click detection
    std::vector<min::aabbox<float, min::vec2>> _shapes;
    min::grid<float, uint8_t, uint8_t, min::vec2, min::aabbox, min::aabbox> _grid;
    std::ostringstream _stream;

    inline bool action(const ui_id ui, const uint8_t mult)
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
    inline void clear_stream()
    {
        _stream.clear();
        _stream.str(std::string());
    }
    inline void draw_focus_ui() const
    {
        // Get the start of the opaque ui
        const size_t start = _assets.focus_start();
        set_start_index(start);

        // Draw extended ui elements
        const size_t size = (_assets.get_focus_bar()) ? _assets.focus_bar_size() : _assets.focus_size();
        _vb.draw_many(GL_TRIANGLES, _mesh_id, size);
    }
    inline void draw_opaque_extend() const
    {
        // Get the start of the opaque ui
        const size_t start = _assets.opaque_start();
        set_start_index(start);

        // Draw extended ui elements
        const size_t size = _assets.opaque_ext_size();
        _vb.draw_many(GL_TRIANGLES, _mesh_id, size);
    }
    inline void draw_opaque_base() const
    {
        // Get the start of the opaque ui
        const size_t start = _assets.opaque_start();
        set_start_index(start);

        // Draw base ui elements
        const size_t size = _assets.opaque_base_size();
        _vb.draw_many(GL_TRIANGLES, _mesh_id, size);
    }
    inline void draw_title() const
    {
        // Draw the first thing in the buffer, title screen
        _vb.draw_many(GL_TRIANGLES, _mesh_id, 1);
    }
    inline void draw_tooltip_ui() const
    {
        // Get the start of the opaque ui
        const size_t start = _assets.tooltip_start();
        set_start_index(start);

        // Draw extended ui elements
        const size_t size = _assets.tooltip_size();
        _vb.draw_many(GL_TRIANGLES, _mesh_id, size);
    }
    inline void draw_transparent_ui() const
    {
        // Get the start of the transparent ui
        const size_t trans_start = _assets.transparent_start();
        set_start_index(trans_start);

        // Draw the base ui elements
        const size_t size = _assets.transparent_size();
        _vb.draw_many(GL_TRIANGLES, _mesh_id, size);
    }
    inline void load_base_rect()
    {
        // Cached parent mesh
        min::mesh<float, uint32_t> rect("ui");

        // Append vertices
        rect.vertex.insert(
            rect.vertex.end(),
            std::initializer_list<min::vec4<float>>{
                min::vec4<float>(-0.5, -0.5, 1.0, 1.0),
                min::vec4<float>(-0.5, 0.5, 1.0, 1.0),
                min::vec4<float>(0.5, -0.5, 1.0, 1.0),
                min::vec4<float>(0.5, 0.5, 1.0, 1.0)});

        // Append UV's for the box
        rect.uv.insert(
            rect.uv.end(),
            std::initializer_list<min::vec2<float>>{
                min::vec2<float>(0.0, 0.0),
                min::vec2<float>(0.0, 1.0),
                min::vec2<float>(1.0, 0.0),
                min::vec2<float>(1.0, 1.0)});

        // Append indices
        rect.index.insert(
            rect.index.end(),
            std::initializer_list<uint32_t>{
                0, 1, 2,
                2, 1, 3});

        // Add rect mesh to the buffer
        _mesh_id = _vb.add_mesh(rect);

        // Unbind the last VAO to prevent scrambling buffers
        _vb.unbind();

        // Upload the text glyphs to the GPU
        _vb.upload();
    }
    inline void load_button()
    {
        // Get start and end of buttons
        const size_t bb = begin_button();
        const size_t eb = end_button();

        // Update all button icons
        for (size_t i = bb; i < eb; i++)
        {
            stat_unselect(ui_id(i));
        }
    }
    inline void load_cube()
    {
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
            _assets.load_bg_black(ui.bg_cube_index(), p);

            // Update the icon
            set_inventory(ui.cube_index(), (*_inv)[i], p);
        }
    }
    inline void load_extend()
    {
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
            _assets.load_bg_black(ui.bg_ext_index(), p);

            // Update the icon
            set_inventory(ui.ext_index(), (*_inv)[i], p);
        }
    }
    inline void load_key()
    {
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
            _assets.load_bg_black(ui.bg_key_index(), p);

            // Update the icon
            set_inventory(ui.key_index(), (*_inv)[i], p);
        }
    }
    inline void load_store()
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
            _assets.load_bg_black(ui.bg_store_index(), p);

            // Update the icon
            set_inventory(ui.store_index(), (*_inv)[i], p);
        }
    }
    inline void load_extended_gui()
    {
        // Load store
        load_store();

        // Load keys
        load_key();

        // Load extend
        load_extend();

        // Load stat background
        _assets.load_bg_stat();

        // Load cube
        load_cube();

        // Load buttons
        load_button();

        // Select selected key
        select_active();

        // Select hovered key
        select_hover();
    }
    inline void load_grid(const uint16_t width, const uint16_t height)
    {
        // Clear the shapes buffer
        _shapes.clear();
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
            _shapes.push_back(_assets.inv_box(p));

            // Clear and reset the stream
            clear_stream();

            // Update item count
            _stream << static_cast<int>((*_inv)[i].count());

            // Add text for each box
            _text->add_text(_stream.str(), p.x() + _item_count_dx, p.y() + _item_count_dy);
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
            _shapes.push_back(_assets.inv_box(p));

            // Clear and reset the stream
            clear_stream();

            // Update item count
            _stream << static_cast<int>((*_inv)[i].count());

            // Add text for each box
            _text->add_text(_stream.str(), p.x() + _item_count_dx, p.y() + _item_count_dy);
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
            _shapes.push_back(_assets.inv_box(p));

            // Clear and reset the stream
            clear_stream();

            // Update item count
            _stream << static_cast<int>((*_inv)[i].count());

            // Add text for each box
            _text->add_text(_stream.str(), p.x() + _item_count_dx, p.y() + _item_count_dy);
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
            _shapes.push_back(_assets.inv_box(p));

            // Clear and reset the stream
            clear_stream();

            // Update item count
            _stream << static_cast<int>((*_inv)[i].count());

            // Add text for each box
            _text->add_text(_stream.str(), p.x() + _item_count_dx, p.y() + _item_count_dy);
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
            _shapes.push_back(_assets.stat_box(p));
        }

        // Add attr text
        const size_t attr_size = _stat->attr_str_size();
        for (size_t i = 0; i < attr_size; i++)
        {
            // Get attr text position
            const min::vec2<float> p = _assets.attr_position(i, _text_spacing);

            // Add attr string to stream
            clear_stream();
            _stream << _stat->attr_str(i) << ": " << _stat->attr_value(i);

            // Set attr text at offset
            _text->add_text(_stream.str(), p.x(), p.y());
        }

        // Add stat text
        const size_t stat_size = _stat->stat_str_size();
        for (size_t i = 0; i < stat_size; i++)
        {
            // Get stat text position
            const min::vec2<float> p = _assets.stat_position(i, _text_spacing);

            // Add stat descriptor and value text
            _text->add_text(_stat->stat_str(i), p.x(), p.y());

            // Get stat value
            const uint16_t value = _stat->stat_value(i);

            // Create string from value
            clear_stream();
            _stream << value;

            // Set stat text at offset
            const float dx = stat_offset(value);
            _text->add_text(_stream.str(), p.x() + dx, p.y());
        }

        // Calculate the grid scale
        const uint16_t scale = width / 24;

        // Insert the shapes into the grid
        _grid.insert(_shapes, scale);

        // Upload text for display
        upload_text();
    }
    inline GLint load_program_index(const uniforms &uniforms) const
    {
        // Load the uniform buffer with program we will use
        uniforms.set_program_matrix(_prog);

        // Get the start_index uniform location
        const GLint location = glGetUniformLocation(_prog.id(), "start_index");
        if (location == -1)
        {
            throw std::runtime_error("ui_bg: could not find uniform 'start_index'");
        }

        return location;
    }
    inline void load_texture()
    {
        // Load the UI texture
        {
            // Load texture
            const min::mem_file &ui = memory_map::memory.get_file("data/texture/ui.dds");
            const min::dds tex(ui);

            // Load texture into texture buffer
            _ui_id = _tbuffer.add_dds_texture(tex);
        }

        // Load the title screen texture
        {
            // Load texture
            const min::mem_file &title = memory_map::memory.get_file("data/texture/title.dds");
            const min::dds tex(title);

            // Load texture into texture buffer
            _title_id = _tbuffer.add_dds_texture(tex);
        }
    }
    inline min::vec2<float> pos_button(const ui_id ui) const
    {
        // Get row and col
        const unsigned row = ui.index() - begin_button();
        const unsigned col = 0;

        // Calculate ui element position
        return _assets.button_position(row, col);
    }
    inline min::vec2<float> pos_cube(const ui_id ui) const
    {
        // Get row and col
        const unsigned row = ui.row3();
        const unsigned col = ui.col3();

        // Calculate ui element position
        return _assets.cube_position(row, col);
    }
    inline min::vec2<float> pos_ext(const ui_id ui) const
    {
        // Get row and col
        const unsigned row = ui.row8() + 1;
        const unsigned col = ui.col8();

        // Calculate ui element position
        return _assets.toolbar_position(row, col);
    }
    inline min::vec2<float> pos_key(const ui_id ui) const
    {
        // Get row and col
        const unsigned row = 0;
        const unsigned col = ui.col8();

        // Calculate ui element position
        return _assets.toolbar_position(row, col);
    }
    inline min::vec2<float> pos_store(const ui_id ui) const
    {
        // Get row and col
        const unsigned row = 0;
        const unsigned col = ui.col8();

        // Calculate ui element position
        return _assets.store_position(row, col);
    }
    inline void position_ui(const min::vec2<float> &p)
    {
        // Load overlay
        if (_assets.get_draw_title())
        {
            _assets.load_title_overlay();
        }
        else
        {
            _assets.load_health_overlay();
        }

        // Load console background
        _assets.load_console_bg();

        // Load cursor
        if (_assets.get_draw_dead())
        {
            _assets.load_menu_dead();
        }
        else if (_assets.get_draw_pause())
        {
            _assets.load_menu_pause();
        }
        else if (_assets.get_draw_reload())
        {
            _assets.load_cursor_reload();
        }
        else if (_assets.get_draw_target())
        {
            _assets.load_cursor_target();
        }
        else
        {
            // Load aim cursor
            _assets.load_cursor_aim();
        }

        // Load health bar
        _assets.load_health_bar();

        // Load energy bar
        _assets.load_energy_bar();

        // Load energy meter
        _assets.load_energy_meter();

        // Load experience meter
        _assets.load_exp_meter();

        // Load oxygen meter
        _assets.load_oxy_meter();

        // Load health meter
        _assets.load_health_meter();

        // Load the extended gui
        load_extended_gui();

        // Load focus background
        _assets.load_bg_focus();

        // Load focus meter
        _assets.load_focus_meter();

        // Load hover background
        _assets.load_bg_hover(p);
    }
    inline static min::aabbox<float, min::vec2> screen_box(const uint16_t width, const uint16_t height)
    {
        // Create a box from the screen
        const min::vec2<float> min(0, 0);
        const min::vec2<float> max(width, height);

        // Return the screen in a 2D box
        return min::aabbox<float, min::vec2>(min, max);
    }
    inline void select()
    {
        // Bg key placement
        const min::vec2<float> active = _assets.toolbar_position(0, _select.col8());

        // Set activated index color to white
        _assets.load_bg_white(_select.bg_key_index(), active);
    }
    inline void select_active()
    {
        // Bg key placement
        const min::vec2<float> active = _assets.toolbar_position(0, _select.col8());

        // Set activated index color to white
        _assets.load_bg_yellow(_select.bg_key_index(), active);
    }
    inline void select_click()
    {
        // Determine inventory type to update bg color
        const ui_type type = _click.type();
        switch (type)
        {
        case ui_type::button:
            return stat_select(_click);
        case ui_type::cube:
            return _assets.load_bg_light_blue(_click.bg_cube_index(), pos_cube(_click));
        case ui_type::extend:
            return _assets.load_bg_light_blue(_click.bg_ext_index(), pos_ext(_click));
        case ui_type::key:
            return _assets.load_bg_light_blue(_click.bg_key_index(), pos_key(_click));
        case ui_type::store:
            return _assets.load_bg_light_blue(_click.bg_store_index(), pos_store(_click));
        default:
            break;
        }
    }
    inline void select_hover()
    {
        // Are we hovering?
        if (_hovering)
        {
            // Determine inventory type to update bg color
            const ui_type type = _hover.type();
            if (_clicking)
            {
                switch (type)
                {
                case ui_type::button:
                    return stat_hover(_hover);
                case ui_type::cube:
                    return _assets.load_bg_white(_hover.bg_cube_index(), pos_cube(_hover));
                case ui_type::extend:
                    return _assets.load_bg_white(_hover.bg_ext_index(), pos_ext(_hover));
                case ui_type::key:
                    return _assets.load_bg_white(_hover.bg_key_index(), pos_key(_hover));
                case ui_type::store:
                    return _assets.load_bg_white(_hover.bg_store_index(), pos_store(_hover));
                default:
                    break;
                }
            }
            else
            {
                switch (type)
                {
                case ui_type::button:
                    return stat_hover(_hover);
                case ui_type::cube:
                    return _assets.load_bg_yellow(_hover.bg_cube_index(), pos_cube(_hover));
                case ui_type::extend:
                    return _assets.load_bg_yellow(_hover.bg_ext_index(), pos_ext(_hover));
                case ui_type::key:
                    return _assets.load_bg_yellow(_hover.bg_key_index(), pos_key(_hover));
                case ui_type::store:
                    return _assets.load_bg_yellow(_hover.bg_store_index(), pos_store(_hover));
                default:
                    break;
                }
            }
        }
    }
    inline bool set_click_down(const ui_id ui)
    {
        // If we are clicking a button
        if (ui.type() == ui_type::button)
        {
            // If clicking reset it
            if (_clicking)
            {
                unselect_click();
            }

            // Set selected index
            _click = ui;

            // Select hover
            select_click();

            // Set clicking
            _clicking = true;

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
        else if (_clicking && _click == ui)
        {
            // If we are unclicking
            unselect_click();

            // Unset clicking
            _clicking = false;
        }
        else if (_clicking)
        {
            // Unselect old click
            unselect_click();

            // If not a button
            if (_click.type() != ui_type::button)
            {
                // Swap inventory
                _inv->swap(_click.index(), ui.index());

                // Disable clicking
                _clicking = false;
            }
        }
        else
        {
            // Set selected index
            _click = ui;

            // Select hover
            select_click();

            // Set clicking
            _clicking = true;
        }

        // A click happened
        return true;
    }
    inline void set_hover_down(const ui_id ui)
    {
        // Already hovering here?
        if (_hovering && _hover == ui)
        {
            return;
        }

        // Unselect hover
        unselect_hover();

        // Set selected index
        _hover = ui;

        // Set hovering
        _hovering = true;

        // If key is not selected or clicked change color
        const bool clicked = (_clicking && _click == ui);
        if (!clicked && _select != ui)
        {
            // Select hover
            select_hover();
        }
    }
    inline void set_inventory(const ui_id ui, const item &it, const min::vec2<float> &p)
    {
        // Draw key icon overlay based on type
        const item_type type = it.type();
        switch (type)
        {
        case item_type::empty:
            return _assets.load_empty_icon(ui, p);
        case item_type::skill:
            switch (it.id())
            {
            case item_id::AUTO_BEAM:
                return _assets.load_auto_icon(ui, p);
            case item_id::BEAM:
                return _assets.load_beam_icon(ui, p);
            case item_id::CHARGE:
                return _assets.load_charge_icon(ui, p);
            case item_id::GRAPPLE:
                return _assets.load_grapple_icon(ui, p);
            case item_id::GRENADE:
                return _assets.load_grenade_icon(ui, p);
            case item_id::JET:
                return _assets.load_jet_icon(ui, p);
            case item_id::MISSILE:
                return _assets.load_missile_icon(ui, p);
            case item_id::PORTAL:
                return _assets.load_portal_icon(ui, p);
            case item_id::SCAN:
                return _assets.load_scan_icon(ui, p);
            case item_id::SCATTER:
                return _assets.load_scatter_icon(ui, p);
            case item_id::SPEED:
                return _assets.load_speed_icon(ui, p);
            default:
                return;
            }
        case item_type::block:
            return _assets.load_block_icon(ui, it.to_block_id(), p);
        case item_type::item:
            return _assets.load_item_icon(ui, it.to_item_id(), p);
        }
    }
    inline void set_start_index(const GLint start_index) const
    {
        // Set the sampler active texture
        glUniform1i(_index_location, start_index);
    }
    inline void stat_select(const ui_id ui)
    {
        if (_stat->has_stat_points())
        {
            return _assets.load_stat_click(ui.button_index(), pos_button(ui));
        }

        return _assets.load_stat_grey(ui.button_index(), pos_button(ui));
    }
    inline void stat_hover(const ui_id ui)
    {
        if (_stat->has_stat_points())
        {
            return _assets.load_stat_hover(ui.button_index(), pos_button(ui));
        }

        return _assets.load_stat_grey(ui.button_index(), pos_button(ui));
    }
    inline void stat_unselect(const ui_id ui)
    {
        if (_stat->has_stat_points())
        {
            return _assets.load_stat_red(ui.button_index(), pos_button(ui));
        }

        return _assets.load_stat_grey(ui.button_index(), pos_button(ui));
    }
    inline static constexpr float stat_offset(const uint16_t value)
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
    inline void unselect()
    {
        // Bg key placement
        const min::vec2<float> prev = _assets.toolbar_position(0, _select.col8());

        // Set previous unselected color to black
        _assets.load_bg_black(_select.bg_key_index(), prev);
    }
    inline void unselect_click()
    {
        if (_click == _select)
        {
            select_active();
        }
        else if (_click == _hover)
        {
            select_hover();
        }
        else
        {
            // Determine inventory type to update bg color
            const ui_type type = _click.type();
            switch (type)
            {
            case ui_type::button:
                return stat_unselect(_click);
            case ui_type::cube:
                return _assets.load_bg_black(_click.bg_cube_index(), pos_cube(_click));
            case ui_type::extend:
                return _assets.load_bg_black(_click.bg_ext_index(), pos_ext(_click));
            case ui_type::key:
                return _assets.load_bg_black(_click.bg_key_index(), pos_key(_click));
            case ui_type::store:
                return _assets.load_bg_black(_click.bg_store_index(), pos_store(_click));
            default:
                break;
            }
        }
    }
    inline void unselect_hover()
    {
        // Are we hovering?
        if (_hovering)
        {
            // Don't deselect an active click or select icon
            const bool skip = (_clicking && _hover == _click) || _hover == _select;
            if (!skip)
            {
                // Determine inventory type to update bg color
                const ui_type type = _hover.type();
                switch (type)
                {
                case ui_type::button:
                    return stat_unselect(_hover);
                case ui_type::cube:
                    return _assets.load_bg_black(_hover.bg_cube_index(), pos_cube(_hover));
                case ui_type::extend:
                    return _assets.load_bg_black(_hover.bg_ext_index(), pos_ext(_hover));
                case ui_type::key:
                    return _assets.load_bg_black(_hover.bg_key_index(), pos_key(_hover));
                case ui_type::store:
                    return _assets.load_bg_black(_hover.bg_store_index(), pos_store(_hover));
                default:
                    break;
                }
            }
        }
    }
    inline void update_inv_slot(const ui_id ui, const item &it)
    {
        // Determine inventory type to update icon
        const ui_type type = ui.type();
        switch (type)
        {
        case ui_type::cube:
            return set_inventory(ui.cube_index(), it, pos_cube(ui));
        case ui_type::extend:
            return set_inventory(ui.ext_index(), it, pos_ext(ui));
        case ui_type::key:
            return set_inventory(ui.key_index(), it, pos_key(ui));
        case ui_type::store:
            return set_inventory(ui.store_index(), it, pos_store(ui));
        default:
            break;
        }
    }

  public:
    ui_bg(const uniforms &uniforms, inventory &inv, stats &stat, min::text_buffer &text, const uint16_t width, const uint16_t height)
        : _vertex(memory_map::memory.get_file("data/shader/ui.vertex"), GL_VERTEX_SHADER),
          _fragment(memory_map::memory.get_file("data/shader/ui.fragment"), GL_FRAGMENT_SHADER),
          _prog(_vertex, _fragment), _index_location(load_program_index(uniforms)), _mesh_id(0),
          _clicking(false), _click(0), _focus(false), _hovering(false), _minimized(false), _hover(0), _select(inv.begin_key()),
          _assets(width, height), _inv(&inv), _stat(&stat), _text(&text), _invalid_str("Invalid"), _grid(screen_box(width, height))
    {
        // Format string stream
        _stream.precision(3);

        // Create the instance rectangle
        load_base_rect();

        // Load texture
        load_texture();

        // Reserve shape memory
        _shapes.reserve(_inv->size());

        // Load the grid inventory boxes
        load_grid(width, height);

        // Reposition all ui on the screen
        position_ui(min::vec2<float>());
    }
    inline std::pair<bool, ui_id> action_hover(const uint8_t mult)
    {
        if (_hover.type() != ui_type::button && _hovering && !_minimized)
        {
            // Do action on hovering index
            return std::make_pair(action(_hover, mult), _hover);
        }

        // No action
        return std::make_pair(false, _hover);
    }
    inline std::pair<bool, ui_id> action_select(const uint8_t mult)
    {
        if (_select.type() != ui_type::button)
        {
            // Do action on selected index
            return std::make_pair(action(_select, mult), _select);
        }

        // No action
        return std::make_pair(false, _select);
    }
    inline size_t bg_text_size() const
    {
        // If extended draw
        if (_assets.get_draw_ex())
        {
            return _text->size();
        }
        else if (_assets.get_draw_title())
        {
            // Do not draw anything
            return 0;
        }

        // If not extended
        return _inv->end_key();
    }
    inline bool click_down()
    {
        // Click on the currently hovered icon
        if (_hovering && !_minimized)
        {
            // Click happened
            return set_click_down(_hover);
        }

        return false;
    }
    inline void click_up()
    {
        const ui_type type = _click.type();
        if (_clicking && type == ui_type::button)
        {
            // Deselect button on click up
            unselect_click();

            // Unset clicking
            _clicking = false;
        }
    }
    inline void draw_opaque() const
    {
        // Bind the text_buffer vao
        _vb.bind();

        // Bind the ui program
        _prog.use();

        // If we are drawing the title screen
        if (_assets.get_draw_title())
        {
            // Bind the ui texture for drawing
            _tbuffer.bind(_title_id, 0);

            // Draw title
            draw_title();
        }
        else if (_assets.get_draw_ex())
        {
            // Bind the ui texture for drawing
            _tbuffer.bind(_ui_id, 0);

            // Draw extended ui?
            draw_opaque_extend();
        }
        else
        {
            // Bind the ui texture for drawing
            _tbuffer.bind(_ui_id, 0);

            // Draw only base ui
            draw_opaque_base();
        }

        // Draw focus UI
        if (_focus)
        {
            draw_focus_ui();
        }
    }
    inline void draw_tooltips() const
    {
        const bool draw = _assets.get_draw_ex() && _hovering && _hover.type() != ui_type::button;
        if (draw)
        {
            // Bind the text_buffer vao
            _vb.bind();

            // Bind the ui program
            _prog.use();

            // Bind the ui texture for drawing
            _tbuffer.bind(_ui_id, 0);

            // Draw tooltips
            draw_tooltip_ui();
        }
    }
    inline void draw_transparent() const
    {
        if (!_assets.get_draw_title())
        {
            // Bind the text_buffer vao
            _vb.bind();

            // Bind the ui program
            _prog.use();

            // Bind the ui texture for drawing
            _tbuffer.bind(_ui_id, 0);

            // Draw transparent ui
            draw_transparent_ui();
        }
    }
    inline bool drop()
    {
        // If we are hovering
        if (_hovering && _hover.type() != ui_type::button)
        {
            // Drop hovering inventory
            _inv->drop(_hover.index());

            // Return that we dropped the item
            return true;
        }

        return false;
    }
    inline const std::string &get_hover_info() const
    {
        return (_hover.type() != ui_type::button) ? _inv->get_info((*_inv)[_hover.index()].id()) : _invalid_str;
    }
    inline const std::string &get_hover_name() const
    {
        return (_hover.type() != ui_type::button) ? _inv->get_name((*_inv)[_hover.index()].id()) : _invalid_str;
    }
    inline const std::vector<min::mat3<float>> &get_scale() const
    {
        return _assets.get_scale();
    }
    inline ui_id get_selected() const
    {
        return _select;
    }
    inline const stats &get_stats() const
    {
        return *_stat;
    }
    inline const std::vector<min::mat3<float>> &get_uv() const
    {
        return _assets.get_uv();
    }
    inline bool is_extended() const
    {
        return _assets.get_draw_ex();
    }
    inline bool is_focused() const
    {
        return _focus;
    }
    inline std::pair<bool, ui_id> overlap(const min::vec2<float> &p)
    {
        // Is the inventory open?
        if (is_extended() && !_minimized)
        {
            // Bad point
            if (!_grid.inside(p))
            {
                return std::make_pair(false, 0);
            }

            // Search for overlapping cells
            const std::vector<uint8_t> &map = _grid.get_index_map();
            const std::vector<uint8_t> &hits = _grid.point_inside(p);

            ui_id id(0);

            // Set hover if overlapping
            bool hit = false;
            for (auto &h : hits)
            {
                if (_shapes[map[h]].point_inside(p))
                {
                    // Flag that we hit something
                    hit = true;

                    // Cache id
                    id = ui_id(map[h]);

                    // Highlight the inventory cell
                    set_hover_down(id);
                    break;
                }
            }

            // If he didn't hit anything
            if (!hit)
            {
                // Unselect hover
                unselect_hover();

                // Reset the hover index
                _hovering = false;

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
                _assets.load_bg_hover(p);

                // Overlapping a UI element
                return std::make_pair(true, id);
            }
        }

        // Not overlapping a UI element
        return std::make_pair(false, 0);
    }
    inline void reset_menu()
    {
        // Turn off drawing the dead or pause
        _assets.set_draw_aim();

        // Set the cursor to aim
        _assets.load_cursor_aim();
    }
    inline void respawn()
    {
        // Turn off showing menu
        reset_menu();
    }
    inline void set_cursor_aim()
    {
        if (!_assets.get_draw_menu())
        {
            _assets.set_draw_aim();
            _assets.load_cursor_aim();
        }
    }
    inline void set_cursor_reload()
    {
        if (!_assets.get_draw_menu())
        {
            _assets.set_draw_reload();
            _assets.load_cursor_reload();
        }
    }
    inline void set_cursor_target()
    {
        if (!_assets.get_draw_menu())
        {
            _assets.set_draw_target();
            _assets.load_cursor_target();
        }
    }
    inline void set_draw_console(const bool flag)
    {
        _assets.set_draw_console(flag);

        // Reload console data
        _assets.load_console_bg();
    }
    inline void set_draw_focus(const bool flag)
    {
        _focus = flag;
    }
    inline void set_draw_title(const bool flag)
    {
        _assets.set_draw_title(flag);

        // Set the overlay
        if (_assets.get_draw_title())
        {
            _assets.load_title_overlay();
        }
        else
        {
            _assets.load_health_overlay();
        }
    }
    inline void set_energy(const float energy)
    {
        _assets.set_energy(energy);
    }
    inline void set_exp(const float exp)
    {
        _assets.set_experience(exp);
    }
    inline void set_focus(const float bar)
    {
        _assets.set_focus_bar(bar);
    }
    inline void set_oxygen(const float oxy)
    {
        _assets.set_oxygen(oxy);
    }
    inline void set_health(const float health)
    {
        _assets.set_health(health);
    }
    inline void set_key_down(const size_t index)
    {
        // Unselect old key
        unselect();

        // Set selected index
        _select = ui_id(index).to_key();

        // If select a clicked cell 'unclick it'
        if (_clicking && _select == _click)
        {
            _clicking = false;
        }

        // Activate the selected key
        select();
    }
    inline void set_key_down_fail(const size_t index)
    {
        const min::vec2<float> p = _assets.toolbar_position(0, index);

        // Convert index to ui_id
        const ui_id ui = ui_id(index).to_key();

        // Set background color
        _assets.load_bg_red(ui.bg_key_index(), p);
    }
    inline void set_key_up(const size_t index)
    {
        // Calculate ui element position
        const min::vec2<float> p = _assets.toolbar_position(0, index);

        // Convert index to ui_id
        const ui_id ui = ui_id(index).to_key();

        // Set background color
        if (_select.id() == ui.id())
        {
            _assets.load_bg_yellow(ui.bg_key_index(), p);
        }
        else
        {
            _assets.load_bg_black(ui.bg_key_index(), p);
        }
    }
    inline void set_menu_dead()
    {
        // Draw the menu
        _assets.set_draw_dead();

        // Show the dead menu
        _assets.load_menu_dead();
    }
    inline void set_menu_pause()
    {
        // Draw the menu
        _assets.set_draw_pause();

        // Show the pause menu
        _assets.load_menu_pause();
    }
    inline void set_minimized(const bool flag)
    {
        _minimized = flag;
    }
    inline void set_screen(const min::vec2<float> &p, const uint16_t width, const uint16_t height)
    {
        // Set asset screen size
        _assets.set_screen(width, height);
        _text->set_screen(width, height);

        // Reposition all ui on the screen
        position_ui(p);

        // Resize the screen grid box
        _grid.resize(screen_box(width, height));

        // Load grid inventory boxes
        load_grid(width, height);
    }
    inline void toggle_draw_console()
    {
        _assets.toggle_draw_console();

        // Reload console data
        _assets.load_console_bg();
    }
    inline void toggle_draw_ex()
    {
        // Toggle draw_ex flag
        _assets.toggle_draw_ex();

        // If we are closing the extended overlay
        if (!_assets.get_draw_ex())
        {
            // Untoggle hover and select
            unselect_hover();
            unselect_click();
        }
    }
    inline void update()
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
                clear_stream();
                _stream << static_cast<int>(it.count());

                // Might require a shift in the future
                _text->set_text(ui.index(), _stream.str());

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
                const min::vec2<float> p = _assets.attr_position(i, _text_spacing);

                // Get text buffer offset
                const size_t index = attr_off + i;

                // Add attr string to stream
                clear_stream();
                _stream << _stat->attr_str(i) << ": " << _stat->attr_value(i);

                // Add attr descriptor and value text
                _text->set_text(index, _stream.str(), p.x(), p.y());
            }

            // Update stat text
            const size_t stat_off = (ec - bs) + attr_size;
            const size_t stat_size = _stat->stat_str_size();
            for (size_t i = 0; i < stat_size; i++)
            {
                // Get stat text position
                const min::vec2<float> p = _assets.stat_position(i, _text_spacing);

                // Get text buffer offset
                const size_t index = stat_off + (i * 2);

                // Add stat descriptor and value text
                _text->set_text(index, _stat->stat_str(i), p.x(), p.y());

                // Get stat value
                const uint16_t value = _stat->stat_value(i);

                // Create string from value
                clear_stream();
                _stream << value;

                // Set stat text at offset
                const float dx = stat_offset(value);
                _text->set_text(index + 1, _stream.str(), p.x() + dx, p.y());
            }

            // Update buttons
            const size_t bb = begin_button();
            const size_t eb = end_button();

            // Update all button icons
            for (size_t i = bb; i < eb; i++)
            {
                if (!_hovering || (_hovering && _hover.index() != i))
                {
                    stat_unselect(ui_id(i));
                }
            }
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
    inline void upload_text() const
    {
        // Unbind the last VAO to prevent scrambling buffers
        _text->unbind();

        // Upload the text glyphs to the GPU
        _text->upload();
    }
};
}

#endif
