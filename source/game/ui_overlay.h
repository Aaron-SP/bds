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
#ifndef _UI_OVERLAY__
#define _UI_OVERLAY__

#include <game/inventory.h>
#include <game/ui_bg.h>
#include <game/ui_text.h>
#include <string>

namespace game
{

class ui_overlay
{
  private:
    ui_text _text;
    ui_bg _bg;
    bool _dirty;
    const std::string _res;
    float _time;

  public:
    ui_overlay(const uniforms &uniforms, inventory *const inv, stats *const stat, const uint16_t width, const uint16_t height)
        : _text(28, width, height),
          _bg(uniforms, inv, stat, _text.get_bg_text(), width, height),
          _dirty(false), _res("Low Power!"), _time(-1.0) {}

    inline void click()
    {
        if (_bg.is_extended())
        {
            _bg.click();
        }
    }
    inline void draw_opaque() const
    {
        // Draw background ui
        _bg.draw_opaque();
    }
    inline void draw_transparent() const
    {
        // Draw background ui
        _bg.draw_transparent();

        // Draw the text
        const size_t bg_size = _bg.bg_text_size();
        _text.draw(bg_size);
    }
    inline void disable_console()
    {
        _bg.set_draw_console(false);
        _text.set_draw_console(false);
    }
    inline void enable_console()
    {
        _bg.set_draw_console(true);
        _text.set_draw_console(true);
    }
    inline const std::vector<min::mat3<float>> &get_scale() const
    {
        return _bg.get_scale();
    }
    inline const std::vector<min::mat3<float>> &get_uv() const
    {
        return _bg.get_uv();
    }
    inline bool is_extended() const
    {
        return _bg.is_extended();
    }
    inline bool overlap(const min::vec2<float> &p)
    {
        return _bg.overlap(p);
    }
    inline void reset_menu()
    {
        _bg.reset_menu();
    }
    inline void respawn()
    {
        _bg.respawn();
    }
    inline void set_console_string(const std::string &str)
    {
        // Update the console text
        _text.update_console(str);

        // Upload changes to the buffer
        _text.upload();
    }
    inline void set_cursor_aim()
    {
        _bg.set_cursor_aim();
    }
    inline void set_cursor_reload()
    {
        _bg.set_cursor_reload();
    }
    inline void set_cursor_target()
    {
        _bg.set_cursor_target();
    }
    inline void set_draw_title(const bool flag)
    {
        _bg.set_draw_title(flag);
    }
    inline void set_draw_text_ui(const bool flag)
    {
        _text.set_draw_ui(flag);
    }
    inline void set_energy(const float energy)
    {
        _bg.set_energy(energy);
    }
    inline void set_health(const float health)
    {
        _bg.set_health(health);
    }
    inline void set_key_down(const size_t index)
    {
        _bg.set_key_down(index);
    }
    inline void set_key_down_fail(const size_t index)
    {
        _bg.set_key_down_fail(index);
    }
    inline void set_key_up(const size_t index)
    {
        _bg.set_key_up(index);
    }
    inline void set_minimized(const bool flag)
    {
        _bg.set_minimized(flag);
    }
    inline void set_menu_dead()
    {
        _bg.set_menu_dead();
    }
    inline void set_menu_pause()
    {
        _bg.set_menu_pause();
    }
    inline void set_screen(const uint16_t width, const uint16_t height)
    {
        // Set bg screen dimensions
        _bg.set_screen(width, height);

        // Set text screen dimensions
        _text.set_screen(width, height);
    }
    inline void set_ui_error_resource()
    {
        // Flag dirty
        _dirty = true;

        // Enable drawing error text
        _text.set_draw_error(true);

        // Show resource message
        _text.update_ui_error(_res);

        // Add some time on the clock
        _time = 2.0;
    }
    inline ui_text &text()
    {
        return _text;
    }
    inline const ui_text &text() const
    {
        return _text;
    }
    inline void toggle_console()
    {
        _text.toggle_draw_console();
        _bg.toggle_draw_console();
    }
    inline void toggle_extend()
    {
        _bg.toggle_draw_ex();
    }
    inline void toggle_debug_text()
    {
        _text.toggle_draw_debug();
    }
    inline void update(const float dt)
    {
        _bg.update();

        // If we need to upload text
        if (_dirty)
        {
            // Flag cleaned
            _dirty = false;

            // Upload the changed text
            _text.upload();
        }

        // Decrement time on timer
        if (_time > 0.0)
        {
            _time -= dt;
            if (_time < 0.0)
            {
                // Disable error text
                _text.set_draw_error(false);
            }
        }
    }
    void update_text(const min::vec3<float> &p, const min::vec3<float> &dir,
                     const float health, const float energy, const double fps,
                     const double idle, const size_t chunks)
    {
        // Update all text and upload it
        if (_text.is_draw_debug())
        {
            _text.set_debug_position(p);
            _text.set_debug_direction(dir);
            _text.set_debug_health(health);
            _text.set_debug_energy(energy);
            _text.set_debug_fps(fps);
            _text.set_debug_idle(idle);
            _text.set_debug_chunks(chunks);
        }

        _text.update_ui(health, energy);

        // Upload the changed text
        _text.upload();
    }
};
}

#endif
