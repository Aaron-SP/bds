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
#include <game/stats.h>
#include <game/ui_bg.h>
#include <game/ui_text.h>
#include <string>
#include <utility>

namespace game
{

class ui_overlay
{
  private:
    ui_text _text;
    ui_bg _bg;
    int _order;
    const std::string _action_fail;
    const std::string _ast;
    const std::string _health;
    const std::string _inside;
    const std::string _intro;
    const std::string _item;
    const std::string _item_fail;
    const std::string _level;
    const std::string _oxygen;
    const std::string _peace;
    const std::string _power;
    const std::string _res;
    const std::string _drone;
    const std::string _drone_kill;
    const std::string _thrust;
    float _time;
    uint_fast8_t _mult;

    inline void set_ui_alert(const std::string &str, const float time, const int order)
    {
        // If alert is higher precedence
        if (order > _order)
        {
            // Enable drawing alert text
            _text.set_draw_alert(true);

            // Show resource message
            _text.set_ui_alert(str);

            // Add some time on the clock
            _time = time;

            // Set the current UI alert order
            _order = order;
        }
    }

  public:
    ui_overlay(const uniforms &uniforms, inventory &inv, stats &stat, const uint_fast16_t width, const uint_fast16_t height)
        : _text(width, height),
          _bg(uniforms, inv, stat, _text.get_bg_text(), width, height),
          _order(-1),
          _action_fail("Can't use or craft that item!"),
          _ast("Incoming asteroids, take cover!"),
          _health("Low Health!"),
          _inside("Can't place block inside player!"),
          _intro("You awaken in an unfamiliar, mysterious place."),
          _item("You received a random item!"),
          _item_fail("You need a key to open this chest!"),
          _level("Level up!"),
          _oxygen("Low Oxygen!"),
          _peace("Everything seems peaceful!"),
          _power("Low Power!"),
          _res("Not enough blocks/ether for that operation!"),
          _drone("Space pirates have invaded your planet!"),
          _drone_kill("Space pirates pillaged all your belongings!"),
          _thrust("Thrusters are now online!"),
          _time(-1.0), _mult(1) {}

    void add_stream_float(const std::string &str, const float value)
    {
        _text.add_stream_float(str, value);
    }
    inline void add_stream_text(const std::string &str)
    {
        _text.add_stream_text(str);
    }
    inline bool action_hover()
    {
        // Check if we failed action
        const std::pair<bool, ui_id> action = _bg.action_hover(_mult);
        if (!action.first && action.second.type() != ui_type::button)
        {
            set_alert_action_fail();
        }

        // Return action status
        return action.first;
    }
    inline bool action_select()
    {
        // Check if we failed action
        const std::pair<bool, ui_id> action = _bg.action_select(_mult);
        if (!action.first && action.second.type() != ui_type::button)
        {
            set_alert_action_fail();
        }

        // Return action status
        return action.first;
    }
    inline void blink_console()
    {
        _text.toggle_draw_console();
        _bg.toggle_draw_console();

        // Upload the changed text
        _text.upload();
    }
    inline bool click_down()
    {
        if (_bg.is_extended())
        {
            return _bg.click_down();
        }

        return false;
    }
    inline void click_up()
    {
        return _bg.click_up();
    }
    inline void draw_opaque() const
    {
        // Draw background ui
        _bg.draw_opaque();
    }
    inline void draw_tooltips() const
    {
        // Disable the depth test
        glDisable(GL_DEPTH_TEST);

        // Draw the tooltip bg
        _bg.draw_tooltips();

        // Draw the tooltip text
        _text.draw_tooltips();

        // Enable the depth test
        glEnable(GL_DEPTH_TEST);
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
    inline bool drop()
    {
        // If UI is extended
        if (is_extended())
        {
            return _bg.drop();
        }

        // Didn't drop anything
        return false;
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
    inline ui_id get_selected() const
    {
        return _bg.get_selected();
    }
    inline const std::vector<min::mat3<float>> &get_uv() const
    {
        return _bg.get_uv();
    }
    inline bool is_extended() const
    {
        return _bg.is_extended();
    }
    inline bool is_focused() const
    {
        return _bg.is_focused();
    }
    inline bool overlap(const min::vec2<float> &p)
    {
        // Test for overlap on UI
        const std::pair<bool, ui_id> overlap = _bg.overlap(p);

        // Show tooltips here!
        if (overlap.first && overlap.second.type() != ui_type::button)
        {
            // Get the hover text
            const ui_info info = _bg.get_ui_info();

            // Enable drawing hover text
            const bool draw_stats = (info.type() == item_type::skill);
            _text.set_draw_hover(true, draw_stats);

            // Update the hover text
            _text.set_hover(p, info);
        }
        else
        {
            // Disable drawing hover text
            _text.set_draw_hover(false, false);
        }

        // Are we overlapping a UI element?
        return overlap.first;
    }
    inline void reset_menu()
    {
        _bg.reset_menu();
    }
    inline void respawn()
    {
        _bg.respawn();
    }
    inline void set_alert_action_fail()
    {
        set_ui_alert(_action_fail, 2.0, 1);
    }
    inline void set_alert_asteroid()
    {
        set_ui_alert(_ast, 5.0, 4);
    }
    inline void set_alert_block_inside()
    {
        set_ui_alert(_inside, 2.0, 1);
    }
    inline void set_alert_intro()
    {
        set_ui_alert(_intro, 10.0, 5);
    }
    inline void set_alert_item()
    {
        set_ui_alert(_item, 5.0, 3);
    }
    inline void set_alert_item_fail()
    {
        set_ui_alert(_item_fail, 5.0, 3);
    }
    inline void set_alert_level()
    {
        set_ui_alert(_level, 10.0, 5);
    }
    inline void set_alert_peace()
    {
        set_ui_alert(_peace, 5.0, 4);
    }
    inline void set_alert_low_power()
    {
        set_ui_alert(_power, 2.0, 1);
    }
    inline void set_alert_low_resource()
    {
        set_ui_alert(_res, 2.0, 1);
    }
    inline void set_alert_drone()
    {
        set_ui_alert(_drone, 5.0, 4);
    }
    inline void set_alert_drone_kill()
    {
        set_ui_alert(_drone_kill, 5.0, 4);
    }
    inline void set_alert_thruster()
    {
        set_ui_alert(_thrust, 10.0, 6);
    }
    inline void set_console_string(const std::string &str)
    {
        _text.set_console(str);
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
    inline void set_draw_text_ui(const bool flag)
    {
        _text.set_draw_ui(flag);
    }
    inline void set_draw_timer(const bool flag)
    {
        _text.set_draw_timer(flag);
    }
    inline void set_draw_title(const bool flag)
    {
        _bg.set_draw_title(flag);
    }
    inline void set_energy(const float energy)
    {
        _bg.set_energy(energy);
    }
    inline void set_experience(const float exp)
    {
        _bg.set_exp(exp);
    }
    inline void set_draw_focus(const bool flag)
    {
        _bg.set_draw_focus(flag);
        _text.set_draw_focus(flag);
    }
    inline void set_focus(const float bar)
    {
        _bg.set_focus(bar);
    }
    inline void set_focus_string(const std::string &str)
    {
        _text.set_focus(str);
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
    inline void set_oxygen(const float oxygen)
    {
        _bg.set_oxygen(oxygen);
    }
    inline void set_menu_dead()
    {
        _bg.set_menu_dead();
    }
    inline void set_menu_pause()
    {
        _bg.set_menu_pause();
    }
    inline void set_minimized(const bool flag)
    {
        _bg.set_minimized(flag);
    }
    inline void set_multiplier(const uint_fast8_t mult)
    {
        _mult = mult;
    }
    inline void set_screen(const min::vec2<float> &p, const uint_fast16_t width, const uint_fast16_t height)
    {
        // Set bg screen dimensions
        _bg.set_screen(p, width, height);

        // Set text screen dimensions
        _text.set_screen(p, width, height);
    }
    inline void stream_low_health()
    {
        add_stream_text(_health);
    }
    inline void stream_low_oxygen()
    {
        add_stream_text(_oxygen);
    }
    inline ui_text &text()
    {
        return _text;
    }
    inline const ui_text &text() const
    {
        return _text;
    }
    inline void toggle_debug_text()
    {
        _text.toggle_draw_debug();
    }
    inline void toggle_extend()
    {
        // Toggle bg and text extended flags
        _bg.toggle_draw_ex();

        // Force reloading of hover flag
        _text.set_draw_hover(false, false);
    }
    inline void update(const min::vec3<float> &p, const min::vec3<float> &dir,
                       const float health, const float energy, const double fps,
                       const double idle, const size_t chunks, const size_t insts,
                       const std::string &target, const float time, const float dt)
    {
        // Update bg
        _bg.update();

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
            _text.set_debug_insts(insts);
            _text.set_debug_target(target);
        }

        // Update the drone timer
        _text.set_timer(time);

        // Update ui text
        _text.set_ui(health, energy);

        // Update stream text
        _text.update_stream(dt);

        // Upload the changed text
        _text.upload();

        // Decrement time on timer
        if (_time > 0.0)
        {
            _time -= dt;
            if (_time < 0.0)
            {
                // Disable alert text
                _text.set_draw_alert(false);

                // Unlock ui if locked
                _order = -1;
            }
        }
    }
};
}

#endif
