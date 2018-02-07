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
#ifndef __UI_TEXT__
#define __UI_TEXT__

#include <game/memory_map.h>
#include <iomanip>
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
    static constexpr size_t _debug_offset = 3;
    static constexpr size_t _console_offset = _debug_offset + 8;
    static constexpr size_t _ui_offset = _console_offset + 1;
    static constexpr size_t _end = _ui_offset + 2;
    static constexpr float _y_console = 90.0;
    static constexpr float _x_console_wrap = 400.0;
    static constexpr float _y_console_wrap = 40.0;
    static constexpr float _x_health_offset = 248.0;
    static constexpr float _x_energy_offset = 200.0;
    static constexpr float _y_ui = 150.0;

    // Text OpenGL stuff
    min::shader _vertex;
    min::shader _fragment;
    min::program _prog;

    // Buffer for holding text
    min::text_buffer _text;
    min::text_buffer _text_bg;
    std::vector<size_t> _indices;
    std::ostringstream _stream;
    size_t _font_size;
    bool _draw_console;
    bool _draw_debug;
    bool _draw_ui;

    inline void add_text(const std::string &s, const float x, const float y)
    {
        const size_t index = _text.add_text(s, x, y);

        // Add text index to index buffer
        _indices.push_back(index);
    }
    inline void bind() const
    {
        // Bind the text_buffer vao, and textures on channel '1'
        _text.bind(0);

        // Bind the text program
        _prog.use();
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
            _text.set_text_location(i, 10, y);
            y -= _font_size;
        }

        // Position the console elements
        const uint16_t w2 = (width / 2);
        _text.set_text_center(_console_offset, w2, _y_console);

        // Position the ui elements
        _text.set_text_location(_console_offset + 1, w2 - _x_health_offset, _y_ui);
        _text.set_text_location(_console_offset + 2, w2 + _x_energy_offset, _y_ui);
    }
    inline void update_text(const size_t index, const std::string &s)
    {
        _text.set_text(s, index);
    }

  public:
    ui_text(const size_t font_size, const uint16_t width, const uint16_t height)
        : _vertex(memory_map::memory.get_file("data/shader/text.vertex"), GL_VERTEX_SHADER),
          _fragment(memory_map::memory.get_file("data/shader/text.fragment"), GL_FRAGMENT_SHADER),
          _prog(_vertex, _fragment),
          _text("data/fonts/open_sans.ttf", font_size),
          _text_bg("data/fonts/open_sans.ttf", 14),
          _font_size(font_size), _draw_console(false), _draw_debug(false), _draw_ui(false)
    {
        // Update the text buffer screen dimensions
        _text.set_screen(width, height);
        _text_bg.set_screen(width, height);

        // Add title text
        add_text("Title", 0, 0);
        add_text("Vendor", 0, 0);
        add_text("Renderer", 0, 0);

        // Add 8 text entries
        for (size_t i = _debug_offset; i < _console_offset; i++)
        {
            add_text("", 0, 0);
        }

        // Add 1 text entries
        for (size_t i = _console_offset; i < _ui_offset; i++)
        {
            add_text("", 0, 0);
            _text.set_line_wrap(i, _x_console_wrap, _y_console_wrap);
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
            _text.draw_all();
        }
        else if (_draw_debug && _draw_ui)
        {
            // Bind texture and program
            bind();

            // Draw only debug text
            _text.draw(0, _console_offset - 1);

            // Draw only ui text
            _text.draw(_ui_offset, _end - 1);
        }
        else if (_draw_console && _draw_ui)
        {
            // Bind texture and program
            bind();

            // Draw from console start to end of buffer
            _text.draw(_console_offset, _end - 1);
        }
        else if (_draw_ui)
        {
            // Bind texture and program
            bind();

            // Draw only ui text
            _text.draw(_ui_offset, _end - 1);
        }
        else if (_draw_console)
        {
            // Bind texture and program
            bind();

            // Draw only debug text
            _text.draw(_console_offset, _ui_offset - 1);
        }
        else
        {
            // Bind program for text_bg
            bind();
        }

        // Draw the bg text
        _text_bg.bind(0);
        _text_bg.draw_all();
    }
    inline min::text_buffer &get_bg_text()
    {
        return _text_bg;
    }
    inline bool is_draw_debug() const
    {
        return _draw_debug;
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
        _text.set_screen(width, height);

        // Rescale all text on the screen
        reposition_text(width, height);

        // Upload new text
        upload();
    }
    inline void set_debug_title(const char *title)
    {
        // Clear and reset the stream
        clear_stream();

        // Title text
        _stream << title;
        update_text(0, _stream.str());
    }
    inline void set_debug_vendor(const char *vendor)
    {
        // Clear and reset the stream
        clear_stream();

        // Vendor text
        _stream << vendor;
        update_text(1, _stream.str());
    }
    inline void set_debug_renderer(const char *renderer)
    {
        // Clear and reset the stream
        clear_stream();

        // Renderer text
        _stream << renderer;
        update_text(2, _stream.str());
    }
    inline void set_debug_position(const min::vec3<float> &p)
    {
        // Clear and reset the _stream
        clear_stream();

        // Update player position debug text
        _stream << std::fixed << std::setprecision(4) << "POS- X: " << p.x() << ", Y: " << p.y() << ", Z: " << p.z();
        update_text(_debug_offset, _stream.str());
    }
    inline void set_debug_direction(const min::vec3<float> &dir)
    {
        // Clear and reset the _stream
        clear_stream();

        // Update player direction debug text
        _stream << "DIR- X: " << dir.x() << ", Y: " << dir.y() << ", Z: " << dir.z();
        update_text(_debug_offset + 1, _stream.str());
    }
    inline void set_debug_mode(const std::string &mode)
    {
        // Clear and reset the _stream
        clear_stream();

        // Update the game mode text
        update_text(_debug_offset + 2, mode);
    }
    inline void set_debug_health(const float health)
    {
        // Clear and reset the _stream
        clear_stream();

        // Update the energy text
        _stream << "HEALTH: " << health;
        update_text(_debug_offset + 3, _stream.str());
    }
    inline void set_debug_energy(const float energy)
    {
        // Clear and reset the _stream
        clear_stream();

        // Update the energy text
        _stream << "ENERGY: " << energy;
        update_text(_debug_offset + 4, _stream.str());
    }
    inline void set_debug_fps(const float fps)
    {
        // Clear and reset the _stream
        clear_stream();

        // Update FPS and IDLE
        _stream << "FPS: " << std::round(fps);
        update_text(_debug_offset + 5, _stream.str());
    }
    inline void set_debug_idle(const double idle)
    {
        // Clear and reset the _stream
        clear_stream();

        // Update FPS and IDLE
        _stream << "IDLE: " << idle;
        update_text(_debug_offset + 6, _stream.str());
    }
    inline void set_debug_chunks(const size_t chunks)
    {
        // Clear and reset the _stream
        clear_stream();

        // Update FPS and IDLE
        _stream << "CHUNKS: " << chunks;
        update_text(_debug_offset + 7, _stream.str());
    }
    inline void toggle_draw_console()
    {
        _draw_console = !_draw_console;
    }
    inline void toggle_draw_debug()
    {
        _draw_debug = !_draw_debug;
    }
    inline void update_ui(const float health, const float energy)
    {
        // Clear and reset the _stream
        clear_stream();

        // Update the energy text
        _stream << static_cast<int>(std::round(health));
        update_text(_ui_offset, _stream.str());

        // Clear and reset the _stream
        clear_stream();

        // Update the energy text
        _stream << static_cast<int>(std::round(energy));
        update_text(_ui_offset + 1, _stream.str());
    }
    inline void update_console(const std::string &str)
    {
        // Update console text
        update_text(_console_offset, str);

        // Get the screen dimensions
        const std::pair<float, float> size = _text.get_screen_size();

        // Position the console elements
        const uint16_t w2 = (size.first / 2);
        _text.set_text_center(_console_offset, w2, _y_console);
    }
    inline void upload() const
    {
        // Unbind the last VAO to prevent scrambling buffers
        _text.unbind();

        // Upload the text glyphs to the GPU
        _text.upload();
    }
};
}

#endif
