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

#include <min/camera.h>
#include <min/sound_buffer.h>
#include <min/wave.h>

namespace game
{

class sound_info
{
  private:
    size_t _b;
    size_t _s;

  public:
    sound_info(const size_t b, const size_t s) : _b(b), _s(s) {}
    size_t buffer() const
    {
        return _b;
    }
    size_t source() const
    {
        return _s;
    }
};

class sound
{
  private:
    static constexpr size_t _max_sounds = 5;
    static constexpr float _land_threshold = 3.0;
    static constexpr float _max_speed = 10.0;

    // GAINS
    static constexpr float _click_gain = 0.6;
    static constexpr float _land_gain = 0.1;
    static constexpr float _grap_gain = 0.75;
    static constexpr float _ex_gain = 0.75;
    static constexpr float _shot_gain = 0.125;

    // GRAPPLE DROP OFF
    static constexpr float _grap_max_dist = 100.0;
    static constexpr float _grap_ref_dist = 2.0;
    static constexpr float _grap_roll = 1.0;

    // EXPLODE DROP OFF
    static constexpr float _ex_max_dist = 100.0;
    static constexpr float _ex_ref_dist = 2.0;
    static constexpr float _ex_roll = 0.5;

    min::sound_buffer _buffer;
    std::vector<sound_info> _si;
    bool _landing;

    inline size_t click_source() const
    {
        return _si[0].source();
    }
    inline size_t land_source() const
    {
        return _si[1].source();
    }
    inline size_t grapple_source() const
    {
        return _si[2].source();
    }
    inline size_t explode_source() const
    {
        return _si[3].source();
    }
    inline size_t shot_source() const
    {
        return _si[4].source();
    }

    inline void load_sound(const min::wave &sound)
    {
        // Load a WAVE file into buffer
        const size_t b = _buffer.add_wave_pcm(sound);

        // Create a source
        const size_t s = _buffer.add_source();

        // Bind source to wave data
        _buffer.bind(b, s);

        // Create a new sound info
        _si.emplace_back(b, s);
    }
    inline void load_click_sound()
    {
        // Load a WAVE file
        const min::wave sound = min::wave("data/sound/click.wav");
        load_sound(sound);

        // Adjust the gain
        _buffer.set_source_gain(click_source(), _click_gain);
    }
    inline void load_land_sound()
    {
        // Load a WAVE file
        const min::wave sound = min::wave("data/sound/land.wav");
        load_sound(sound);
    }
    inline void load_grapple_sound()
    {
        // Load a WAVE file
        const min::wave sound = min::wave("data/sound/grapple.wav");
        load_sound(sound);

        // Get the grapple source
        const size_t s = grapple_source();

        // Adjust the gain
        _buffer.set_source_gain(s, _grap_gain);

        // Adjust the rolloff rate
        _buffer.set_source_rolloff(s, _grap_roll);

        // Adjust the max distance
        _buffer.set_source_max_dist(s, _grap_max_dist);

        // Adjust the reference distance
        _buffer.set_source_ref_dist(s, _grap_ref_dist);

        // Adjust the gain
        _buffer.set_source_loop(s, true);
    }
    inline void load_explode_sound()
    {
        // Load a WAVE file
        const min::wave sound = min::wave("data/sound/explode.wav");
        load_sound(sound);

        // Get the explode source
        const size_t s = explode_source();

        // Adjust the gain
        _buffer.set_source_gain(s, _ex_gain);

        // Adjust the rolloff rate
        _buffer.set_source_rolloff(s, _ex_roll);

        // Adjust the max distance
        _buffer.set_source_max_dist(s, _ex_max_dist);

        // Adjust the reference distance
        _buffer.set_source_ref_dist(s, _ex_ref_dist);
    }
    inline void load_shot_sound()
    {
        // Load a WAVE file
        const min::wave sound = min::wave("data/sound/shot.wav");
        load_sound(sound);

        // Adjust the gain
        _buffer.set_source_gain(shot_source(), _shot_gain);
    }
    inline void reserve_memory()
    {
        _si.reserve(_max_sounds);
    }

  public:
    sound() : _landing(false)
    {
        // Reserve memory for sources and buffers
        reserve_memory();

        // Load click sound into buffer
        load_click_sound();

        // Load land sound into buffer
        load_land_sound();

        // Load grapple sound into buffer
        load_grapple_sound();

        // Load explode sound into buffer
        load_explode_sound();

        // Load shot sound into buffer
        load_shot_sound();

        // Set the buffer distance model
        _buffer.set_distance_model(AL_INVERSE_DISTANCE_CLAMPED);
    }
    inline void play_click() const
    {
        _buffer.play_async(click_source());
    }
    inline void play_land(const float v)
    {
        // Set debouncer for landing, and filter out soft hits
        if (!_landing && v > _land_threshold)
        {
            _landing = true;

            // Limit the gain relative to maximum velocity
            const float gain = std::min(1.0f, _land_gain * (v / _max_speed));

            // Get the land source
            const size_t s = land_source();

            // Adjust the gain
            _buffer.set_source_gain(s, gain);

            // Play the land sound asynchronously
            _buffer.play_async(s);
        }
    }
    inline void update_land()
    {
        _landing = _buffer.is_playing(land_source());
    }
    inline void play_explode(const min::vec3<float> &p) const
    {
        // Get the explode source
        const size_t s = explode_source();

        // Set the sound position
        _buffer.set_source_position(s, p);

        // Play the sound
        _buffer.play_async(s);
    }
    inline void play_grapple() const
    {
        _buffer.play_async(grapple_source());
    }
    inline void stop_grapple() const
    {
        _buffer.stop_async(grapple_source());
    }
    inline void play_shot() const
    {
        _buffer.play_async(shot_source());
    }
    inline void update(const min::camera<float> &cam, const min::vec3<float> &vel)
    {
        // Get camera vectors
        const min::vec3<float> &p = cam.get_position();
        const min::vec3<float> &at = cam.get_forward();
        const min::vec3<float> &up = cam.get_up();

        // Update the listener position
        _buffer.set_listener_position(p);

        // Update the listener orientation
        _buffer.set_listener_orientation(at, up);

        // Update the listener velocity
        _buffer.set_listener_velocity(vel);

        // Update the click source at listener position
        _buffer.set_source_at_listener(click_source());

        // Update the click source at listener position
        const min::vec3<float> front = cam.project_point(0.5);
        _buffer.set_source_position(shot_source(), front);

        // Update the land source below listener
        const min::vec3<float> below(p.x(), p.y() - 1.0, p.z());
        _buffer.set_source_position(land_source(), below);
    }
    inline void update_grapple_position(const min::vec3<float> &p, const min::vec3<float> &dir)
    {
        // Update the grapple source position
        const size_t s = grapple_source();
        _buffer.set_source_position(s, p);
        _buffer.set_source_direction(s, dir);
    }
};
}

#endif
