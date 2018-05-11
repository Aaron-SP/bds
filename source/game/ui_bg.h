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

namespace game
{

class ui_bg
{
  private:
    constexpr static size_t _text_size = 20;
    constexpr static float _inv_ui_dx = (_s_fg - _ui_font_size) * 0.5;
    constexpr static float _inv_ui_dy = -(_s_fg / 2.0);

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
    inv_id _click;
    bool _hovering;
    bool _minimized;
    inv_id _hover;
    inv_id _select;

    // Background assets
    ui_bg_assets _assets;
    inventory *const _inv;
    stats *const _stat;
    min::text_buffer *const _text;

    // Click detection
    std::vector<min::aabbox<float, min::vec2>> _shapes;
    min::grid<float, uint16_t, uint16_t, min::vec2, min::aabbox, min::aabbox> _grid;
    std::ostringstream _stream;

    inline bool action(const size_t index, const uint8_t mult)
    {
        // Choose between crafting and decaying
        const std::pair<bool, uint8_t> p = (index >= _inv->begin_cube() && index < _inv->end_cube())
                                               ? _inv->craft(index, mult)
                                               : _inv->decay(index, mult);

        // If decaying consumables add stat points
        const item_id it_id = static_cast<item_id>(p.second);
        if (p.first)
        {
            switch (it_id)
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
    inline void draw_opaque_extend() const
    {
        // Bind the ui texture for drawing
        _tbuffer.bind(_ui_id, 0);

        // Get the start of the opaque ui
        const size_t start = _assets.opaque_start();
        set_start_index(start);

        // Draw extended ui elements
        const size_t size = _assets.opaque_ext_size();
        _vb.draw_many(GL_TRIANGLES, _mesh_id, size);
    }
    inline void draw_opaque_base() const
    {
        // Bind the ui texture for drawing
        _tbuffer.bind(_ui_id, 0);

        // Get the start of the opaque ui
        const size_t start = _assets.opaque_start();
        set_start_index(start);

        // Draw base ui elements
        const size_t size = _assets.opaque_base_size();
        _vb.draw_many(GL_TRIANGLES, _mesh_id, size);
    }
    inline void draw_title() const
    {
        // Bind the ui texture for drawing
        _tbuffer.bind(_title_id, 0);

        // Draw the first thing in the buffer, title screen
        _vb.draw_many(GL_TRIANGLES, _mesh_id, 1);
    }
    inline void draw_tooltip_ui() const
    {
        // Bind the ui texture for drawing
        _tbuffer.bind(_ui_id, 0);

        // Get the start of the opaque ui
        const size_t start = _assets.tooltip_start();
        set_start_index(start);

        // Draw extended ui elements
        const size_t size = _assets.tooltip_size();
        _vb.draw_many(GL_TRIANGLES, _mesh_id, size);
    }
    inline void draw_transparent_ui() const
    {
        // Bind the ui texture for drawing
        _tbuffer.bind(_ui_id, 0);

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
            // Offset this index for a ui bg key
            const inv_id inv = inv_id(i);

            // Get ui position
            const min::vec2<float> p = pos_store(inv);

            // Add shape to buffer
            _shapes.push_back(_assets.inv_box(p));

            // Clear and reset the stream
            clear_stream();

            // Update item count
            _stream << static_cast<int>((*_inv)[i].count());

            // Add text for each box
            _text->add_text(_stream.str(), p.x() + _inv_ui_dx, p.y() + _inv_ui_dy);
        }

        // Get start and end of keys
        const size_t bk = _inv->begin_key();
        const size_t ek = _inv->end_key();

        // First row
        for (size_t i = bk; i < ek; i++)
        {
            // Offset this index for a ui bg key
            const inv_id inv = inv_id(i);

            // Get ui position
            const min::vec2<float> p = pos_key(inv);

            // Add shape to buffer
            _shapes.push_back(_assets.inv_box(p));

            // Clear and reset the stream
            clear_stream();

            // Update item count
            _stream << static_cast<int>((*_inv)[i].count());

            // Add text for each box
            _text->add_text(_stream.str(), p.x() + _inv_ui_dx, p.y() + _inv_ui_dy);
        }

        // Get start and end of keys
        const size_t be = _inv->begin_extend();
        const size_t ee = _inv->end_extend();

        // Extended rows
        for (size_t i = be; i < ee; i++)
        {
            // Offset this index for a ui bg key
            const inv_id inv = inv_id(i);

            // Get ui extended position
            const min::vec2<float> p = pos_ext(inv);

            // Add shape to buffer
            _shapes.push_back(_assets.inv_box(p));

            // Clear and reset the stream
            clear_stream();

            // Update item count
            _stream << static_cast<int>((*_inv)[i].count());

            // Add text for each box
            _text->add_text(_stream.str(), p.x() + _inv_ui_dx, p.y() + _inv_ui_dy);
        }

        // Get start and end of keys
        const size_t bc = _inv->begin_cube();
        const size_t ec = _inv->end_cube();

        // Extended rows
        for (size_t i = bc; i < ec; i++)
        {
            // Offset this index for a ui bg key
            const inv_id inv = inv_id(i);

            // Get ui extended position
            const min::vec2<float> p = pos_cube(inv);

            // Add shape to buffer
            _shapes.push_back(_assets.inv_box(p));

            // Clear and reset the stream
            clear_stream();

            // Update item count
            _stream << static_cast<int>((*_inv)[i].count());

            // Add text for each box
            _text->add_text(_stream.str(), p.x() + _inv_ui_dx, p.y() + _inv_ui_dy);
        }

        // Add attr text
        const size_t attr_size = _stat->attr_str_size();
        for (size_t i = 0; i < attr_size; i++)
        {
            // Get attr text position
            const min::vec2<float> p = _assets.attr_position(i, _text_size);

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
            const min::vec2<float> p = _assets.stat_position(i, _text_size);

            // Add stat descriptor and value text
            _text->add_text(_stat->stat_str(i), p.x(), p.y());

            // Get stat value
            const uint16_t value = _stat->stat_value(i);

            // Create string from value
            clear_stream();
            _stream << value;

            // Set stat text at offset
            const size_t dx = stat_offset(value);
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
    inline min::vec2<float> pos_cube(const inv_id inv) const
    {
        // Get row and col
        const size_t row = inv.row3();
        const size_t col = inv.col3();

        // Calculate ui element position
        return _assets.cube_position(row, col);
    }
    inline min::vec2<float> pos_ext(const inv_id inv) const
    {
        // Get row and col
        const size_t row = inv.row8() + 1;
        const size_t col = inv.col8();

        // Calculate ui element position
        return _assets.toolbar_position(row, col);
    }
    inline min::vec2<float> pos_key(const inv_id inv) const
    {
        // Get row and col
        const size_t row = 0;
        const size_t col = inv.col8();

        // Calculate ui element position
        return _assets.toolbar_position(row, col);
    }
    inline min::vec2<float> pos_store(const inv_id inv) const
    {
        // Get row and col
        const size_t row = 0;
        const size_t col = inv.col8();

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

        // Load the inventory
        update_inventory();

        // Load stat background
        _assets.load_bg_stat();

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
        // Get the ui type
        const inv_type type = _click.type();

        // Determine inventory type to update bg color
        if (type == inv_type::cube)
        {
            _assets.load_bg_light_blue(_click.bg_cube_index(), pos_cube(_click));
        }
        else if (type == inv_type::extend)
        {
            _assets.load_bg_light_blue(_click.bg_ex_index(), pos_ext(_click));
        }
        else if (type == inv_type::key)
        {
            _assets.load_bg_light_blue(_click.bg_key_index(), pos_key(_click));
        }
        else
        {
            _assets.load_bg_light_blue(_click.bg_store_index(), pos_store(_click));
        }
    }
    inline void select_hover()
    {
        // Are we hovering?
        if (_hovering)
        {
            // Get the ui type
            const inv_type type = _hover.type();

            if (_clicking)
            {
                // Determine inventory type to update bg color
                if (type == inv_type::cube)
                {
                    _assets.load_bg_white(_hover.bg_cube_index(), pos_cube(_hover));
                }
                else if (type == inv_type::extend)
                {
                    _assets.load_bg_white(_hover.bg_ex_index(), pos_ext(_hover));
                }
                else if (type == inv_type::key)
                {
                    _assets.load_bg_white(_hover.bg_key_index(), pos_key(_hover));
                }
                else
                {
                    _assets.load_bg_white(_hover.bg_store_index(), pos_store(_hover));
                }
            }
            else
            {
                // Determine inventory type to update bg color
                if (type == inv_type::cube)
                {
                    _assets.load_bg_yellow(_hover.bg_cube_index(), pos_cube(_hover));
                }
                else if (type == inv_type::extend)
                {
                    _assets.load_bg_yellow(_hover.bg_ex_index(), pos_ext(_hover));
                }
                else if (type == inv_type::key)
                {
                    _assets.load_bg_yellow(_hover.bg_key_index(), pos_key(_hover));
                }
                else
                {
                    _assets.load_bg_yellow(_hover.bg_store_index(), pos_store(_hover));
                }
            }
        }
    }
    inline void set_click_down(const inv_id inv)
    {
        // If we are unclicking
        if (_clicking && _click == inv)
        {
            unselect_click();

            // Unset clicking
            _clicking = false;
        }
        else if (_clicking)
        {
            // Unselect old click
            unselect_click();

            // Swap inventory
            _inv->swap(_click.index(), inv.index());

            // Disable clicking
            _clicking = false;
        }
        else
        {
            // Set selected index
            _click = inv;

            // Select hover
            select_click();

            // Set clicking
            _clicking = true;
        }
    }
    inline void set_hover_down(const inv_id inv)
    {
        // Already hovering here?
        if (_hovering && _hover == inv)
        {
            return;
        }

        // Unselect hover
        unselect_hover();

        // Set selected index
        _hover = inv;

        // Set hovering
        _hovering = true;

        // If key is not selected or clicked change color
        const bool clicked = (_clicking && _click == inv);
        if (!clicked && _select != inv)
        {
            // Select hover
            select_hover();
        }
    }
    inline void set_inventory(const inv_id inv, const item &it, const min::vec2<float> &p)
    {
        // Draw key icon overlay based on type
        const item_type type = it.type();
        switch (type)
        {
        case item_type::empty:
            return _assets.load_empty_icon(inv, p);
        case item_type::skill:
            switch (it.id())
            {
            case id_value(skill_id::AUTO_BEAM):
                return _assets.load_auto_icon(inv, p);
            case id_value(skill_id::BEAM):
                return _assets.load_beam_icon(inv, p);
            case id_value(skill_id::CHARGE):
                return _assets.load_charge_icon(inv, p);
            case id_value(skill_id::GRAPPLE):
                return _assets.load_grapple_icon(inv, p);
            case id_value(skill_id::GRENADE):
                return _assets.load_grenade_icon(inv, p);
            case id_value(skill_id::JET):
                return _assets.load_jet_icon(inv, p);
            case id_value(skill_id::MISSILE):
                return _assets.load_missile_icon(inv, p);
            case id_value(skill_id::PORTAL):
                return _assets.load_portal_icon(inv, p);
            case id_value(skill_id::SCAN):
                return _assets.load_scan_icon(inv, p);
            case id_value(skill_id::SCATTER):
                return _assets.load_scatter_icon(inv, p);
            default:
                return;
            }
        case item_type::block:
            return _assets.load_block_icon(inv, it.to_block_id(), p);
        case item_type::item:
            return _assets.load_item_icon(inv, it.to_item_id(), p);
        }
    }
    inline void set_start_index(const GLint start_index) const
    {
        // Set the sampler active texture
        glUniform1i(_index_location, start_index);
    }
    inline static size_t stat_offset(const uint16_t value)
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
            // Get the ui type
            const inv_type type = _click.type();

            // Determine inventory type to update bg color
            if (type == inv_type::cube)
            {
                _assets.load_bg_black(_click.bg_cube_index(), pos_cube(_click));
            }
            else if (type == inv_type::extend)
            {
                _assets.load_bg_black(_click.bg_ex_index(), pos_ext(_click));
            }
            else if (type == inv_type::key)
            {
                _assets.load_bg_black(_click.bg_key_index(), pos_key(_click));
            }
            else
            {
                _assets.load_bg_black(_click.bg_store_index(), pos_store(_click));
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
                // Get the ui type
                const inv_type type = _hover.type();

                // Determine inventory type to update bg color
                if (type == inv_type::cube)
                {
                    _assets.load_bg_black(_hover.bg_cube_index(), pos_cube(_hover));
                }
                else if (type == inv_type::extend)
                {
                    _assets.load_bg_black(_hover.bg_ex_index(), pos_ext(_hover));
                }
                else if (type == inv_type::key)
                {
                    _assets.load_bg_black(_hover.bg_key_index(), pos_key(_hover));
                }
                else
                {
                    _assets.load_bg_black(_hover.bg_store_index(), pos_store(_hover));
                }
            }
        }
    }
    inline void update_store()
    {
        // Get start and end of store
        const size_t bs = _inv->begin_store();
        const size_t es = _inv->end_store();

        // Update all store icons
        for (size_t i = bs; i < es; i++)
        {
            // Offset this index for a ui bg key
            const inv_id inv = inv_id(i);

            // Calculate icon position
            const min::vec2<float> p = pos_store(inv);

            // Update the black bg icon
            _assets.load_bg_black(inv.bg_store_index(), p);

            // Update the icon
            set_inventory(inv.store_index(), (*_inv)[i], p);
        }
    }
    inline void update_cube()
    {
        // Get start and end of keys
        const size_t bc = _inv->begin_cube();
        const size_t ec = _inv->end_cube();

        // Update all key icons
        for (size_t i = bc; i < ec; i++)
        {
            // Offset this index for a ui bg key
            const inv_id inv = inv_id(i);

            // Calculate icon position
            const min::vec2<float> p = pos_cube(inv);

            // Update the black bg icon
            _assets.load_bg_black(inv.bg_cube_index(), p);

            // Update the icon
            set_inventory(inv.cube_index(), (*_inv)[i], p);
        }
    }
    inline void update_key()
    {
        // Get start and end of keys
        const size_t bk = _inv->begin_key();
        const size_t ek = _inv->end_key();

        // Update all key icons
        for (size_t i = bk; i < ek; i++)
        {
            // Offset this index for a ui bg key
            const inv_id inv = inv_id(i);

            // Calculate icon position
            const min::vec2<float> p = pos_key(inv);

            // Update the black bg icon
            _assets.load_bg_black(inv.bg_key_index(), p);

            // Update the icon
            set_inventory(inv.key_index(), (*_inv)[i], p);
        }
    }
    inline void update_extend()
    {
        // Get start and end of keys
        const size_t be = _inv->begin_extend();
        const size_t ee = _inv->end_extend();

        // Update all extended icons
        for (size_t i = be; i < ee; i++)
        {
            // Offset this index for a ui bg key
            const inv_id inv = inv_id(i);

            // Calculate icon position
            const min::vec2<float> p = pos_ext(inv);

            // Update the black bg icon
            _assets.load_bg_black(inv.bg_ex_index(), p);

            // Update the icon
            set_inventory(inv.ex_index(), (*_inv)[i], p);
        }
    }
    inline void update_inventory()
    {
        // Update store
        update_store();

        // Update keys
        update_key();

        // Update extend
        update_extend();

        // Update cube
        update_cube();

        // Select selected key
        select_active();

        // Select hovered key
        select_hover();
    }
    inline void update_inv_slot(const inv_id inv, const item &it)
    {
        // Get the ui type
        const inv_type type = inv.type();

        // Determine inventory type to update icon
        if (type == inv_type::cube)
        {
            set_inventory(inv.cube_index(), it, pos_cube(inv));
        }
        else if (type == inv_type::extend)
        {
            set_inventory(inv.ex_index(), it, pos_ext(inv));
        }
        else if (type == inv_type::key)
        {
            set_inventory(inv.key_index(), it, pos_key(inv));
        }
        else
        {
            set_inventory(inv.store_index(), it, pos_store(inv));
        }
    }

  public:
    ui_bg(const uniforms &uniforms, inventory &inv, stats &stat, min::text_buffer &text, const uint16_t width, const uint16_t height)
        : _vertex(memory_map::memory.get_file("data/shader/ui.vertex"), GL_VERTEX_SHADER),
          _fragment(memory_map::memory.get_file("data/shader/ui.fragment"), GL_FRAGMENT_SHADER),
          _prog(_vertex, _fragment), _index_location(load_program_index(uniforms)), _mesh_id(0),
          _clicking(false), _click(0), _hovering(false), _minimized(false), _hover(0), _select(inv.begin_key()),
          _assets(width, height), _inv(&inv), _stat(&stat), _text(&text), _grid(screen_box(width, height))
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
    inline bool action_hover(const uint8_t mult)
    {
        if (_hovering && !_minimized)
        {
            // Do action on hovering index
            return action(_hover.index(), mult);
        }

        // No action if not hovering
        return false;
    }
    inline bool action_select(const uint8_t mult)
    {
        // Do action on selected index
        return action(_select.index(), mult);
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
    inline bool click()
    {
        // Click on the currently hovered icon
        if (_hovering && !_minimized)
        {
            set_click_down(_hover);

            // Click happened
            return true;
        }

        return false;
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
            draw_title();
        }
        else if (_assets.get_draw_ex())
        {
            // Draw extended ui?
            draw_opaque_extend();
        }
        else
        {
            // Draw only base ui
            draw_opaque_base();
        }
    }
    inline void draw_tooltips() const
    {
        if (_hovering && _assets.get_draw_ex())
        {
            // Bind the text_buffer vao
            _vb.bind();

            // Bind the ui program
            _prog.use();

            // Draw tooltip ui
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

            // Draw transparent ui
            draw_transparent_ui();
        }
    }
    inline bool drop()
    {
        // If we are hovering
        if (_hovering)
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
        return _inv->get_info((*_inv)[_hover.index()]);
    }
    inline const std::string &get_hover_name() const
    {
        return _inv->get_name((*_inv)[_hover.index()]);
    }
    inline const std::vector<min::mat3<float>> &get_scale() const
    {
        return _assets.get_scale();
    }
    inline inv_id get_selected() const
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
    inline bool overlap(const min::vec2<float> &p)
    {
        // Is the inventory open?
        if (is_extended() && !_minimized)
        {
            // Bad point
            if (!_grid.inside(p))
            {
                return false;
            }

            // Search for overlapping cells
            const std::vector<uint16_t> &map = _grid.get_index_map();
            const std::vector<uint16_t> &hits = _grid.point_inside(p);

            // Set hover if overlapping
            bool hit = false;
            for (auto &h : hits)
            {
                if (_shapes[map[h]].point_inside(p))
                {
                    // Flag that we hit something
                    hit = true;

                    // Highlight the inventory cell
                    set_hover_down(inv_id(map[h]));
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
                return false;
            }
            else
            {
                // Update tooltips
                _assets.load_bg_hover(p);
            }

            // Overlapping a UI element
            return true;
        }

        // Not overlapping a UI element
        return false;
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
        _select = inv_id(index).to_key();

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

        // Convert index to inv_id
        const inv_id inv = inv_id(index).to_key();

        // Set background color
        _assets.load_bg_red(inv.bg_key_index(), p);
    }
    inline void set_key_up(const size_t index)
    {
        // Calculate ui element position
        const min::vec2<float> p = _assets.toolbar_position(0, index);

        // Convert index to inv_id
        const inv_id inv = inv_id(index).to_key();

        // Set background color
        if (_select.id() == inv.id())
        {
            _assets.load_bg_yellow(inv.bg_key_index(), p);
        }
        else
        {
            _assets.load_bg_black(inv.bg_key_index(), p);
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
            const std::vector<inv_id> &updates = _inv->get_updates();

            // Update all slots
            for (auto i : updates)
            {
                const item &it = (*_inv)[i.index()];

                // Update item count text
                clear_stream();
                _stream << static_cast<int>(it.count());

                // Might require a shift in the future
                _text->set_text(_stream.str(), i.index());

                // Update the slot
                update_inv_slot(i, it);
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
                const min::vec2<float> p = _assets.attr_position(i, _text_size);

                // Get text buffer offset
                const size_t index = attr_off + i;

                // Add attr string to stream
                clear_stream();
                _stream << _stat->attr_str(i) << ": " << _stat->attr_value(i);

                // Add attr descriptor and value text
                _text->set_text(_stream.str(), index, p.x(), p.y());
            }

            // Update stat text
            const size_t stat_off = (ec - bs) + attr_size;
            const size_t stat_size = _stat->stat_str_size();
            for (size_t i = 0; i < stat_size; i++)
            {
                // Get stat text position
                const min::vec2<float> p = _assets.stat_position(i, _text_size);

                // Get text buffer offset
                const size_t index = stat_off + (i * 2);

                // Add stat descriptor and value text
                _text->set_text(_stat->stat_str(i), index, p.x(), p.y());

                // Get stat value
                const uint16_t value = _stat->stat_value(i);

                // Create string from value
                clear_stream();
                _stream << value;

                // Set stat text at offset
                const size_t dx = stat_offset(value);
                _text->set_text(_stream.str(), index + 1, p.x() + dx, p.y());
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
