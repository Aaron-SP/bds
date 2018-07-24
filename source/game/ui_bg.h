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
#ifndef _UI_BACKGROUND_BDS_
#define _UI_BACKGROUND_BDS_

#include <game/def.h>
#include <game/id.h>
#include <game/inventory.h>
#include <game/memory_map.h>
#include <game/stats.h>
#include <game/ui_bg_assets.h>
#include <game/ui_control_inv.h>
#include <game/ui_control_menu.h>
#include <game/ui_info.h>
#include <game/ui_state.h>
#include <game/uniforms.h>
#include <min/aabbox.h>
#include <min/dds.h>
#include <min/program.h>
#include <min/shader.h>
#include <min/text_buffer.h>
#include <min/texture_buffer.h>
#include <min/ui_vertex.h>
#include <min/vec2.h>
#include <min/vertex_buffer.h>
#include <sstream>
#include <utility>

namespace game
{

class ui_bg
{
  private:
    // OpenGL stuff
    min::shader _vertex;
    min::shader _fragment;
    min::program _prog;
    const GLint _index_location;

    // Instance buffer stuff
    min::vertex_buffer<float, uint32_t, min::ui_vertex, GL_FLOAT, GL_UNSIGNED_INT> _vb;
    const size_t _mesh_id;

    // Texture stuff
    min::texture_buffer _tbuffer;
    const GLuint _title_id;
    const GLuint _ui_id;

    // UI state
    bool _focus;
    ui_state _state;
    ui_bg_assets _assets;
    min::text_buffer *const _text;

    // Click detection
    ui_tree _tree;
    std::vector<min::aabbox<float, min::vec2>> _shapes;

    // UI controllers
    ui_control_inv _control_inv;
    ui_control_menu _control_menu;
    std::ostringstream _stream;

    inline void bind() const
    {
        // Bind the text_buffer vao
        _vb.bind();

        // Bind the ui program
        _prog.use();
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
    inline void draw_opaque_menu() const
    {
        // Get the start of the opaque ui
        const size_t start = _assets.menu_base_start();
        set_start_index(start);

        // Draw base ui elements
        const size_t size = _assets.menu_base_size();
        _vb.draw_many(GL_TRIANGLES, _mesh_id, size);
    }
    inline void draw_opaque_menu_ext() const
    {
        // Get the start of the opaque ui
        const size_t start = _assets.menu_ext_start();
        set_start_index(start);

        // Draw base ui elements
        const size_t size = _assets.menu_ext_size();
        _vb.draw_many(GL_TRIANGLES, _mesh_id, size);
    }
    inline void draw_title() const
    {
        // Get the start of the opaque ui
        const size_t start = _assets.title_start();
        set_start_index(start);

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
        if (!_state.is_menu_mode())
        {
            // Get the start of the transparent ui
            const size_t trans_start = _assets.transparent_start();
            set_start_index(trans_start);

            // Draw the transparent ui
            const size_t size = _assets.transparent_size();
            _vb.draw_many(GL_TRIANGLES, _mesh_id, size);
        }
        else if (!_state.is_title_mode())
        {
            // Get the start of the splash menu ui
            const size_t menu_splash_start = _assets.menu_splash_start();
            set_start_index(menu_splash_start);

            // Draw the splash menu ui
            const size_t size = _assets.menu_splash_size();
            _vb.draw_many(GL_TRIANGLES, _mesh_id, size);
        }
    }
    inline size_t load_base_rect()
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
        const size_t id = _vb.add_mesh(rect);

        // Unbind the last VAO to prevent scrambling buffers
        _vb.unbind();

        // Upload the text glyphs to the GPU
        _vb.upload();

        // Return the id
        return id;
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
    inline GLuint load_title_texture()
    {
        // Load texture
        const min::mem_file &title = memory_map::memory.get_file("data/texture/title.dds");
        const min::dds tex(title);

        // Load texture into texture buffer
        return _tbuffer.add_dds_texture(tex, true);
    }
    inline GLuint load_ui_texture()
    {
        // Load texture
        const min::mem_file &ui = memory_map::memory.get_file("data/texture/ui.dds");
        const min::dds tex(ui);

        // Load texture into texture buffer
        return _tbuffer.add_dds_texture(tex, true);
    }
    inline void position_ui(const min::vec2<float> &p)
    {
        // Load overlay
        if (_state.is_title_mode())
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
            _assets.load_splash_dead();
        }
        else if (_assets.get_draw_pause())
        {
            _assets.load_splash_pause();
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
        if (_state.is_inv_mode())
        {
            _control_inv.position_ui(_state);
        }
        else if (_state.is_menu_mode())
        {
            _control_menu.position_ui(_state);
        }

        // Load focus background
        _assets.load_bg_focus();

        // Load focus meter
        _assets.load_focus_meter();

        // Load hover background
        _assets.load_bg_hover(p);
    }
    inline static min::aabbox<float, min::vec2> screen_box(const uint_fast16_t width, const uint_fast16_t height)
    {
        // Create a box from the screen
        const min::vec2<float> min(0, 0);
        const min::vec2<float> max(width, height);

        // Return the screen in a 2D box
        return min::aabbox<float, min::vec2>(min, max);
    }
    inline void set_start_index(const GLint start_index) const
    {
        // Set the sampler active texture
        glUniform1i(_index_location, start_index);
    }

  public:
    ui_bg(const uniforms &uniforms, inventory &inv, stats &stat, min::text_buffer &text, ui_menu &menu, const uint_fast16_t width, const uint_fast16_t height)
        : _vertex(memory_map::memory.get_file("data/shader/ui.vertex"), GL_VERTEX_SHADER),
          _fragment(memory_map::memory.get_file("data/shader/ui.fragment"), GL_FRAGMENT_SHADER),
          _prog(_vertex, _fragment), _index_location(load_program_index(uniforms)),
          _mesh_id(load_base_rect()), _title_id(load_title_texture()), _ui_id(load_ui_texture()),
          _focus(false), _state(inv.begin_key()), _assets(width, height), _text(&text),
          _tree(screen_box(width, height)),
          _control_inv(_assets, inv, stat, text, _tree, _shapes), _control_menu(_assets, menu, _tree, _shapes)
    {
        // Format string stream
        _stream.precision(3);

        // Reposition all ui on the screen
        position_ui(min::vec2<float>());

        // Reserve shape memory
        _shapes.reserve(inv.size());
    }
    inline void reset()
    {
        // Reset UI state
        _focus = false;
        _state = ui_state(_state.get_select());

        // Reset UI assets
        _assets.reset();

        // Reset UI controllers
        _control_inv.reset();
        _control_menu.reset();

        // Clear the stream
        clear_stream();
    }
    inline std::pair<bool, ui_id> action_hover(const uint_fast8_t mult)
    {
        if (_state.is_inv_mode())
        {
            return _control_inv.action_hover(_state, mult);
        }
        else if (_state.is_menu_mode())
        {
            return _control_menu.action_hover(_state, mult);
        }

        return std::make_pair(false, 0);
    }
    inline std::pair<bool, ui_id> action_select(const uint_fast8_t mult)
    {
        if (_state.is_inv_mode())
        {
            return _control_inv.action_select(_state, mult);
        }
        else if (_state.is_menu_mode())
        {
            return _control_menu.action_select(_state, mult);
        }

        return std::make_pair(false, 0);
    }
    inline size_t bg_text_size() const
    {
        return _control_inv.bg_text_size(_state);
    }
    inline bool click_down()
    {
        if (_state.get_mode() == ui_mode::INV_EXT)
        {
            return _control_inv.click_down(_state);
        }
        else if (_state.is_menu_mode())
        {
            return _control_menu.click_down(_state);
        }

        return false;
    }
    inline void click_up()
    {
        if (_state.get_mode() == ui_mode::INV_EXT)
        {
            _control_inv.click_up(_state);
        }
        else if (_state.is_menu_mode())
        {
            _control_menu.click_up(_state);
        }
    }
    inline void draw_opaque() const
    {
        // If we are drawing the title screen
        if (_state.is_title_mode())
        {
            // Bind the VAO and program
            bind();

            // Bind the ui texture for drawing
            _tbuffer.bind(_title_id, 0);

            // Draw title
            draw_title();
        }

        if (_state.get_mode() == ui_mode::INV_EXT)
        {
            // Bind the VAO and program
            bind();

            // Bind the ui texture for drawing
            _tbuffer.bind(_ui_id, 0);

            // Draw extended ui?
            draw_opaque_extend();

            // Draw focus UI
            if (_focus)
            {
                draw_focus_ui();
            }
        }
        else if (_state.get_mode() == ui_mode::INV)
        {
            // Bind the VAO and program
            bind();

            // Bind the ui texture for drawing
            _tbuffer.bind(_ui_id, 0);

            // Draw only base ui
            draw_opaque_base();

            // Draw focus UI
            if (_focus)
            {
                draw_focus_ui();
            }
        }
        else if (_state.is_menu_mode())
        {
            // Bind the VAO and program
            bind();

            // Bind the ui texture for drawing
            _tbuffer.bind(_ui_id, 0);

            // Draw menu ui
            if (_state.get_mode() == ui_mode::MENU_EXT)
            {
                draw_opaque_menu_ext();
            }
            else
            {
                draw_opaque_menu();
            }

            // Draw focus UI
            if (_focus)
            {
                draw_focus_ui();
            }
        }
    }
    inline void draw_tooltips() const
    {
        // Bind the VAO and program
        bind();

        // Bind the ui texture for drawing
        _tbuffer.bind(_ui_id, 0);

        // Draw tooltips
        draw_tooltip_ui();
    }
    inline void draw_transparent() const
    {
        if (!_state.is_title_mode())
        {
            // Bind the VAO and program
            bind();

            // Bind the ui texture for drawing
            _tbuffer.bind(_ui_id, 0);

            // Draw transparent ui
            draw_transparent_ui();
        }
    }
    inline bool drop()
    {
        // If UI is extended
        if (_state.get_mode() == ui_mode::INV_EXT)
        {
            return _control_inv.drop(_state);
        }

        // Didn't drop anything
        return false;
    }
    inline const std::vector<min::mat3<float>> &get_scale() const
    {
        return _assets.get_scale();
    }
    inline ui_id get_selected() const
    {
        return _state.get_select();
    }
    inline const std::vector<min::mat3<float>> &get_uv() const
    {
        return _assets.get_uv();
    }
    inline bool is_draw_tooltips() const
    {
        const bool extended = _state.get_mode() == ui_mode::INV_EXT;
        const bool hovering = _state.is_hovering_not_button();

        return extended && hovering;
    }
    inline bool is_focused() const
    {
        return _focus;
    }
    inline std::pair<bool, ui_id> overlap(const min::vec2<float> &p)
    {
        if (_state.is_inv_mode())
        {
            return _control_inv.overlap(_state, p);
        }
        else if (_state.is_menu_mode())
        {
            return _control_menu.overlap(_state, p);
        }

        return std::make_pair(false, 0);
    }
    inline void reset_cursor()
    {
        // Turn off drawing the dead or pause
        _assets.set_draw_aim();

        // Set the cursor to aim
        _assets.load_cursor_aim();
    }
    inline void respawn()
    {
        reset_cursor();
    }
    inline void set_cursor_aim()
    {
        if (!_assets.get_draw_splash())
        {
            _assets.set_draw_aim();
            _assets.load_cursor_aim();
        }
    }
    inline void set_cursor_reload()
    {
        if (!_assets.get_draw_splash())
        {
            _assets.set_draw_reload();
            _assets.load_cursor_reload();
        }
    }
    inline void set_cursor_target()
    {
        if (!_assets.get_draw_splash())
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
        if (_state.is_inv_mode())
        {
            _control_inv.set_key_down(_state, index);
        }
        else if (_state.is_menu_mode())
        {
            _control_menu.set_key_down(_state, index);
        }
    }
    inline void set_key_down_fail(const size_t index)
    {
        if (_state.is_inv_mode())
        {
            _control_inv.set_key_down_fail(_state, index);
        }
        else if (_state.is_menu_mode())
        {
            _control_menu.set_key_down_fail(_state, index);
        }
    }
    inline void set_key_up(const size_t index)
    {
        if (_state.is_inv_mode())
        {
            _control_inv.set_key_up(_state, index);
        }
        else if (_state.is_menu_mode())
        {
            _control_menu.set_key_up(_state, index);
        }
    }
    inline void set_splash_dead()
    {
        // Draw the menu
        _assets.set_draw_dead();

        // Show the dead menu
        _assets.load_splash_dead();
    }
    inline void set_splash_pause()
    {
        // Draw the menu
        _assets.set_draw_splash();

        // Show the pause menu
        _assets.load_splash_pause();
    }
    inline void set_minimized(const bool flag)
    {
        if (_state.is_inv_mode())
        {
            _control_inv.set_minimized(flag);
        }
        else if (_state.is_menu_mode())
        {
            _control_menu.set_minimized(flag);
        }
    }
    inline void set_screen(const min::vec2<float> &p, const uint_fast16_t width, const uint_fast16_t height)
    {
        // Update the controller screen dimensions
        // Set asset screen size
        _assets.set_screen(width, height);
        _text->set_screen(width, height);

        // Reposition all ui on the screen
        position_ui(p);

        // Resize the screen tree box
        _tree.resize(screen_box(width, height));

        // Load tree inventory boxes
        if (_state.is_inv_mode())
        {
            _control_inv.load_tree(_state, _stream, width, height);
        }
        else if (_state.is_menu_mode())
        {
            _control_menu.load_tree(_state, _stream, width, height);
        }
    }
    inline void switch_mode(const ui_mode mode)
    {
        // Reset the state
        _state.set_mode(mode);

        const uint_fast16_t width = _assets.get_width();
        const uint_fast16_t height = _assets.get_height();

        // Load title screen
        if (_state.is_title_mode())
        {
            _assets.load_title_overlay();
        }

        // Load health overlay
        if (_state.is_inv_mode())
        {
            _assets.load_health_overlay();
            _control_inv.position_ui(_state);
            _control_inv.load_tree(_state, _stream, width, height);
        }
        else if (_state.is_menu_mode())
        {
            _control_menu.position_ui(_state);
            _control_menu.load_tree(_state, _stream, width, height);
        }
    }
    inline void toggle_draw_console()
    {
        _assets.toggle_draw_console();

        // Reload console data
        _assets.load_console_bg();
    }
    inline void transition_state()
    {
        // Set UI state
        if (_state.is_inv_mode())
        {
            _control_inv.transition_state(_state);
        }
        else if (_state.is_menu_mode())
        {
            _control_menu.transition_state(_state);
        }
    }
    inline ui_info get_ui_info() const
    {
        return _control_inv.get_ui_info(_state);
    }
    inline const ui_state &get_ui_state() const
    {
        return _state;
    }
    inline ui_state &get_ui_state()
    {
        return _state;
    }
    inline void update()
    {
        if (_state.is_inv_mode())
        {
            _control_inv.update(_state, _stream);
        }
        else if (_state.is_menu_mode())
        {
            _control_menu.update(_state, _stream);
        }
    }
};
}

#endif
