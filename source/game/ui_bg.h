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

#include <game/memory_map.h>
#include <game/ui_bg_assets.h>
#include <game/ui_vertex.h>
#include <game/uniforms.h>
#include <min/aabbox.h>
#include <min/dds.h>
#include <min/grid.h>
#include <min/program.h>
#include <min/shader.h>
#include <min/texture_buffer.h>
#include <min/vec2.h>
#include <min/vertex_buffer.h>

namespace game
{

class ui_bg
{
  private:
    static constexpr uint8_t _cols = 8;

    // OpenGL stuff
    min::shader _vertex;
    min::shader _fragment;
    min::program _prog;

    // Instance buffer stuff
    min::vertex_buffer<float, uint32_t, ui_vertex, GL_FLOAT, GL_UNSIGNED_INT> _vb;
    size_t _mesh_id;

    // Texture stuff
    min::texture_buffer _tbuffer;
    GLuint _title_id;
    GLuint _ui_id;

    // Misc
    inv_id _click;
    inv_id _hover;
    inv_id _select;
    bool _clicking;
    bool _hovering;
    bool _minimized;

    // Background assets
    ui_bg_assets _assets;
    inventory *const _inv;

    // Click detection
    std::vector<min::aabbox<float, min::vec2>> _shapes;
    min::grid<float, uint16_t, uint16_t, min::vec2, min::aabbox, min::aabbox> _grid;

    inline void draw_all() const
    {
        // Bind the ui texture for drawing
        _tbuffer.bind(_ui_id, 0);

        // Draw the all ui elements
        const size_t size = _assets.size();
        _vb.draw_many(GL_TRIANGLES, _mesh_id, size);
    }
    inline void draw_title() const
    {
        // Bind the ui texture for drawing
        _tbuffer.bind(_title_id, 0);

        // Draw the first thing in the buffer, title screen
        _vb.draw_many(GL_TRIANGLES, _mesh_id, 1);
    }
    inline void draw_ui() const
    {
        // Bind the ui texture for drawing
        _tbuffer.bind(_ui_id, 0);

        // Draw the base ui elements
        const size_t size = _assets.ui_size();
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

        // First row
        for (size_t i = 0; i < _cols; i++)
        {
            // Offset this index for a ui bg key
            const inv_id inv = inv_id(i);

            // Get row and col
            const size_t row = inv.row();
            const size_t col = inv.col();

            // Calculate ui element position
            const min::vec2<float> p = _assets.toolbar_position(row, col);

            // Add shape to buffer
            _shapes.push_back(_assets.inv_box(p));
        }

        // Extended rows
        const size_t size = _inv->size();
        for (size_t i = _cols; i < size; i++)
        {
            // Offset this index for a ui bg key
            const inv_id inv = inv_id(i);

            // Get row and col
            const size_t row = inv.ext_row();
            const size_t col = inv.col();

            // Calculate ui element position
            const min::vec2<float> p = _assets.toolbar_position(row, col);

            // Add shape to buffer
            _shapes.push_back(_assets.inv_box(p));
        }

        // Calculate the grid scale
        const uint16_t scale = width / 24;

        // Insert the shapes into the grid
        _grid.insert(_shapes, scale);
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
    inline min::vec2<float> position(const inv_id inv)
    {
        // Get row and col
        const size_t row = inv.row();
        const size_t col = inv.col();

        // Calculate ui element position
        return _assets.toolbar_position(row, col);
    }
    inline min::vec2<float> position_ext(const inv_id inv)
    {
        // Get row and col
        const size_t row = inv.ext_row();
        const size_t col = inv.col();

        // Calculate ui element position
        return _assets.toolbar_position(row, col);
    }
    inline void position_ui()
    {
        if (_assets.get_draw_title())
        {
            _assets.load_title_overlay();
        }
        else
        {
            // Load health overlay
            _assets.load_health_overlay();
        }

        // Load console background
        _assets.load_console_bg();

        if (_assets.get_draw_dead())
        {
            // Load dead message
            _assets.load_menu_dead();
        }
        else if (_assets.get_draw_pause())
        {
            // Load pause message
            _assets.load_menu_pause();
        }
        else if (_assets.get_draw_reload())
        {
            // Load reload cursor
            _assets.load_cursor_reload();
        }
        else
        {
            // Load FPS cursor
            _assets.load_cursor_fps();
        }

        // Load health meter
        _assets.load_energy_meter();

        // Load health meter
        _assets.load_health_meter();

        // Load the inventory
        update_inventory();
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
        const min::vec2<float> active = _assets.toolbar_position(0, _select.id());

        // Set activated index color to white
        _assets.load_bg_white(_select.bg_key_index(), active);
    }
    void select_active()
    {
        // Bg key placement
        const min::vec2<float> active = _assets.toolbar_position(0, _select.id());

        // Set activated index color to white
        _assets.load_bg_yellow(_select.bg_key_index(), active);
    }
    void select_click()
    {
        // Determine if this is extended inventory
        if (_click.id() >= 8)
        {
            // Get ui position
            const min::vec2<float> p = position_ext(_click);

            // Update the bg color
            _assets.load_bg_light_blue(_click.bg_inv_index(), p);
        }
        else
        {
            // Get ui position
            const min::vec2<float> p = position(_click);

            // Update the bg color
            _assets.load_bg_light_blue(_click.bg_key_index(), p);
        }
    }
    void select_hover()
    {
        // Are we hovering?
        if (_hovering)
        {
            // Determine if this is extended inventory
            if (_hover.id() >= 8)
            {
                // Get ui position
                const min::vec2<float> p = position_ext(_hover);

                // Update the bg color
                _assets.load_bg_yellow(_hover.bg_inv_index(), p);
            }
            else
            {
                // Get ui position
                const min::vec2<float> p = position(_hover);

                // Update the bg color
                _assets.load_bg_yellow(_hover.bg_key_index(), p);
            }
        }
    }
    inline void set_inventory(const inv_id inv, const uint8_t id, const min::vec2<float> &p)
    {
        // Draw key icon overlay
        switch (id)
        {
        case 0:
            return _assets.load_cube_icon(inv, 23, p);
        case 1:
            return _assets.load_beam_icon(inv, p);
        case 2:
            return _assets.load_missile_icon(inv, p);
        case 3:
            return _assets.load_grapple_icon(inv, p);
        case 4:
            return _assets.load_jet_icon(inv, p);
        case 5:
            return _assets.load_scan_icon(inv, p);
        case 6:
            break;
        case 7:
            break;
        case 8:
            break;
        default:
            return _assets.load_cube_icon(inv, _inv->id_to_atlas(id), p);
        }
    }
    inline void unselect()
    {
        // Bg key placement
        const min::vec2<float> prev = _assets.toolbar_position(0, _select.id());

        // Set previous unselected color to black
        _assets.load_bg_black(_select.bg_key_index(), prev);
    }
    inline void unselect_click()
    {
        // Determine if this is extended inventory
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
            if (_click.id() >= 8)
            {
                // Get ui position
                const min::vec2<float> p = position_ext(_click);

                // Update the bg color
                _assets.load_bg_black(_click.bg_inv_index(), p);
            }
            else
            {
                // Get ui position
                const min::vec2<float> p = position(_click);

                // Update the bg color
                _assets.load_bg_black(_click.bg_key_index(), p);
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
                // Determine if this is extended inventory
                if (_hover.id() >= 8)
                {
                    // Get ui position
                    const min::vec2<float> p = position_ext(_hover);

                    // Update the bg color
                    _assets.load_bg_black(_hover.bg_inv_index(), p);
                }
                else
                {
                    // Get ui position
                    const min::vec2<float> p = position(_hover);

                    // Update the bg color
                    _assets.load_bg_black(_hover.bg_key_index(), p);
                }
            }
        }
    }
    inline void update_inventory()
    {
        // Update all bg icons
        for (size_t i = 0; i < _cols; i++)
        {
            // Offset this index for a ui bg key
            const inv_id inv = inv_id(i);

            // Get ui position
            const min::vec2<float> p = position(inv);

            // Update the black bg icon
            _assets.load_bg_black(inv.bg_key_index(), p);
        }

        // Update all key icons
        for (size_t i = 0; i < _cols; i++)
        {
            // Offset this index for a ui bg key
            const inv_id inv = inv_id(i);

            // Get ui position
            const min::vec2<float> p = position(inv);

            // Get the inventory id
            const uint8_t id = (*_inv)[i].id();

            // Update the icon
            set_inventory(inv.key_index(), id, p);
        }

        // Update all bg inventory icons
        const size_t size = _inv->size();
        for (size_t i = _cols; i < size; i++)
        {
            // Offset this index for a ui bg key
            const inv_id inv = inv_id(i);

            // Get ui position
            const min::vec2<float> p = position_ext(inv);

            // Update the black bg icon
            _assets.load_bg_black(inv.bg_inv_index(), p);
        }

        // Update all inventory icons
        for (size_t i = _cols; i < size; i++)
        {
            // Offset this index for a ui bg key
            const inv_id inv = inv_id(i);

            // Get ui position
            const min::vec2<float> p = position_ext(inv);

            // Get the inventory id
            const uint8_t id = (*_inv)[i].id();

            // Update the icon
            set_inventory(inv.inv_index(), id, p);
        }

        // Select selected key
        select_active();

        // Select hovered key
        select_hover();
    }
    inline void update_inv_slot(const inv_id inv, const uint8_t id)
    {
        // Determine if this is extended inventory
        if (inv.id() >= 8)
        {
            // Get ui position
            const min::vec2<float> p = position_ext(inv);

            // Update the icon
            set_inventory(inv.inv_index(), id, p);
        }
        else
        {
            // Get ui position
            const min::vec2<float> p = position(inv);

            // Update the icon
            set_inventory(inv.key_index(), id, p);
        }
    }

  public:
    ui_bg(const uniforms &uniforms, inventory *const inv, const uint16_t width, const uint16_t height)
        : _vertex(memory_map::memory.get_file("data/shader/ui.vertex"), GL_VERTEX_SHADER),
          _fragment(memory_map::memory.get_file("data/shader/ui.fragment"), GL_FRAGMENT_SHADER),
          _prog(_vertex, _fragment), _mesh_id(0),
          _click(0), _hover(0), _select(0),
          _clicking(false), _hovering(false), _minimized(false),
          _assets(width, height), _inv(inv), _grid(screen_box(width, height))
    {
        // Create the instance rectangle
        load_base_rect();

        // Load texture
        load_texture();

        // Reserve shape memory
        _shapes.reserve(_inv->size());

        // Load the grid inventory boxes
        load_grid(width, height);

        // Load the uniform buffer with program we will use
        uniforms.set_program_matrix(_prog);

        // Reposition all ui on the screen
        position_ui();
    }

    inline void draw(const uniforms &uniforms) const
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
            draw_all();
        }
        else
        {
            // Draw only basic ui
            draw_ui();
        }
    }
    inline const std::vector<min::mat3<float>> &get_scale() const
    {
        return _assets.get_scale();
    }
    inline const std::vector<min::mat3<float>> &get_uv() const
    {
        return _assets.get_uv();
    }
    inline void click()
    {
        // Click on the currently hovered icon
        if (_hovering && !_minimized)
        {
            set_click_down(_hover);
        }
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
            const std::vector<size_t> &map = _grid.get_index_map();
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
            }

            return true;
        }

        return false;
    }
    inline void reset_menu()
    {
        // Turn off drawing the dead or pause
        _assets.set_draw_fps();

        // Set the cursor to fps
        _assets.load_cursor_fps();
    }
    inline void respawn()
    {
        // Turn off showing menu
        reset_menu();
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
            _assets.set_draw_fps();
            _assets.load_cursor_fps();
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
    inline void set_health(const float health)
    {
        _assets.set_health(health);
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
        if (_click != inv && _select != inv)
        {
            // Select hover
            select_hover();
        }
    }
    inline void set_key_down(const size_t index)
    {
        // Unselect old key
        unselect();

        // Set selected index
        _select = inv_id(index);

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
        const inv_id inv = inv_id(index);

        // Set background color
        _assets.load_bg_red(inv.bg_key_index(), p);
    }
    inline void set_key_up(const size_t index)
    {
        // Calculate ui element position
        const min::vec2<float> p = _assets.toolbar_position(0, index);

        // Convert index to inv_id
        const inv_id inv = inv_id(index);

        // Set background color
        if (_select.id() == index)
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
    inline void set_screen(const uint16_t width, const uint16_t height)
    {
        // Set asset screen size
        _assets.set_screen(width, height);

        // Reposition all ui on the screen
        position_ui();

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
        _assets.toggle_draw_ex();
    }
    void update()
    {
        // Update the inventory matrices if dirty
        if (_inv->dirty())
        {
            // Get all updated slots
            const std::vector<inv_id> &updates = _inv->get_updates();

            // Update all slots
            for (auto i : updates)
            {
                const item &it = (*_inv)[i.index()];
                update_inv_slot(i, it.id());
            }

            // Flag that we clean updated the inventory state
            _inv->clean();
        }
    }
};
}

#endif
