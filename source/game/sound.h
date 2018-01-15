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

#include <chrono>
#include <game/memory_map.h>
#include <min/camera.h>
#include <min/ogg.h>
#include <min/sound_buffer.h>
#include <min/wave.h>
#include <random>
#include <thread>

namespace game
{

class sound_info
{
  private:
    size_t _b;
    size_t _s;
    float _max_gain;
    float _fade_speed;
    float _gain;
    bool _fade_in;
    bool _fade_out;
    bool _play;

  public:
    sound_info(const size_t b, const size_t s, const float gain, const float fade_speed)
        : _b(b), _s(s), _max_gain(gain), _fade_speed(fade_speed), _gain(gain), _fade_in(false), _fade_out(false), _play(false) {}

    float max_gain() const
    {
        return _max_gain;
    }
    size_t buffer() const
    {
        return _b;
    }
    float fade()
    {
        // Change gain by constant factor
        if (_fade_out)
        {
            _gain -= _fade_speed;
            if (_gain < 0.0)
            {
                _gain = 0.0;
            }
        }
        else if (_fade_in)
        {
            _gain += _fade_speed;
            if (_gain > _max_gain)
            {
                _gain = _max_gain;
            }
        }

        return _gain;
    }
    bool fade_in() const
    {
        return _fade_in;
    }
    bool fade_out() const
    {
        return _fade_out;
    }
    float gain() const
    {
        return _gain;
    }
    bool playing() const
    {
        return _play;
    }
    void set_fade_in(const bool flag)
    {
        _fade_in = flag;
    }
    void set_fade_out(const bool flag)
    {
        _fade_out = flag;
    }
    void set_gain(const float gain)
    {
        _gain = gain;
    }
    void set_play(const bool flag)
    {
        _play = flag;
    }
    size_t source() const
    {
        return _s;
    }
};

class sound
{
  private:
    static constexpr size_t _max_delay = 120;
    static constexpr size_t _max_sounds = 7;
    static constexpr float _land_threshold = 3.0;
    static constexpr float _max_speed = 10.0;
    static constexpr float _fade_speed = 0.1;
    static constexpr float _fade_tol = 0.001;
    static constexpr float _fade_in = _fade_tol * 2.0;
    static constexpr float _gain_adjust = 0.025;

    // FADE FRAMES
    static constexpr float _bg_ff = 50;
    static constexpr float _charge_ff = 20;
    static constexpr float _grap_ff = 20;
    static constexpr float _jet_ff = 80;
    static constexpr float _miss_launch_ff = 20;

    // GAINS
    static constexpr float _bg_gain = 0.025;
    static constexpr float _charge_gain = 0.35;
    static constexpr float _click_gain = 0.2;
    static constexpr float _land_gain = 0.1;
    static constexpr float _ex_gain = 0.65;
    static constexpr float _grap_gain = 0.6;
    static constexpr float _jet_gain = 0.5;
    static constexpr float _miss_ex_gain = 0.75;
    static constexpr float _miss_launch_gain = 0.7;
    static constexpr float _shot_gain = 0.125;

    // FADES
    static constexpr float _charge_fade = _charge_gain / _charge_ff;
    static constexpr float _grap_fade = _grap_gain / _grap_ff;
    static constexpr float _jet_fade = _jet_gain / _jet_ff;
    static constexpr float _bg_fade = _bg_gain / _bg_ff;
    static constexpr float _miss_launch_fade = _miss_launch_gain / _miss_launch_ff;

    // GRAPPLE DROP OFF
    static constexpr float _grap_max_dist = 100.0;
    static constexpr float _grap_ref_dist = 8.0;
    static constexpr float _grap_roll = 1.0;

    // EXPLODE DROP OFF
    static constexpr float _ex_max_dist = 100.0;
    static constexpr float _ex_ref_dist = 8.0;
    static constexpr float _ex_roll = 0.5;

    min::sound_buffer _buffer;
    std::vector<sound_info> _si;
    std::vector<size_t> _music;
    std::uniform_int_distribution<size_t> _int_dist;
    std::uniform_real_distribution<double> _real_dist;
    std::mt19937 _gen;
    double _delay;
    std::chrono::high_resolution_clock::time_point _start;
    bool _waiting;
    bool _enable_bg;

    inline sound_info &bg_info()
    {
        return _si[0];
    }
    inline sound_info &charge_info()
    {
        return _si[1];
    }
    inline sound_info &click_info()
    {
        return _si[2];
    }
    inline sound_info &explode_m_info()
    {
        return _si[3];
    }
    inline sound_info &explode_s_info()
    {
        return _si[4];
    }
    inline sound_info &grapple_info()
    {
        return _si[5];
    }
    inline sound_info &jet_info()
    {
        return _si[6];
    }
    inline sound_info &land_info()
    {
        return _si[7];
    }
    inline sound_info &miss_ex_info()
    {
        return _si[8];
    }
    inline sound_info &miss_launch_info()
    {
        return _si[9];
    }
    inline sound_info &shot_info()
    {
        return _si[10];
    }
    inline void load_explosion_settings(const size_t s)
    {
        // Adjust the rolloff rate
        _buffer.set_source_rolloff(s, _ex_roll);

        // Adjust the max distance
        _buffer.set_source_max_dist(s, _ex_max_dist);

        // Adjust the reference distance
        _buffer.set_source_ref_dist(s, _ex_ref_dist);
    }
    inline void load_sound(const size_t b, const float gain, const float fade_speed = _fade_speed)
    {
        // Create a source
        const size_t s = _buffer.add_source();

        // Adjust the gain
        _buffer.set_source_gain(s, gain);

        // Bind source to sound data
        _buffer.bind(b, s);

        // Create a new sound info
        _si.emplace_back(b, s, gain, fade_speed);
    }
    inline void load_wave_sound(const min::wave &sound, const float gain, const float fade_speed = _fade_speed)
    {
        // Load a WAVE file into buffer
        const size_t b = _buffer.add_wave_pcm(sound);

        // Load the sound into the sound buffer
        load_sound(b, gain, fade_speed);
    }
    inline void load_ogg_sound(const min::ogg &sound, const float gain, const float fade_speed = _fade_speed)
    {
        // Load a WAVE file into buffer
        const size_t b = _buffer.add_ogg_pcm(sound);

        // Load the sound into the sound buffer
        load_sound(b, gain, fade_speed);
    }
    inline void load_bg_sound()
    {
        // Load a music OGG files
        const min::mem_file &ogg_file1 = memory_map::memory.get_file("data/sound/music1_s.ogg");
        const min::ogg sound1(ogg_file1);
        const min::mem_file &ogg_file2 = memory_map::memory.get_file("data/sound/music2_s.ogg");
        const min::ogg sound2(ogg_file2);

        // Create music buffers
        const size_t b1 = _buffer.add_ogg_pcm(sound1);
        const size_t b2 = _buffer.add_ogg_pcm(sound2);

        // Store music buffer indices
        _music.push_back(b1);
        _music.push_back(b2);

        // Load the first sound into the sound buffer
        load_sound(b1, _bg_gain, _bg_fade);
    }
    inline void load_charge_sound()
    {
        // Load a WAVE file
        const min::mem_file &ogg = memory_map::memory.get_file("data/sound/charge_s.ogg");
        const min::ogg sound(ogg);
        load_ogg_sound(sound, _charge_gain, _charge_fade);

        // Set the audio to loop
        _buffer.set_source_loop(charge_info().source(), true);
    }
    inline void load_click_sound()
    {
        // Load a WAVE file
        const min::mem_file &wave = memory_map::memory.get_file("data/sound/click_s.wav");
        const min::wave sound(wave);
        load_wave_sound(sound, _click_gain);
    }
    inline void load_explode_mono_sound()
    {
        // Load a WAVE file
        const min::mem_file &ogg = memory_map::memory.get_file("data/sound/explode_m.ogg");
        const min::ogg sound(ogg);
        load_ogg_sound(sound, _ex_gain);

        // Load explosion settings
        load_explosion_settings(explode_m_info().source());
    }
    inline void load_explode_stereo_sound()
    {
        // Load a WAVE file
        const min::mem_file &ogg = memory_map::memory.get_file("data/sound/explode_s.ogg");
        const min::ogg sound(ogg);
        load_ogg_sound(sound, _ex_gain);

        // Load explosion settings
        load_explosion_settings(explode_s_info().source());
    }
    inline void load_grapple_sound()
    {
        // Load a WAVE file
        const min::mem_file &ogg = memory_map::memory.get_file("data/sound/grapple_s.ogg");
        const min::ogg sound(ogg);
        load_ogg_sound(sound, _grap_gain, _grap_fade);

        // Get the grapple source
        const size_t s = grapple_info().source();

        // Adjust the rolloff rate
        _buffer.set_source_rolloff(s, _grap_roll);

        // Adjust the max distance
        _buffer.set_source_max_dist(s, _grap_max_dist);

        // Adjust the reference distance
        _buffer.set_source_ref_dist(s, _grap_ref_dist);

        // Set the audio to loop
        _buffer.set_source_loop(s, true);
    }
    inline void load_jet_sound()
    {
        // Load a WAVE file
        const min::mem_file &ogg = memory_map::memory.get_file("data/sound/jet_s.ogg");
        const min::ogg sound(ogg);
        load_ogg_sound(sound, _jet_gain, _jet_fade);

        // Set the audio to loop
        _buffer.set_source_loop(jet_info().source(), true);
    }
    inline void load_land_sound()
    {
        // Load a WAVE file
        const min::mem_file &wave = memory_map::memory.get_file("data/sound/land_s.wav");
        const min::wave sound(wave);
        load_wave_sound(sound, _land_gain);
    }
    inline void load_miss_ex_sound()
    {
        // Load a WAVE file
        const min::mem_file &ogg = memory_map::memory.get_file("data/sound/missile_ex_m.ogg");
        const min::ogg sound(ogg);
        load_ogg_sound(sound, _miss_ex_gain);

        // Load explosion settings
        load_explosion_settings(miss_ex_info().source());
    }
    inline void load_miss_launch_sound()
    {
        // Load a WAVE file
        const min::mem_file &ogg = memory_map::memory.get_file("data/sound/jet_m.ogg");
        const min::ogg sound(ogg);
        load_ogg_sound(sound, _miss_launch_gain, _miss_launch_fade);

        // Get the sound source
        const size_t s = miss_launch_info().source();

        // Load explosion settings
        load_explosion_settings(s);

        // Set the audio to loop
        _buffer.set_source_loop(s, true);
    }
    inline void load_shot_sound()
    {
        // Load a WAVE file
        const min::mem_file &wave = memory_map::memory.get_file("data/sound/shot_s.wav");
        const min::wave sound(wave);
        load_wave_sound(sound, _shot_gain);
    }
    inline void random_music()
    {
        // Get a random music index
        const size_t index = _int_dist(_gen);

        // Bind source to random music data
        _buffer.bind(_music[index], bg_info().source());
    }
    inline void random_wait()
    {
        // Update the current time
        _start = std::chrono::high_resolution_clock::now();

        // Calculate delay time
        _delay = _real_dist(_gen);
    }
    inline bool delay()
    {
        // Get the time since we started counting
        const auto now = std::chrono::high_resolution_clock::now();
        const double delta = std::chrono::duration<double>(now - _start).count();

        // Return if we need to delay
        return delta < _delay;
    }
    inline void reserve_memory()
    {
        _si.reserve(_max_sounds);
    }

  public:
    sound()
        : _int_dist(0, 1), _real_dist(0, _max_delay), _gen(std::chrono::high_resolution_clock::now().time_since_epoch().count()),
          _delay(0.0), _start{}, _waiting(false), _enable_bg(false)
    {
        // Reserve memory for sources and buffers
        reserve_memory();

        // Load background music
        load_bg_sound();

        // Load charge sound into buffer
        load_charge_sound();

        // Load click sound into buffer
        load_click_sound();

        // Load explode sound into buffer
        load_explode_mono_sound();

        // Load explode sound into buffer
        load_explode_stereo_sound();

        // Load grapple sound into buffer
        load_grapple_sound();

        // Load jet sound into buffer
        load_jet_sound();

        // Load land sound into buffer
        load_land_sound();

        // Load missile explode sound into buffer
        load_miss_ex_sound();

        // Load missile launch sound into buffer
        load_miss_launch_sound();

        // Load shot sound into buffer
        load_shot_sound();

        // Set the buffer distance model
        _buffer.set_distance_model(AL_INVERSE_DISTANCE_CLAMPED);
    }
    inline void bg_gain_up()
    {
        sound_info &si = bg_info();
        const float adjust = si.gain() + _gain_adjust;
        const float gain = (adjust >= 1.0) ? 1.0 : adjust;
        si.set_gain(gain);

        // Reset the gain
        _buffer.set_source_gain(si.source(), si.gain());
    }
    inline void bg_gain_down()
    {
        sound_info &si = bg_info();
        const float adjust = si.gain() - _gain_adjust;
        const float gain = (adjust <= 0.0) ? 0.0 : adjust;
        si.set_gain(gain);

        // Reset the gain
        _buffer.set_source_gain(si.source(), si.gain());
    }
    inline void play_bg(const bool flag)
    {
        _enable_bg = flag;
    }
    inline void play_charge()
    {
        sound_info &si = charge_info();

        // Set the fade and play flags
        si.set_fade_in(true);
        si.set_fade_out(false);
        si.set_gain(_fade_in);
        si.set_play(true);

        // Play the sound asynchronously at full gain
        _buffer.set_source_gain(si.source(), si.gain());
        _buffer.play_async(si.source());
    }
    inline void stop_charge()
    {
        sound_info &si = charge_info();

        // Turn on fading
        si.set_fade_in(false);
        si.set_fade_out(true);
    }
    inline void play_click()
    {
        _buffer.play_async(click_info().source());
    }
    inline void play_explode_mono(const min::vec3<float> &p)
    {
        // Get the explode source
        const size_t s = explode_m_info().source();

        // Set the sound position
        _buffer.set_source_position(s, p);

        // Play the sound
        _buffer.play_async(s);
    }
    inline void play_explode_stereo(const min::vec3<float> &p)
    {
        // Get the explode source
        const size_t s = explode_s_info().source();

        // Set the sound position
        _buffer.set_source_position(s, p);

        // Play the sound
        _buffer.play_async(s);
    }
    inline void play_grapple()
    {
        sound_info &si = grapple_info();

        // Set the fade and play flags
        si.set_fade_out(false);
        si.set_gain(_grap_gain);
        si.set_play(true);

        // Play the sound asynchronously at full gain
        _buffer.set_source_gain(si.source(), si.gain());
        _buffer.play_async(si.source());
    }
    inline void stop_grapple()
    {
        sound_info &si = grapple_info();

        // Turn on fading
        si.set_fade_out(true);
    }
    inline void play_jet()
    {
        sound_info &si = jet_info();

        // Set the fade and play flags
        si.set_fade_in(true);
        si.set_fade_out(false);
        si.set_gain(_fade_in);
        si.set_play(true);

        // Play the sound asynchronously at full gain
        _buffer.set_source_gain(si.source(), si.gain());
        _buffer.play_async(si.source());
    }
    inline void stop_jet()
    {
        sound_info &si = jet_info();

        // Turn on fading
        si.set_fade_in(false);
        si.set_fade_out(true);
    }
    inline void play_land(const float v)
    {
        // Get the landing sound info
        sound_info &land = land_info();

        // Set debouncer for landing, and filter out soft hits
        if (!land.playing() && v > _land_threshold)
        {
            // Flag that we are playing the sound
            land.set_play(true);

            // Limit the gain relative to maximum velocity
            const float gain = std::min(1.0f, _land_gain * (v / _max_speed));

            // Get the land source
            const size_t s = land_info().source();

            // Adjust the gain
            _buffer.set_source_gain(s, gain);

            // Play the land sound asynchronously
            _buffer.play_async(s);
        }
    }
    inline void play_miss_ex(const min::vec3<float> &p)
    {
        // Get the missile explode source
        const size_t s = miss_ex_info().source();

        // Set the sound position
        _buffer.set_source_position(s, p);

        // Play the sound
        _buffer.play_async(s);
    }
    inline void play_miss_launch(const min::vec3<float> &p)
    {
        sound_info &si = miss_launch_info();

        // Set the fade and play flags
        si.set_fade_out(false);
        si.set_gain(_miss_launch_gain);
        si.set_play(true);

        // Get the sound source
        const size_t s = si.source();

        // Set the sound position and gain
        _buffer.set_source_position(s, p);
        _buffer.set_source_gain(s, si.gain());

        // Play the sound asynchronously at full gain
        _buffer.play_async(s);
    }
    inline void stop_miss_launch()
    {
        sound_info &si = miss_launch_info();

        // Turn on fading
        si.set_fade_out(true);
    }
    inline void update_miss_launch(const min::vec3<float> &p)
    {
        sound_info &si = miss_launch_info();

        // Set the sound position
        _buffer.set_source_position(si.source(), p);
    }
    inline void play_shot()
    {
        _buffer.play_async(shot_info().source());
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

        // DO NOT UPDATE STEREO POSITIONS
        // CLICK == JET == LAND == SHOT == STEREO

        // If music is not disabled
        if (_enable_bg)
        {
            // If we are not playing any music
            sound_info &bg_si = bg_info();
            if (!bg_si.playing())
            {
                // If we haven't calculated a delay time yet
                if (!_waiting)
                {
                    // Calculate a random delay
                    random_wait();

                    // Flag that we are waiting
                    _waiting = true;
                }

                // If we are done waiting
                if (!delay())
                {
                    // Get a random track
                    random_music();

                    // Set the sound is playing
                    bg_si.set_fade_in(true);
                    bg_si.set_gain(_fade_in);
                    bg_si.set_play(true);

                    // Play the background music
                    _buffer.play_async(bg_si.source());

                    // Reset the wait flag
                    _waiting = false;
                }
            }
        }

        // Update all sounds
        for (sound_info &si : _si)
        {
            if (si.fade_in() && si.playing())
            {
                // Get the sound source
                const size_t s = si.source();

                // Fade in the sound for this frame
                const float gain = si.fade();
                if (gain > si.max_gain() - _fade_tol)
                {
                    si.set_fade_in(false);
                }

                // Set the faded gain
                _buffer.set_source_gain(s, gain);
            }
            else if (si.fade_out() && si.playing())
            {
                // Get the sound source
                const size_t s = si.source();

                // Fade out the sound for this frame
                const float gain = si.fade();
                if (gain < _fade_tol)
                {
                    // Stop playing the sound because gain is too low
                    _buffer.stop_async(s);
                    si.set_fade_out(false);
                    si.set_play(false);
                }

                // Set the faded gain
                _buffer.set_source_gain(s, gain);
            }
            else if (si.playing())
            {
                // Update the playing sound flags
                si.set_play(_buffer.is_playing(si.source()));
            }
        }
    }
};
}

#endif
