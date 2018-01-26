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

namespace game
{

class ui_overlay
{
  private:
    ui_text _text;
    ui_bg _bg;

  public:
    ui_overlay(const uniforms &uniforms, const inventory *const inv, const uint16_t width, const uint16_t height)
        : _text(28, width, height),
          _bg(uniforms, inv, width, height) {}

    inline void draw(const uniforms &uniforms) const
    {
        // Draw background ui
        _bg.draw(uniforms);

        // Draw the text
        _text.draw();
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
    void update(inventory &inv)
    {
        // Update the inventory matrices if dirty
        if (inv.dirty())
        {
            // Get all updated slots
            const std::vector<inv_id> &updates = inv.get_updates();

            // Update all slots
            for (auto i : updates)
            {
                const item &it = inv[i.index()];
                _bg.update_inv_slot(i, it.id());
            }

            // Flag that we clean updated the inventory state
            inv.clean();
        }
    }
    void update_text(const min::vec3<float> &p, const min::vec3<float> &f, const std::string &mode,
                     const float health, const float energy, const double fps, const double idle, const size_t chunks)
    {
        // Update all text and upload it
        _text.update_debug_text(p, f, mode, health, energy, fps, idle, chunks);
        _text.update_ui(health, energy);

        // Upload the changed text
        _text.upload();
    }
};
}

#endif
