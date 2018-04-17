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
#include <game/ui_config.h>
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
    static constexpr size_t _console = 0;
    static constexpr size_t _ui = _console + 1;
    static constexpr size_t _alert = _ui + 2;
    static constexpr size_t _debug = _alert + 1;
    static constexpr size_t _hover = _debug + 11;
    static constexpr size_t _end = _hover + 2;

    // Hover
    static constexpr float _hover_info_dx = (_s_hover_bg_x - _s_hover_text_x) * 0.5;
    static constexpr float _hover_info_dy = _s_hover_text_y - 90.0;
    static constexpr float _hover_name_dx = _s_hover_bg_x * 0.5 - 1.0;
    static constexpr float _hover_name_dy = _s_hover_text_y - 30.0;
    static constexpr float _ui_health_dx = _health_dx - _font_size * 3.0;
    static constexpr float _ui_energy_dx = _energy_dx + _font_size;

    // Text OpenGL stuff
    min::shader _vertex;
    min::shader _fragment;
    min::program _prog;

    // Buffer for holding text
    min::text_buffer _text;
    min::text_buffer _text_bg;
    std::vector<size_t> _indices;
    std::ostringstream _stream;
    bool _draw_console;
    bool _draw_debug;
    bool _draw_alert;
    bool _draw_ui;
    bool _draw_hover;

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
    inline void reposition_text(const min::vec2<float> &p, const uint16_t width, const uint16_t height)
    {
        // Position the console element
        const uint16_t w2 = (width / 2);
        _text.set_text_center(_console, w2, _console_dy);

        // Position the ui elements
        _text.set_text_location(_ui, w2 + _ui_health_dx, _y_ui_text);
        _text.set_text_location(_ui + 1, w2 + _ui_energy_dx, _y_ui_text);

        // Position alert element
        _text.set_text_center(_alert, w2, height + _alert_dy);

        // Rescale all debug text
        uint16_t y = height - 20;
        for (size_t i = _debug; i < _end; i++)
        {
            // Update the text location
            _text.set_text_location(i, 10, y);
            y -= _font_size;
        }

        // Position the hover elements
        _text.set_text_location(_hover, p.x() + _hover_name_dx, p.y() + _hover_name_dy);
        _text.set_text_location(_hover + 1, p.x() + _hover_info_dx, p.y() + _hover_info_dy);
    }
    inline void reserve_memory()
    {
        _text.reserve(_end);
    }
    inline void update_text(const size_t index, const std::string &s)
    {
        _text.set_text(s, index);
    }

  public:
    ui_text(const uint16_t width, const uint16_t height)
        : _vertex(memory_map::memory.get_file("data/shader/text.vertex"), GL_VERTEX_SHADER),
          _fragment(memory_map::memory.get_file("data/shader/text.fragment"), GL_FRAGMENT_SHADER),
          _prog(_vertex, _fragment),
          _text("data/fonts/open_sans.ttf", _font_size),
          _text_bg("data/fonts/open_sans.ttf", _ui_font_size),
          _draw_console(false), _draw_debug(false), _draw_alert(false),
          _draw_ui(false), _draw_hover(false)
    {
        // Update the text buffer screen dimensions
        _text.set_screen(width, height);
        _text_bg.set_screen(width, height);

        // Reserve text buffer memory
        reserve_memory();

        // Add 1 console entries
        for (size_t i = _console; i < _ui; i++)
        {
            add_text("", 0, 0);
            _text.set_line_wrap(i, _x_console_wrap, _y_console_wrap);
        }

        // Add 2 ui entries
        for (size_t i = _ui; i < _alert; i++)
        {
            add_text("", 0, 0);
        }

        // Add 1 alert entry
        for (size_t i = _alert; i < _debug; i++)
        {
            add_text("", 0, 0);
            _text.set_line_wrap(i, _x_alert_wrap, _y_alert_wrap);
        }

        // Add 11 debug entries
        for (size_t i = _debug; i < _hover; i++)
        {
            add_text("", 0, 0);
        }

        // Add 2 hover entries
        for (size_t i = _hover; i < _end; i++)
        {
            add_text("", 0, 0);
        }

        // Set hover text line wrapping
        _text.set_line_wrap(_hover, _s_hover_bg_x, _y_hover_wrap);
        _text.set_line_wrap(_hover + 1, _s_hover_text_x, _y_hover_wrap);

        // Reposition all of the text
        reposition_text(min::vec2<float>(), width, height);
    }
    void draw(const size_t bg_size) const
    {
        // Minimize draw calls by lumping togetherness
        if (_draw_console && _draw_ui && _draw_alert && _draw_debug)
        {
            bind();
            _text.draw(_console, _hover - 1);
        }
        else if (_draw_console && _draw_ui && _draw_alert && !_draw_debug)
        {
            bind();
            _text.draw(_console, _debug - 1);
        }
        else if (_draw_console && _draw_ui && !_draw_alert && !_draw_debug)
        {
            bind();
            _text.draw(_console, _alert - 1);
        }
        else if (_draw_console && _draw_ui && !_draw_alert && _draw_debug)
        {
            bind();
            _text.draw(_console, _alert - 1);
            _text.draw(_debug, _hover - 1);
        }
        else if (_draw_console && !_draw_ui && !_draw_alert && !_draw_debug)
        {
            bind();
            _text.draw(_console);
        }
        else
        {
            // For all other permutations
            bind();
            if (_draw_console)
            {
                _text.draw(_console, _ui - 1);
            }
            if (_draw_ui)
            {
                _text.draw(_ui, _alert - 1);
            }
            if (_draw_alert)
            {
                _text.draw(_alert, _debug - 1);
            }
            if (_draw_debug)
            {
                _text.draw(_debug, _hover - 1);
            }
        }

        // Draw the background text
        if (bg_size > 0)
        {
            _text_bg.bind(0);
            _text_bg.draw(0, bg_size - 1);
        }
    }
    void draw_tooltips() const
    {
        if (_draw_hover)
        {
            bind();
            _text.draw(_hover, _end - 1);
        }
    }
    inline min::text_buffer &get_bg_text()
    {
        return _text_bg;
    }
    inline bool is_draw_debug() const
    {
        return _draw_debug;
    }
    inline void set_draw_console(const bool flag)
    {
        _draw_console = flag;
    }
    inline void set_draw_debug(const bool flag)
    {
        _draw_debug = flag;
    }
    inline void set_draw_alert(const bool flag)
    {
        _draw_alert = flag;
    }
    inline void set_draw_hover(const bool flag)
    {
        _draw_hover = flag;
    }
    inline void set_draw_ui(const bool flag)
    {
        _draw_ui = flag;
    }
    inline void set_screen(const min::vec2<float> &p, const uint16_t width, const uint16_t height)
    {
        // Update the text buffer screen dimensions
        _text.set_screen(width, height);

        // Rescale all text on the screen
        reposition_text(p, width, height);

        // Upload new text
        upload();
    }
    inline void set_debug_title(const char *title)
    {
        // Clear and reset the stream
        clear_stream();

        // Title text
        _stream << title;
        update_text(_debug, _stream.str());
    }
    inline void set_debug_vendor(const char *vendor)
    {
        // Clear and reset the stream
        clear_stream();

        // Vendor text
        _stream << vendor;
        update_text(_debug + 1, _stream.str());
    }
    inline void set_debug_renderer(const char *renderer)
    {
        // Clear and reset the stream
        clear_stream();

        // Renderer text
        _stream << renderer;
        update_text(_debug + 2, _stream.str());
    }
    inline void set_debug_position(const min::vec3<float> &p)
    {
        // Clear and reset the _stream
        clear_stream();

        // Update player position debug text
        _stream << std::fixed << std::setprecision(4) << "POS- X: " << p.x() << ", Y: " << p.y() << ", Z: " << p.z();
        update_text(_debug + 3, _stream.str());
    }
    inline void set_debug_direction(const min::vec3<float> &dir)
    {
        // Clear and reset the _stream
        clear_stream();

        // Update player direction debug text
        _stream << "DIR- X: " << dir.x() << ", Y: " << dir.y() << ", Z: " << dir.z();
        update_text(_debug + 4, _stream.str());
    }
    inline void set_debug_health(const float health)
    {
        // Clear and reset the _stream
        clear_stream();

        // Update the energy text
        _stream << "HEALTH: " << health;
        update_text(_debug + 5, _stream.str());
    }
    inline void set_debug_energy(const float energy)
    {
        // Clear and reset the _stream
        clear_stream();

        // Update the energy text
        _stream << "ENERGY: " << energy;
        update_text(_debug + 6, _stream.str());
    }
    inline void set_debug_fps(const float fps)
    {
        // Clear and reset the _stream
        clear_stream();

        // Update FPS and IDLE
        _stream << "FPS: " << std::round(fps);
        update_text(_debug + 7, _stream.str());
    }
    inline void set_debug_idle(const double idle)
    {
        // Clear and reset the _stream
        clear_stream();

        // Update FPS and IDLE
        _stream << "IDLE: " << idle;
        update_text(_debug + 8, _stream.str());
    }
    inline void set_debug_chunks(const size_t chunks)
    {
        // Clear and reset the _stream
        clear_stream();

        // Update FPS and IDLE
        _stream << "CHUNKS: " << chunks;
        update_text(_debug + 9, _stream.str());
    }
    inline void set_debug_version(const std::string &str)
    {
        update_text(_debug + 10, str);
    }
    inline void toggle_draw_console()
    {
        _draw_console = !_draw_console;
    }
    inline void toggle_draw_debug()
    {
        _draw_debug = !_draw_debug;
    }
    inline void update_console(const std::string &str)
    {
        // Update console text
        update_text(_console, str);

        // Get the screen dimensions
        const std::pair<float, float> size = _text.get_screen_size();

        // Position the console elements
        const uint16_t w2 = (size.first / 2);
        _text.set_text_center(_console, w2, _console_dy);
    }
    inline void update_ui(const float health, const float energy)
    {
        // Clear and reset the _stream
        clear_stream();

        // Update the energy text
        _stream << static_cast<int>(std::round(health));
        update_text(_ui, _stream.str());

        // Clear and reset the _stream
        clear_stream();

        // Update the energy text
        _stream << static_cast<int>(std::round(energy));
        update_text(_ui + 1, _stream.str());
    }
    inline void update_ui_alert(const std::string &alert)
    {
        // Update the alert text
        update_text(_alert, alert);

        // Get the screen dimensions
        const std::pair<float, float> size = _text.get_screen_size();

        // Position the console elements
        const uint16_t w2 = (size.first / 2);
        _text.set_text_center(_alert, w2, size.second + _alert_dy);
    }
    inline void update_hover(const min::vec2<float> &p, const std::string &name, const std::string &info)
    {
        // Update the hover text
        update_text(_hover, name);
        update_text(_hover + 1, info);

        // Get the screen dimensions
        const std::pair<float, float> size = _text.get_screen_size();
        const uint16_t half_height = size.second / 2;

        // Calculate hover y offset to avoid off screen issues
        const float hover_offset = (p.y() > half_height) ? -_s_hover_text_y : 0.0;

        // Calculate name location and position element
        const float hover_name_dy = _hover_name_dy + hover_offset;
        const float x_name = p.x() + _hover_name_dx;
        const float y_name = p.y() + hover_name_dy;
        _text.set_text_center(_hover, x_name, y_name);

        // Calculate info location and position element
        const float hover_info_dy = _hover_info_dy + hover_offset;
        const float x_info = p.x() + _hover_info_dx;
        const float y_info = p.y() + hover_info_dy;
        _text.set_text_location(_hover + 1, x_info, y_info);
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
