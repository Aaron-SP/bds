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
#ifndef __UI_TEXT__
#define __UI_TEXT__

#include <game/memory_map.h>
#include <iomanip>
#include <min/dds.h>
#include <min/program.h>
#include <min/shader.h>
#include <min/text_buffer.h>
#include <sstream>
#include <vector>

namespace game
{

class ui_text
{
  private:
    static constexpr size_t _console_offset = 8;
    static constexpr size_t _ui_offset = 10;
    static constexpr size_t _end = 12;
    static constexpr float _y_console = 90.0;
    static constexpr float _x_console_wrap = 400.0;
    static constexpr float _y_console_wrap = 40.0;
    static constexpr float _x_health_offset = 248.0;
    static constexpr float _x_energy_offset = 200.0;
    static constexpr float _y_ui = 150.0;

    // Text OpenGL stuff
    min::shader _text_vertex;
    min::shader _text_fragment;
    min::program _text_prog;

    // Buffer for holding text
    min::text_buffer _text_buffer;
    std::vector<size_t> _indices;
    std::ostringstream _stream;
    size_t _font_size;
    bool _draw_console;
    bool _draw_debug;
    bool _draw_ui;

    inline void add_text(const std::string &s, const float x, const float y)
    {
        const size_t index = _text_buffer.add_text(s, x, y);

        // Add text index to index buffer
        _indices.push_back(index);
    }
    inline void bind() const
    {
        // Bind the text_buffer vao, and textures on channel '1'
        _text_buffer.bind(1);

        // Bind the text program
        _text_prog.use();
    }
    inline void clear_stream()
    {
        _stream.clear();
        _stream.str(std::string());
    }
    inline void reposition_text(const uint16_t width, const uint16_t height)
    {
        // Rescale all text items
        uint16_t y = height - 20;
        for (size_t i = 0; i < _console_offset; i++)
        {
            // Update the text location
            _text_buffer.set_text_location(i, 10, y);
            y -= _font_size;
        }

        // Position the console elements
        const uint16_t w2 = (width / 2);
        _text_buffer.set_text_center(_console_offset, w2, _y_console);

        // Position the ui elements
        _text_buffer.set_text_location(_console_offset + 1, w2 - _x_health_offset, _y_ui);
        _text_buffer.set_text_location(_console_offset + 2, w2 + _x_energy_offset, _y_ui);
    }
    inline void update_text(const size_t index, const std::string &s)
    {
        _text_buffer.set_text(s, index);
    }

  public:
    ui_text(const size_t font_size, const uint16_t width, const uint16_t height)
        : _text_vertex(memory_map::memory.get_file("data/shader/text.vertex"), GL_VERTEX_SHADER),
          _text_fragment(memory_map::memory.get_file("data/shader/text.fragment"), GL_FRAGMENT_SHADER),
          _text_prog(_text_vertex, _text_fragment),
          _text_buffer("data/fonts/open_sans.ttf", font_size),
          _font_size(font_size), _draw_console(false), _draw_debug(false), _draw_ui(false)
    {
        // Set the texture channel for this program, we need to do this here because we render text on channel '1'
        // _text_prog will be in use by the end of this call
        _text_buffer.set_texture_uniform(_text_prog, "in_texture", 1);

        // Update the text buffer screen dimensions
        _text_buffer.set_screen(width, height);

        // Add title text
        add_text("Fractex: Official Demo", 0, 0);

        // Add 7 text entries
        for (size_t i = 1; i < _console_offset; i++)
        {
            add_text("", 0, 0);
        }

        // Add 1 text entries
        for (size_t i = _console_offset; i < _ui_offset; i++)
        {
            add_text("", 0, 0);
            _text_buffer.set_line_wrap(i, _x_console_wrap, _y_console_wrap);
        }

        // Add 2 text entries
        for (size_t i = _ui_offset; i < _end; i++)
        {
            add_text("", 0, 0);
        }

        // Reposition all of the text
        reposition_text(width, height);
    }
    void draw() const
    {
        if (_draw_debug && _draw_console && _draw_ui)
        {
            // Bind texture and program
            bind();

            // Draw all the text
            _text_buffer.draw_all();
        }
        else if (_draw_debug && _draw_ui)
        {
            // Bind texture and program
            bind();

            // Draw only debug text
            _text_buffer.draw(0, _console_offset - 1);

            // Draw only ui text
            _text_buffer.draw(_ui_offset, _end - 1);
        }
        else if (_draw_console && _draw_ui)
        {
            // Bind texture and program
            bind();

            // Draw from console start to end of buffer
            _text_buffer.draw(_console_offset, _end - 1);
        }
        else if (_draw_ui)
        {
            // Bind texture and program
            bind();

            // Draw only ui text
            _text_buffer.draw(_ui_offset, _end - 1);
        }
        else if (_draw_console)
        {
            // Bind texture and program
            bind();

            // Draw only debug text
            _text_buffer.draw(_console_offset, _ui_offset - 1);
        }
    }
    inline void set_draw_debug(const bool flag)
    {
        _draw_debug = flag;
    }
    inline void set_draw_console(const bool flag)
    {
        _draw_console = flag;
    }
    inline void set_draw_ui(const bool flag)
    {
        _draw_ui = flag;
    }
    inline void set_screen(const uint16_t width, const uint16_t height)
    {
        // Update the text buffer screen dimensions
        _text_buffer.set_screen(width, height);

        // Rescale all text on the screen
        reposition_text(width, height);

        // Upload new text
        upload();
    }
    inline void toggle_draw_console()
    {
        _draw_console = !_draw_console;
    }
    inline void toggle_draw_debug()
    {
        _draw_debug = !_draw_debug;
    }
    void update_debug_text(
        const min::vec3<float> &p, const min::vec3<float> &f, const std::string &mode,
        const float health, const float energy, const double fps, const double idle)
    {
        // If drawing text mode is on, update text
        if (_draw_debug)
        {
            // Update player position debug text
            _stream << std::fixed << std::setprecision(4) << "POS- X: " << p.x() << ", Y: " << p.y() << ", Z: " << p.z();
            update_text(1, _stream.str());

            // Clear and reset the _stream
            clear_stream();

            // Update player direction debug text
            _stream << "DIR- X: " << f.x() << ", Y: " << f.y() << ", Z: " << f.z();
            update_text(2, _stream.str());

            // Clear and reset the _stream
            clear_stream();

            // Update the game mode text
            update_text(3, mode);

            // Update the energy text
            _stream << "HEALTH: " << health;
            update_text(4, _stream.str());

            // Clear and reset the _stream
            clear_stream();

            // Update the energy text
            _stream << "ENERGY: " << energy;
            update_text(5, _stream.str());

            // Clear and reset the _stream
            clear_stream();

            // Update FPS and IDLE
            _stream << "FPS: " << std::round(fps);
            update_text(6, _stream.str());

            // Clear and reset the _stream
            clear_stream();

            // Update FPS and IDLE
            _stream << "IDLE: " << idle;
            update_text(7, _stream.str());

            // Clear and reset the _stream
            clear_stream();
        }
    }
    void update_ui(const float health, const float energy)
    {
        // Update the energy text
        _stream << static_cast<int>(std::round(health));
        update_text(_console_offset + 1, _stream.str());

        // Clear and reset the _stream
        clear_stream();

        // Update the energy text
        _stream << static_cast<int>(std::round(energy));
        update_text(_console_offset + 2, _stream.str());

        // Clear and reset the _stream
        clear_stream();
    }
    void update_console(const std::string &str)
    {
        // Update console text
        update_text(_console_offset, str);

        // Get the screen dimensions
        const std::pair<float, float> size = _text_buffer.get_screen_size();

        // Position the console elements
        const uint16_t w2 = (size.first / 2);
        _text_buffer.set_text_center(_console_offset, w2, _y_console);
    }
    inline void upload() const
    {
        // Unbind the last VAO to prevent scrambling buffers
        _text_buffer.unbind();

        // Upload the text glyphs to the GPU
        _text_buffer.upload();
    }
};
}

#endif
