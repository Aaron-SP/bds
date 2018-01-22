/* Copyright [2013-2018] [Aaron Springstroh, Minimal Graphics Library]

This file is part of the Fractex.

Fractex is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Fractex is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Fractex.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _UI_BACKGROUND__
#define _UI_BACKGROUND__

#include <game/memory_map.h>
#include <game/ui_bg_assets.h>
#include <game/ui_vertex.h>
#include <game/uniforms.h>
#include <min/dds.h>
#include <min/program.h>
#include <min/shader.h>
#include <min/texture_buffer.h>
#include <min/vertex_buffer.h>

namespace game
{

class ui_bg
{
  private:
    // OpenGL stuff
    min::shader _vertex;
    min::shader _fragment;
    min::program _prog;

    // Instance buffer stuff
    min::vertex_buffer<float, uint32_t, game::ui_vertex, GL_FLOAT, GL_UNSIGNED_INT> _vb;
    size_t _mesh_id;

    // Texture stuff
    min::texture_buffer _tbuffer;
    GLuint _title_id;
    GLuint _ui_id;

    // Misc
    size_t _selected;

    // Background assets
    ui_bg_assets _assets;

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
    inline void position_ui()
    {
        if (_assets.get_draw_title())
        {
            _assets.load_title_overlay();
        }
        else
        {
            // Add health overlay
            _assets.load_health_overlay();
        }

        // Add console background
        _assets.load_console_bg();

        // Add 8 black rectangles along bottom
        for (size_t i = 0; i < 8; i++)
        {
            set_key_up(i);
        }

        // Add FPS cursor
        _assets.load_cursor_fps();

        // Add health meter
        _assets.load_energy_meter();

        // Add health meter
        _assets.load_health_meter();

        // Load pause text
        _assets.load_menu_pause();
    }

  public:
    ui_bg(const game::uniforms &uniforms, const uint16_t width, const uint16_t height)
        : _vertex(memory_map::memory.get_file("data/shader/ui.vertex"), GL_VERTEX_SHADER),
          _fragment(memory_map::memory.get_file("data/shader/ui.fragment"), GL_FRAGMENT_SHADER),
          _prog(_vertex, _fragment), _mesh_id(0), _selected(0), _assets(width, height)
    {
        // Create the instance rectangle
        load_base_rect();

        // Load texture
        load_texture();

        // Load the uniform buffer with program we will use
        uniforms.set_program_matrix(_prog);

        // Reposition all ui on the screen
        position_ui();
    }

    inline void draw(game::uniforms &uniforms) const
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
        else
        {
            draw_all();
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
    inline void reset_menu()
    {
        // Turn off drawing the menu
        _assets.set_draw_menu(false);

        // Set the cursor to fps
        _assets.load_cursor_fps();
    }
    inline void respawn()
    {
        // Turn off showing menu
        reset_menu();
    }
    inline void set_cursor_reload()
    {
        if (!_assets.get_draw_menu())
        {
            _assets.load_cursor_reload();
        }
    }
    inline void set_cursor_target()
    {
        if (!_assets.get_draw_menu())
        {
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
    inline void set_key_down(const size_t index)
    {
        // Set unselected color to black
        _assets.load_background_black(_selected);

        // Set selected index
        _selected = index;

        // Set selected color to white
        _assets.load_background_white(index);
    }
    inline void set_key_down_fail(const size_t index)
    {
        _assets.load_background_red(index);
    }
    inline void set_key_up(const size_t index)
    {
        // Set correct background if selected
        if (index == _selected)
        {
            _assets.load_background_yellow(index);
        }
        else
        {
            _assets.load_background_black(index);
        }

        // Draw key overlay
        switch (index)
        {
        case 0:
            return _assets.load_beam_icon(index);
        case 1:
            return _assets.load_missile_icon(index);
        case 2:
            return _assets.load_grapple_icon(index);
        case 3:
            return _assets.load_jet_icon(index);
        case 4:
            return _assets.load_scan_icon(index);
        }
    }
    inline void set_menu_dead()
    {
        // Draw the menu
        _assets.set_draw_menu(true);

        // Show the dead menu
        _assets.load_menu_dead();
    }
    inline void set_menu_pause()
    {
        // Draw the menu
        _assets.set_draw_menu(true);

        // Show the pause menu
        _assets.load_menu_pause();
    }
    inline void set_screen(const float width, const float height)
    {
        // Set asset screen size
        _assets.set_screen(width, height);

        // Reposition all ui on the screen
        position_ui();
    }

    inline void toggle_draw_console()
    {
        _assets.toggle_draw_console();

        // Reload console data
        _assets.load_console_bg();
    }
};
}

#endif
