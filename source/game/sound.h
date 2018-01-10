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
#ifndef __SOUND__
#define __SOUND__

#include <min/sound_buffer.h>
#include <min/wave.h>

namespace game
{

class sound
{
  private:
    static constexpr float _click_gain = 0.6;
    static constexpr float _land_threshold = 3.0;
    static constexpr float _land_gain = 0.1;
    static constexpr float _max_speed = 10.0;
    min::sound_buffer _buffer;
    size_t _click_b;
    size_t _click_s;
    size_t _land_b;
    size_t _land_s;
    bool _landing;

    void load_click_sound()
    {
        // Load a WAVE file
        const min::wave sound = min::wave("data/sound/click.wav");
        _click_b = _buffer.add_wave_pcm(sound);

        // Create a source
        _click_s = _buffer.add_source();

        // Adjust the gain
        _buffer.set_source_gain(_click_s, _click_gain);

        // Bind source to wave data
        _buffer.bind(_click_b, _click_s);
    }
    void load_land_sound()
    {
        // Load a WAVE file
        const min::wave sound = min::wave("data/sound/land.wav");
        _land_b = _buffer.add_wave_pcm(sound);

        // Create a source
        _land_s = _buffer.add_source();

        // Bind source to wave data
        _buffer.bind(_land_b, _land_s);
    }

  public:
    sound() : _landing(false)
    {
        // Load all of the sound into buffers
        load_click_sound();
        load_land_sound();
    }
    void play_click()
    {
        // Play the click sound asynchronously
        _buffer.play_async(_click_s);
    }
    void stop_click()
    {
        _buffer.stop_async(_click_s);
    }
    void play_land(const float v)
    {
        // Set debouncer for landing, and filter out soft hits
        if (!_landing && v > _land_threshold)
        {
            _landing = true;

            // Limit the gain relative to maximum velocity
            const float gain = std::min(1.0f, _land_gain * (v / _max_speed));

            // Adjust the gain
            _buffer.set_source_gain(_land_s, gain);

            // Play the land sound asynchronously
            _buffer.play_async(_land_s);
        }
    }
    void stop_land()
    {
        _landing = _buffer.is_playing(_land_s);
    }
    void update(const min::vec3<float> &p)
    {
        // Update the listener
        _buffer.set_listener_position(p);

        // Update the click source
        _buffer.set_source_at_listener(_click_s);

        // Update the land source
        _buffer.set_source_at_listener(_land_s);
    }
};
}

#endif
