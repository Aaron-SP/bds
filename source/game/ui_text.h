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

#include <array>
#include <game/file.h>
#include <game/memory_map.h>
#include <game/ui_bg_assets.h>
#include <game/ui_config.h>
#include <game/ui_info.h>
#include <game/ui_menu.h>
#include <iomanip>
#include <min/program.h>
#include <min/shader.h>
#include <min/text_buffer.h>
#include <sstream>
#include <vector>

namespace game
{

class stream_text
{
  private:
    float _time;

  public:
    stream_text() : _time(-1.0) {}
    inline void dec_time(const float time)
    {
        _time -= time;
    }
    inline float get_time() const
    {
        return _time;
    }
    inline void set_time(const float time)
    {
        _time = time;
    }
};

class ui_text
{
  private:
    static constexpr size_t _max_stream = 10;
    static constexpr size_t _console = 0;
    static constexpr size_t _focus = _console + 1;
    static constexpr size_t _timer = _focus + 1;
    static constexpr size_t _ui = _timer + 1;
    static constexpr size_t _alert = _ui + 2;
    static constexpr size_t _debug = _alert + 1;
    static constexpr size_t _stream = _debug + 14;
    static constexpr size_t _menu = _stream + _max_stream;
    static constexpr size_t _text_end = _menu + ui_menu::max_size();

    static constexpr size_t _hover = 0;
    static constexpr size_t _info_end = _hover + 4;

    // Hover
    static constexpr float _hover_info_dx = (_s_hover_bg_x - _s_hover_text_x) * 0.5;
    static constexpr float _hover_info_dy = _s_hover_text_y - 70.0;
    static constexpr float _hover_name_dx = _s_hover_bg_x * 0.5 - 1.0;
    static constexpr float _hover_name_dy = _s_hover_text_y - 30.0;
    static constexpr float _ui_health_dx = _health_dx - _text_font_size * 3.0;
    static constexpr float _ui_energy_dx = _energy_dx + _text_font_size;
    static constexpr float _max_stream_time = 1.0;
    static constexpr float _stream_freq = 10.0;
    static constexpr float _stream_scroll = 400.0;
    static constexpr float _stream_stride = 25.0;

    // Text OpenGL stuff
    min::shader _vertex;
    min::shader _fragment;
    min::program _prog;
    const GLint _index_location;

    // Buffer for holding text
    const ui_menu *const _ui_menu;
    min::text_buffer _text;
    min::text_buffer _text_bg;
    min::text_buffer _text_info;
    std::vector<size_t> _indices;
    size_t _main_batch;
    size_t _stream_batch;
    std::array<stream_text, _max_stream> _st;
    size_t _stream_old;
    std::ostringstream _ss;
    bool _draw_alert;
    bool _draw_console;
    bool _draw_debug;
    bool _draw_focus;
    bool _draw_hover;
    bool _draw_menu;
    bool _draw_stats;
    bool _draw_timer;
    bool _draw_ui;

    inline void clear_stream()
    {
        _ss.clear();
        _ss.str(std::string());
    }
    inline GLint load_program_index() const
    {
        // Bind the text program
        _prog.use();

        // Get the start_index uniform location
        const GLint index_location = glGetUniformLocation(_prog.id(), "ref_color");
        if (index_location == -1)
        {
            throw std::runtime_error("ui_text: could not find uniform 'ref_color'");
        }

        // Return the index
        return index_location;
    }
    inline void reposition_text(const min::vec2<float> &p, const uint_fast16_t width, const uint_fast16_t height)
    {
        // Position the console element
        const uint_fast16_t w2 = (width / 2);
        _text.set_text_center(_console, w2, _console_dy);

        // Position the focus element
        _text.set_text_center(_focus, w2, height - _focus_text_dy);

        // Position the timer element
        _text.set_text_center(_timer, w2, height - _timer_text_dy);

        // Position the ui elements
        _text.set_text_location(_ui, w2 + _ui_health_dx, _y_ui_text);
        _text.set_text_location(_ui + 1, w2 + _ui_energy_dx, _y_ui_text);

        // Position alert element
        _text.set_text_center(_alert, w2, height + _alert_dy);

        // Rescale all debug text
        uint_fast16_t y = height - 20;
        for (size_t i = _debug; i < _stream; i++)
        {
            // Update the text location
            _text.set_text_location(i, 10, y);
            y -= _text_font_size;
        }

        // Update stream text
        for (size_t i = _stream; i < _menu; i++)
        {
            // If this stream text is being used
            const float time = _st[i - _stream].get_time();
            if (time > 0.0)
            {
                // Update stream text location
                const float y = _stream_dy + (_max_stream_time - time);
                _text.set_text_center(i, w2, y);
            }
        }

        // Update menu text
        for (size_t i = _menu; i < _text_end; i++)
        {
            // Update stream text location
            const size_t index = i - _menu;
            const min::vec2<float> p = _ui_menu->position_text(w2, index);
            _text.set_text_center(i, p.x(), p.y());
        }

        // Position the hover elements
        _text_info.set_text_location(_hover, p.x() + _hover_name_dx, p.y() + _hover_name_dy);
        _text_info.set_text_location(_hover + 1, p.x() + _hover_info_dx, p.y() + _hover_info_dy);
        _text_info.set_text_location(_hover + 2, p.x() + _hover_info_dx, p.y() + _hover_info_dy);
        _text_info.set_text_location(_hover + 3, p.x() + _hover_info_dx, p.y() + _hover_info_dy);
    }
    inline void reserve_memory()
    {
        _text.reserve(_text_end);
        _text_info.reserve(_info_end);
    }
    inline void set_reference(const float x, const float y, const float z) const
    {
        // Set the sampler reference point
        glUniform3f(_index_location, x, y, z);
    }
    inline void update_main_batch()
    {
        // Clear all indices
        _indices.clear();

        // Minimize draw calls by lumping togetherness
        // For all other permutations
        if (!_draw_menu)
        {
            if (_draw_console)
            {
                for (size_t i = _console; i < _focus; i++)
                {
                    _indices.push_back(i);
                }
            }
            if (_draw_focus)
            {
                for (size_t i = _focus; i < _timer; i++)
                {
                    _indices.push_back(i);
                }
            }
            if (_draw_timer)
            {
                for (size_t i = _timer; i < _ui; i++)
                {
                    _indices.push_back(i);
                }
            }
            if (_draw_ui)
            {
                for (size_t i = _ui; i < _alert; i++)
                {
                    _indices.push_back(i);
                }
            }
            if (_draw_alert)
            {
                for (size_t i = _alert; i < _debug; i++)
                {
                    _indices.push_back(i);
                }
            }
            if (_draw_debug)
            {
                for (size_t i = _debug; i < _stream; i++)
                {
                    _indices.push_back(i);
                }
            }
        }
        else
        {
            for (size_t i = _menu; i < _text_end; i++)
            {
                _indices.push_back(i);
            }
        }

        // Upload the text glyphs to the GPU
        _main_batch = _text.upload_batch(0, _indices);
    }
    inline void update_stream_batch()
    {
        // Clear all indices
        _indices.clear();

        // Draw stream text
        for (size_t i = _stream; i < _menu; i++)
        {
            // If we have time
            const float time = _st[i - _stream].get_time();
            if (time > 0.0)
            {
                _indices.push_back(i);
            }
        }

        // Upload the text glyphs to the GPU
        _stream_batch = _text.upload_batch(1, _indices);
    }

  public:
    ui_text(const ui_menu &menu, const uint_fast16_t width, const uint_fast16_t height)
        : _vertex(memory_map::memory.get_file("data/shader/text.vertex"), GL_VERTEX_SHADER),
          _fragment(memory_map::memory.get_file("data/shader/text.fragment"), GL_FRAGMENT_SHADER),
          _prog(_vertex, _fragment), _index_location(load_program_index()),
          _ui_menu(&menu),
          _text(DATA_FONTS "/open_sans.ttf", _text_font_size, 2),
          _text_bg(DATA_FONTS "/open_sans.ttf", _inv_font_size),
          _text_info(DATA_FONTS "/open_sans.ttf", _info_font_size),
          _main_batch(0), _stream_batch(0), _st{}, _stream_old(0),
          _draw_alert(false), _draw_console(false), _draw_debug(false),
          _draw_focus(false), _draw_hover(false), _draw_menu(false),
          _draw_stats(false), _draw_timer(false), _draw_ui(false)
    {
        // Set the stream precision
        _ss << std::fixed << std::setprecision(2);

        // Update the text buffer screen dimensions
        _text.set_screen(width, height);
        _text_bg.set_screen(width, height);
        _text_info.set_screen(width, height);

        // Reserve text buffer memory
        reserve_memory();

        // Add 1 console entries
        for (size_t i = _console; i < _focus; i++)
        {
            _text.add_text("", 0, 0);
            _text.set_line_wrap(i, _x_console_wrap, _y_console_wrap);
        }

        // Add 1 focus entry
        for (size_t i = _focus; i < _timer; i++)
        {
            _text.add_text("", 0, 0);
            _text.set_line_wrap(i, _x_focus_wrap, _y_focus_wrap);
        }

        // Add 1 timer entry
        for (size_t i = _timer; i < _ui; i++)
        {
            _text.add_text("", 0, 0);
            _text.set_line_wrap(i, _x_timer_wrap, _y_timer_wrap);
        }

        // Add 2 ui entries
        for (size_t i = _ui; i < _alert; i++)
        {
            _text.add_text("", 0, 0);
        }

        // Add 1 alert entry
        for (size_t i = _alert; i < _debug; i++)
        {
            _text.add_text("", 0, 0);
            _text.set_line_wrap(i, _x_alert_wrap, _y_alert_wrap);
        }

        // Add 11 debug entries
        for (size_t i = _debug; i < _stream; i++)
        {
            _text.add_text("", 0, 0);
        }

        // Add 10 stream entries
        for (size_t i = _stream; i < _menu; i++)
        {
            _text.add_text("", 0, 0);
            _text.set_line_wrap(i, _x_stream_wrap, _y_stream_wrap);
        }

        // Add 5 menu entries
        for (size_t i = _menu; i < _text_end; i++)
        {
            _text.add_text("", 0, 0);
            _text.set_line_wrap(i, _x_menu_wrap, _y_menu_wrap);
        }

        // Add 2 hover entries
        for (size_t i = _hover; i < _info_end; i++)
        {
            _text_info.add_text("", 0, 0);
        }

        // Set hover text line wrapping
        _text_info.set_line_wrap(_hover, _s_hover_bg_x, _y_hover_wrap);
        _text_info.set_line_wrap(_hover + 1, _s_hover_text_x, _y_hover_wrap);
        _text_info.set_line_wrap(_hover + 2, _s_hover_text_x, _y_hover_wrap);
        _text_info.set_line_wrap(_hover + 3, _s_hover_text_x, _y_hover_wrap);

        // Reposition all of the text
        reposition_text(min::vec2<float>(), width, height);
    }
    inline void reset()
    {
        // Reset
        _main_batch = 0;
        _stream_batch = 0;
        _stream_old = 0;

        // Clear stream
        clear_stream();

        // Disable all drawing
        _draw_alert = false;
        _draw_console = false;
        _draw_debug = false;
        _draw_focus = false;
        _draw_hover = false;
        _draw_menu = false;
        _draw_stats = false;
        _draw_timer = false;
        _draw_ui = false;
    }
    inline void add_stream_float(const std::string &str, const float value)
    {
        // Clear and reset the stream
        clear_stream();

        // Add float to stream
        _ss << str << value;
        add_stream_text(_ss.str());
    }
    inline void add_stream_text(const std::string &str)
    {
        // Increment next stream text
        const size_t index = (_stream_old %= _max_stream)++;

        // Set stream time
        _st[index].set_time(_max_stream_time);

        // Calculate text index
        const size_t text_index = _stream + index;

        // Update the stream text
        _text.set_text(text_index, str);

        // Get the screen dimensions
        const std::pair<uint_fast16_t, uint_fast16_t> size = _text.get_screen_size();

        // Get the center width
        const uint_fast16_t w2 = size.first / 2;
        _text.set_text_center(text_index, w2, _stream_dy);
    }
    inline void draw(const size_t bg_size) const
    {
        // Bind text buffer
        _text.bind(0);

        // Bind the text program
        _prog.use();

        // Set reference to white
        set_reference(1.0, 1.0, 1.0);

        // Draw batch
        if (_main_batch > 0)
        {
            _text.bind_buffer(0);
            _text.draw_batch(_main_batch);
        }

        // Set reference to red
        set_reference(0.4, 0.025, 0.025);

        // Draw batch
        if (_stream_batch > 0)
        {
            _text.bind_buffer(1);
            _text.draw_batch(_stream_batch);
        }

        // Set reference to white
        set_reference(1.0, 1.0, 1.0);

        // Draw the background text
        if (bg_size > 0)
        {
            _text_bg.bind(0);
            _text_bg.draw(0, bg_size - 1);
        }
    }
    inline void draw_tooltips() const
    {
        if (_draw_hover)
        {
            // Bind the text_buffer vao, and textures on channel '1'
            _text_info.bind(0);

            // Bind the text program
            _prog.use();

            // Draw title, primary, secondary stat
            set_reference(1.0, 1.0, 1.0);
            _text_info.draw(_hover);

            if (_draw_stats)
            {
                // Set reference to orange and draw primary
                set_reference(0.65, 0.275, 0.0);
                _text_info.draw(_hover + 1);

                // Set reference to blue and draw secondary
                set_reference(0.27, 0.45, 0.5);
                _text_info.draw(_hover + 2);
            }

            // Saved 0.4, 0.4, 0.2, Good light green color

            // Set reference to gold text
            set_reference(0.65, 0.5, 0.22);
            _text_info.draw(_hover + 3);
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
    inline void set_console(const std::string &str)
    {
        // Get the screen dimensions
        const std::pair<uint_fast16_t, uint_fast16_t> size = _text.get_screen_size();

        // Get the center width
        const uint_fast16_t w2 = size.first / 2;
        _text.set_text_center(_console, str, w2, _console_dy);
    }
    inline void set_draw_alert(const bool flag)
    {
        _draw_alert = flag;
    }
    inline void set_draw_console(const bool flag)
    {
        _draw_console = flag;
    }
    inline void set_draw_debug(const bool flag)
    {
        _draw_debug = flag;
    }
    inline void set_draw_focus(const bool flag)
    {
        _draw_focus = flag;
    }
    inline void set_draw_hover(const bool draw_hover, const bool draw_stats)
    {
        _draw_hover = draw_hover;
        _draw_stats = draw_stats;
    }
    inline void set_draw_menu(const bool draw_menu)
    {
        _draw_menu = draw_menu;
    }
    inline void set_draw_timer(const bool flag)
    {
        _draw_timer = flag;
    }
    inline void set_draw_ui(const bool flag)
    {
        _draw_ui = flag;
    }
    inline void set_screen(const min::vec2<float> &p, const uint_fast16_t width, const uint_fast16_t height)
    {
        // Update the text buffer screen dimensions
        _text.set_screen(width, height);
        _text_info.set_screen(width, height);

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
        _ss << title;
        _text.set_text(_debug, _ss.str());
    }
    inline void set_debug_vendor(const char *vendor)
    {
        // Clear and reset the stream
        clear_stream();

        // Vendor text
        _ss << vendor;
        _text.set_text(_debug + 1, _ss.str());
    }
    inline void set_debug_renderer(const char *renderer)
    {
        // Clear and reset the stream
        clear_stream();

        // Renderer text
        _ss << renderer;
        _text.set_text(_debug + 2, _ss.str());
    }
    inline void set_debug_position(const min::vec3<float> &p)
    {
        // Clear and reset the stream
        clear_stream();

        // Update player position debug text
        _ss << "POS- X: " << p.x() << ", Y: " << p.y() << ", Z: " << p.z();
        _text.set_text(_debug + 3, _ss.str());
    }
    inline void set_debug_direction(const min::vec3<float> &dir)
    {
        // Clear and reset the stream
        clear_stream();

        // Update player direction debug text
        _ss << "DIR- X: " << dir.x() << ", Y: " << dir.y() << ", Z: " << dir.z();
        _text.set_text(_debug + 4, _ss.str());
    }
    inline void set_debug_health(const float health)
    {
        // Clear and reset the stream
        clear_stream();

        // Update the energy text
        _ss << "HEALTH: " << health;
        _text.set_text(_debug + 5, _ss.str());
    }
    inline void set_debug_energy(const float energy)
    {
        // Clear and reset the stream
        clear_stream();

        // Update the energy text
        _ss << "ENERGY: " << energy;
        _text.set_text(_debug + 6, _ss.str());
    }
    inline void set_debug_fps(const float fps)
    {
        // Clear and reset the stream
        clear_stream();

        // Update FPS and IDLE
        _ss << "FPS: " << std::round(fps);
        _text.set_text(_debug + 7, _ss.str());
    }
    inline void set_debug_idle(const double idle)
    {
        // Clear and reset the stream
        clear_stream();

        // Update FPS and IDLE
        _ss << "IDLE: " << idle;
        _text.set_text(_debug + 8, _ss.str());
    }
    inline void set_debug_chunks(const size_t chunks)
    {
        // Clear and reset the stream
        clear_stream();

        // Update FPS and IDLE
        _ss << "CHUNKS: " << chunks;
        _text.set_text(_debug + 9, _ss.str());
    }
    inline void set_debug_insts(const size_t insts)
    {
        // Clear and reset the stream
        clear_stream();

        // Update FPS and IDLE
        _ss << "INSTANCES: " << insts;
        _text.set_text(_debug + 10, _ss.str());
    }
    inline void set_debug_target(const std::string &str)
    {
        // Clear and reset the stream
        clear_stream();

        // Update FPS and IDLE
        _ss << "TARGET: " << str;
        _text.set_text(_debug + 11, _ss.str());
    }
    inline void set_debug_version(const std::string &str)
    {
        _text.set_text(_debug + 12, str);
    }
    inline void set_debug_game_mode(const std::string &str)
    {
        _text.set_text(_debug + 13, str);
    }
    inline void set_focus(const std::string &str)
    {
        // Get the screen dimensions
        const std::pair<uint_fast16_t, uint_fast16_t> size = _text.get_screen_size();

        // Get the center width
        const uint_fast16_t w2 = size.first / 2;
        _text.set_text_center(_focus, str, w2, size.second - _focus_text_dy);
    }
    inline void set_hover(const min::vec2<float> &p, const ui_info info)
    {
        // Get the screen dimensions
        const std::pair<uint_fast16_t, uint_fast16_t> size = _text_info.get_screen_size();
        const uint_fast16_t half_height = size.second / 2;

        // Calculate hover y offset to avoid off screen issues
        const float hover_offset = (p.y() > half_height) ? -_s_hover_text_y : 0.0;

        // Set title properties
        const float hover_name_dy = _hover_name_dy + hover_offset;
        const float x_name = p.x() + _hover_name_dx;
        const float y_name = p.y() + hover_name_dy;
        _text_info.set_text_center(_hover, info.get_name(), x_name, y_name);

        const float hover_info_dy = _hover_info_dy + hover_offset;
        const float x_info = p.x() + _hover_info_dx;

        // Set starting height
        float y_info = p.y() + hover_info_dy;

        // If we are drawing stats
        if (_draw_stats)
        {
            // Clear and reset the _stream
            clear_stream();
            _ss << "Primary: " << static_cast<int>(info.primary());

            // Set primary stat
            _text_info.set_text(_hover + 1, _ss.str(), x_info, y_info);

            // Clear and reset the _stream
            clear_stream();
            _ss << "Secondary: " << static_cast<int>(info.secondary());

            // Shift down and set secondary stat
            y_info -= _info_font_size;
            _text_info.set_text(_hover + 2, _ss.str(), x_info, y_info);

            // Shift down
            y_info -= _info_font_size * 1.5;
        }

        // Set info properties
        _text_info.set_text(_hover + 3, info.get_info(), x_info, y_info);

        // Upload changes
        _text_info.upload();
    }
    inline void set_menu()
    {
        // Get the menu strings
        const auto &prefix_arr = _ui_menu->get_prefixs();
        const auto &str_arr = _ui_menu->get_strings();

        // Get the screen dimensions
        const std::pair<uint_fast16_t, uint_fast16_t> size = _text.get_screen_size();
        const uint_fast16_t w2 = size.first / 2;

        // Set all of the menu strings
        for (size_t i = _menu; i < _text_end; i++)
        {
            // Get the center width
            const size_t index = i - _menu;
            const min::vec2<float> p = _ui_menu->position_text(w2, index);

            // Clear and reset the stream
            clear_stream();

            // Combine prefix and string if given a prefix
            if (prefix_arr[index]->size() > 0)
            {
                _ss << *prefix_arr[index] << ": " << *str_arr[index];
            }
            else
            {
                _ss << *str_arr[index];
            }
            _text.set_text_center(i, _ss.str(), p.x(), p.y());
        }
    }
    inline void set_timer(const float time)
    {
        // Get the screen dimensions
        const std::pair<uint_fast16_t, uint_fast16_t> size = _text.get_screen_size();
        const uint_fast16_t w2 = size.first / 2;

        // Clear and reset the stream
        clear_stream();

        // Update the energy text
        _ss << "Next Invasion: " << time << " s";
        _text.set_text_center(_timer, _ss.str(), w2, size.second - _timer_text_dy);
    }
    inline void set_ui(const float health, const float energy)
    {
        // Clear and reset the stream
        clear_stream();

        // Update the energy text
        _ss << static_cast<int>(std::round(health));
        _text.set_text(_ui, _ss.str());

        // Clear and reset the _stream
        clear_stream();

        // Update the energy text
        _ss << static_cast<int>(std::round(energy));
        _text.set_text(_ui + 1, _ss.str());
    }
    inline void set_ui_alert(const std::string &alert)
    {
        // Get the screen dimensions
        const std::pair<uint_fast16_t, uint_fast16_t> size = _text.get_screen_size();

        // Get the center width
        const uint_fast16_t w2 = size.first / 2;
        _text.set_text_center(_alert, alert, w2, size.second + _alert_dy);
    }
    inline void toggle_draw_console()
    {
        _draw_console = !_draw_console;
    }
    inline void toggle_draw_debug()
    {
        _draw_debug = !_draw_debug;
    }
    inline void toggle_draw_focus()
    {
        _draw_focus = !_draw_focus;
    }
    inline void update_stream(const float dt)
    {
        // Get the screen dimensions
        const std::pair<uint_fast16_t, uint_fast16_t> size = _text.get_screen_size();

        // Get the center width
        const uint_fast16_t w2 = size.first / 2;

        // Update all stream elements
        for (size_t i = _stream; i < _menu; i++)
        {
            // Get stream text
            stream_text &st = _st[i - _stream];

            // If this stream text is being used
            const float time = st.get_time();
            if (time >= 0.0)
            {
                // Decrement time
                st.dec_time(dt);

                // Update stream text location
                const float accum = _max_stream_time - time;
                const float x = w2 + (std::sin(accum * _stream_freq) * _stream_stride);
                const float y = _stream_dy + (accum * _stream_scroll);
                _text.set_text_center(i, x, y);
            }
        }
    }
    inline void upload()
    {
        // Unbind the last VAO to prevent scrambling buffers
        _text.unbind();

        // Batch text into buffer
        update_main_batch();

        // Batch stream text into second buffer
        update_stream_batch();
    }
};
}

#endif
