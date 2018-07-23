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
#ifndef _BDS_SOUND_BDS_
#define _BDS_SOUND_BDS_

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

    inline float max_gain() const
    {
        return _max_gain;
    }
    inline size_t buffer() const
    {
        return _b;
    }
    inline float fade()
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
    inline bool fade_in() const
    {
        return _fade_in;
    }
    inline bool fade_out() const
    {
        return _fade_out;
    }
    inline float gain() const
    {
        return _gain;
    }
    inline bool playing() const
    {
        return _play;
    }
    inline void set_fade_in(const bool flag)
    {
        _fade_in = flag;
    }
    inline void set_fade_out(const bool flag)
    {
        _fade_out = flag;
    }
    inline void set_gain(const float gain)
    {
        _gain = gain;
    }
    inline void set_play(const bool flag)
    {
        _play = flag;
    }
    inline size_t source() const
    {
        return _s;
    }
};

class sound
{
  private:
    static constexpr size_t _bg_sounds = 3;
    static constexpr size_t _drone_limit = 10;
    static constexpr size_t _ex_limit = 30;
    static constexpr size_t _miss_launch_limit = 10;
    static constexpr size_t _sounds = 16 + _drone_limit + _ex_limit + _miss_launch_limit;
    static constexpr size_t _voice_sounds = 9;
    static constexpr float _fade_tol = 0.001;
    static constexpr float _fade_in = _fade_tol * 2.0;
    static constexpr float _fade_speed = 0.1;
    static constexpr float _gain_adjust = 0.01;
    static constexpr float _land_threshold = 3.0;
    static constexpr float _max_delay = 120.0;
    static constexpr float _max_speed = 10.0;

    // FADE FRAMES
    static constexpr float _bg_ff = 50;
    static constexpr float _charge_ff = 20;
    static constexpr float _drone_ff = 20;
    static constexpr float _grap_ff = 20;
    static constexpr float _jet_ff = 80;
    static constexpr float _miss_launch_ff = 20;
    static constexpr float _oxygen_ff = 80;

    // GAINS
    static constexpr float _bg_gain = 0.05;
    static constexpr float _blast_gain = 0.65;
    static constexpr float _charge_gain = 0.35;
    static constexpr float _click_gain = 0.2;
    static constexpr float _drone_gain = 0.125;
    static constexpr float _ex_gain = 0.75;
    static constexpr float _focus_gain = 0.1;
    static constexpr float _grap_gain = 0.6;
    static constexpr float _land_gain = 0.1;
    static constexpr float _jet_gain = 0.5;
    static constexpr float _miss_launch_gain = 0.7;
    static constexpr float _oxygen_gain = 0.5;
    static constexpr float _pickup_gain = 0.25;
    static constexpr float _shot_gain = 0.25;
    static constexpr float _shot_ex_gain = 0.25;
    static constexpr float _thrust_gain = 0.15;
    static constexpr float _voice_gain = 0.25;
    static constexpr float _zap_gain = 0.5;

    // FADES
    static constexpr float _bg_fade = _bg_gain / _bg_ff;
    static constexpr float _drone_fade = _drone_gain / _drone_ff;
    static constexpr float _charge_fade = _charge_gain / _charge_ff;
    static constexpr float _grap_fade = _grap_gain / _grap_ff;
    static constexpr float _jet_fade = _jet_gain / _jet_ff;
    static constexpr float _oxygen_fade = _oxygen_gain / _oxygen_ff;
    static constexpr float _miss_launch_fade = _miss_launch_gain / _miss_launch_ff;

    // DRONE DROP OFF
    static constexpr float _drone_max_dist = 10.0;
    static constexpr float _drone_ref_dist = 2.0;
    static constexpr float _drone_roll = 4.0;

    // EXPLODE DROP OFF
    static constexpr float _ex_max_dist = 100.0;
    static constexpr float _ex_ref_dist = 8.0;
    static constexpr float _ex_roll = 0.5;

    // GRAPPLE DROP OFF
    static constexpr float _grap_max_dist = 100.0;
    static constexpr float _grap_ref_dist = 8.0;
    static constexpr float _grap_roll = 1.0;

    min::sound_buffer _buffer;
    std::vector<sound_info> _si;
    std::vector<size_t> _music;
    float _bg_delay;
    bool _bg_enable;
    std::vector<size_t> _drone;
    size_t _drone_old;
    std::vector<size_t> _ex;
    size_t _ex_old;
    std::vector<size_t> _miss_launch;
    size_t _miss_launch_old;
    std::vector<size_t> _voice;
    std::vector<size_t> _v_queue;
    size_t _v_head;
    float _v_delay;
    bool _v_enable;
    std::uniform_int_distribution<size_t> _int_dist;
    std::uniform_real_distribution<float> _real_dist;
    std::mt19937 _gen;

    inline sound_info &bg_info()
    {
        return _si[0];
    }
    inline sound_info &blast_m_info()
    {
        return _si[1];
    }
    inline sound_info &blast_s_info()
    {
        return _si[2];
    }
    inline sound_info &charge_info()
    {
        return _si[3];
    }
    inline sound_info &click_info()
    {
        return _si[4];
    }
    inline sound_info &focus_info()
    {
        return _si[5];
    }
    inline sound_info &grapple_info()
    {
        return _si[6];
    }
    inline sound_info &jet_info()
    {
        return _si[7];
    }
    inline sound_info &land_info()
    {
        return _si[8];
    }
    inline sound_info &oxygen_info()
    {
        return _si[9];
    }
    inline sound_info &pickup_info()
    {
        return _si[10];
    }
    inline sound_info &shot_info()
    {
        return _si[11];
    }
    inline sound_info &shot_ex_info()
    {
        return _si[12];
    }
    inline sound_info &thrust_info()
    {
        return _si[13];
    }
    inline sound_info &voice_info()
    {
        return _si[14];
    }
    inline sound_info &zap_info()
    {
        return _si[15];
    }
    inline sound_info &drone_info(const size_t index)
    {
        return _si[16 + index];
    }
    inline sound_info &ex_info(const size_t index)
    {
        constexpr size_t offset = 16 + _drone_limit;
        return _si[offset + index];
    }
    inline sound_info &miss_launch_info(const size_t index)
    {
        constexpr size_t offset = 16 + _drone_limit + _ex_limit;
        return _si[offset + index];
    }
    inline static size_t v_comply()
    {
        return 0;
    }
    inline static size_t v_critical()
    {
        return 1;
    }
    inline static size_t v_level()
    {
        return 2;
    }
    inline static size_t v_portal_alert()
    {
        return 3;
    }
    inline static size_t v_power()
    {
        return 4;
    }
    inline static size_t v_repair()
    {
        return 5;
    }
    inline static size_t v_resource()
    {
        return 6;
    }
    inline static size_t v_shutdown()
    {
        return 7;
    }
    inline static size_t v_thrust_alert()
    {
        return 8;
    }
    inline void load_drone_settings(const size_t s)
    {
        // Adjust the rolloff rate
        _buffer.set_source_rolloff(s, _drone_roll);

        // Adjust the max distance
        _buffer.set_source_max_dist(s, _drone_max_dist);

        // Adjust the reference distance
        _buffer.set_source_ref_dist(s, _drone_ref_dist);
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
        // Load a OGG file into buffer
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
        const min::mem_file &ogg_file3 = memory_map::memory.get_file("data/sound/music3_s.ogg");
        const min::ogg sound3(ogg_file3);

        // Create music buffers
        _music[0] = _buffer.add_ogg_pcm(sound1);
        _music[1] = _buffer.add_ogg_pcm(sound2);
        _music[2] = _buffer.add_ogg_pcm(sound3);

        // Load the first sound into the sound buffer
        load_sound(_music[0], _bg_gain, _bg_fade);
    }
    inline void load_charge_sound()
    {
        // Load a OGG file
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
    inline void load_drone_sound()
    {
        // Load a OGG file
        const min::mem_file &ogg = memory_map::memory.get_file("data/sound/drone_m.ogg");
        const min::ogg sound(ogg);

        // Load a OGG file into buffer
        const size_t b = _buffer.add_ogg_pcm(sound);

        // Get the gains and fades for this sound
        const float gain = _drone_gain;
        const float fade = _drone_fade;

        // Create many sources
        for (size_t i = 0; i < _drone_limit; i++)
        {
            _drone[i] = _buffer.add_source();

            // Adjust the gain
            _buffer.set_source_gain(_drone[i], gain);

            // Set the audio to loop
            _buffer.set_source_loop(_drone[i], true);

            // Load drone settings
            load_drone_settings(_drone[i]);

            // Bind source to sound data
            _buffer.bind(b, _drone[i]);

            // Create a new sound info
            _si.emplace_back(b, _drone[i], gain, fade);
        }
    }
    inline void load_blast_mono_sound()
    {
        // Load a OGG file
        const min::mem_file &ogg = memory_map::memory.get_file("data/sound/blast_m.ogg");
        const min::ogg sound(ogg);
        load_ogg_sound(sound, _blast_gain);

        // Load explosion settings
        load_explosion_settings(blast_m_info().source());
    }
    inline void load_blast_stereo_sound()
    {
        // Load a OGG file
        const min::mem_file &ogg = memory_map::memory.get_file("data/sound/blast_s.ogg");
        const min::ogg sound(ogg);
        load_ogg_sound(sound, _blast_gain);

        // Load explosion settings
        load_explosion_settings(blast_s_info().source());
    }
    inline void load_focus_sound()
    {
        // Load a OGG file
        const min::mem_file &ogg = memory_map::memory.get_file("data/sound/focus_s.ogg");
        const min::ogg sound(ogg);
        load_ogg_sound(sound, _focus_gain);
    }
    inline void load_grapple_sound()
    {
        // Load a OGG file
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
        // Load a OGG file
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
    inline void load_explode_sound()
    {
        // Load a OGG file
        const min::mem_file &ogg = memory_map::memory.get_file("data/sound/explode_m.ogg");
        const min::ogg sound(ogg);

        // Load a OGG file into buffer
        const size_t b = _buffer.add_ogg_pcm(sound);

        // Get the gains and fades for this sound
        const float gain = _ex_gain;
        const float fade = _fade_speed;

        // Create many sources
        for (size_t i = 0; i < _ex_limit; i++)
        {
            _ex[i] = _buffer.add_source();

            // Adjust the gain
            _buffer.set_source_gain(_ex[i], gain);

            // Load explosion settings
            load_explosion_settings(_ex[i]);

            // Bind source to sound data
            _buffer.bind(b, _ex[i]);

            // Create a new sound info
            _si.emplace_back(b, _ex[i], gain, fade);
        }
    }
    inline void load_miss_launch_sound()
    {
        // Load a OGG file
        const min::mem_file &ogg = memory_map::memory.get_file("data/sound/jet_m.ogg");
        const min::ogg sound(ogg);

        // Load a OGG file into buffer
        const size_t b = _buffer.add_ogg_pcm(sound);

        // Get the gains and fades for this sound
        const float gain = _miss_launch_gain;
        const float fade = _miss_launch_fade;

        // Create many sources
        for (size_t i = 0; i < _miss_launch_limit; i++)
        {
            _miss_launch[i] = _buffer.add_source();

            // Adjust the gain
            _buffer.set_source_gain(_miss_launch[i], gain);

            // Load explosion settings
            load_explosion_settings(_miss_launch[i]);

            // Set the audio to loop
            _buffer.set_source_loop(_miss_launch[i], true);

            // Bind source to sound data
            _buffer.bind(b, _miss_launch[i]);

            // Create a new sound info
            _si.emplace_back(b, _miss_launch[i], gain, fade);
        }
    }
    inline void load_oxygen_sound()
    {
        // Load a OGG file
        const min::mem_file &ogg = memory_map::memory.get_file("data/sound/oxygen_s.ogg");
        const min::ogg sound(ogg);
        load_ogg_sound(sound, _oxygen_gain, _oxygen_fade);

        // Set the audio to loop
        _buffer.set_source_loop(oxygen_info().source(), true);
    }
    inline void load_pickup_sound()
    {
        // Load a OGG file
        const min::mem_file &ogg = memory_map::memory.get_file("data/sound/pickup_s.ogg");
        const min::ogg sound(ogg);
        load_ogg_sound(sound, _pickup_gain);
    }
    inline void load_shot_sound()
    {
        // Load a OGG file
        const min::mem_file &ogg = memory_map::memory.get_file("data/sound/shot_s.ogg");
        const min::ogg sound(ogg);
        load_ogg_sound(sound, _shot_gain);
    }
    inline void load_shot_ex_sound()
    {
        // Load a OGG file
        const min::mem_file &ogg = memory_map::memory.get_file("data/sound/shot_ex_m.ogg");
        const min::ogg sound(ogg);
        load_ogg_sound(sound, _shot_ex_gain);

        // Load explosion settings
        load_explosion_settings(shot_ex_info().source());
    }
    inline void load_thrust_sound()
    {
        // Load a OGG file
        const min::mem_file &ogg = memory_map::memory.get_file("data/sound/thrust_s.ogg");
        const min::ogg sound(ogg);
        load_ogg_sound(sound, _thrust_gain);
    }
    inline void load_zap_sound()
    {
        // Load a OGG file
        const min::mem_file &ogg = memory_map::memory.get_file("data/sound/zap_m.ogg");
        const min::ogg sound(ogg);
        load_ogg_sound(sound, _zap_gain);
    }
    inline void load_voice_sound()
    {
        // Load a music OGG files
        const min::mem_file &ogg1 = memory_map::memory.get_file("data/sound/voice_comply_s.ogg");
        const min::ogg sound1(ogg1);
        const min::mem_file &ogg2 = memory_map::memory.get_file("data/sound/voice_critical_s.ogg");
        const min::ogg sound2(ogg2);
        const min::mem_file &ogg3 = memory_map::memory.get_file("data/sound/voice_level_s.ogg");
        const min::ogg sound3(ogg3);
        const min::mem_file &ogg4 = memory_map::memory.get_file("data/sound/voice_portal_alert_s.ogg");
        const min::ogg sound4(ogg4);
        const min::mem_file &ogg5 = memory_map::memory.get_file("data/sound/voice_power_s.ogg");
        const min::ogg sound5(ogg5);
        const min::mem_file &ogg6 = memory_map::memory.get_file("data/sound/voice_repair_s.ogg");
        const min::ogg sound6(ogg6);
        const min::mem_file &ogg7 = memory_map::memory.get_file("data/sound/voice_resource_s.ogg");
        const min::ogg sound7(ogg7);
        const min::mem_file &ogg8 = memory_map::memory.get_file("data/sound/voice_shutdown_s.ogg");
        const min::ogg sound8(ogg8);
        const min::mem_file &ogg9 = memory_map::memory.get_file("data/sound/voice_thrust_alert_s.ogg");
        const min::ogg sound9(ogg9);

        // Create music buffers
        _voice[0] = _buffer.add_ogg_pcm(sound1);
        _voice[1] = _buffer.add_ogg_pcm(sound2);
        _voice[2] = _buffer.add_ogg_pcm(sound3);
        _voice[3] = _buffer.add_ogg_pcm(sound4);
        _voice[4] = _buffer.add_ogg_pcm(sound5);
        _voice[5] = _buffer.add_ogg_pcm(sound6);
        _voice[6] = _buffer.add_ogg_pcm(sound7);
        _voice[7] = _buffer.add_ogg_pcm(sound8);
        _voice[8] = _buffer.add_ogg_pcm(sound9);

        // Load the first sound into the sound buffer
        load_sound(_voice[0], _voice_gain, _fade_speed);
    }
    inline void reserve_memory()
    {
        _si.reserve(_sounds);
        _v_queue.reserve(_voice_sounds);
    }

  public:
    sound()
        : _music(_bg_sounds), _bg_delay(30.0), _bg_enable(false),
          _drone(_drone_limit), _drone_old(0),
          _ex(_ex_limit), _ex_old(0),
          _miss_launch(_miss_launch_limit), _miss_launch_old(0),
          _voice(_voice_sounds), _v_head(0), _v_delay(1.0), _v_enable(true),
          _int_dist(0, _bg_sounds - 1), _real_dist(0.0, _max_delay),
          _gen(std::chrono::high_resolution_clock::now().time_since_epoch().count())
    {
        // Reserve memory for sources and buffers
        reserve_memory();

        // Load background music
        load_bg_sound();

        // Load explode sound into buffer
        load_blast_mono_sound();

        // Load explode sound into buffer
        load_blast_stereo_sound();

        // Load charge sound into buffer
        load_charge_sound();

        // Load click sound into buffer
        load_click_sound();

        // Load focus sound into buffer
        load_focus_sound();

        // Load grapple sound into buffer
        load_grapple_sound();

        // Load jet sound into buffer
        load_jet_sound();

        // Load land sound into buffer
        load_land_sound();

        // Load oxygen sound
        load_oxygen_sound();

        // Load pickup sound into buffer
        load_pickup_sound();

        // Load shot sound into buffer
        load_shot_sound();

        // Load shot explode sound into buffer
        load_shot_ex_sound();

        // Load thrust sound into buffer
        load_thrust_sound();

        // Load voice sounds into buffer
        load_voice_sound();

        // Load zap sound into buffer
        load_zap_sound();

        // Load drone sound into buffer
        load_drone_sound();

        // Load missile explode sound into buffer
        load_explode_sound();

        // Load missile launch sound into buffer
        load_miss_launch_sound();

        // Set the buffer distance model
        _buffer.set_distance_model(AL_INVERSE_DISTANCE_CLAMPED);
    }
    inline void reset()
    {
        // Stop all sounds
        for (sound_info &si : _si)
        {
            _buffer.stop_async(si.source());
            si.set_fade_in(false);
            si.set_fade_out(false);
            si.set_play(false);
        }

        // Reset oldest positions
        _drone_old = 0;
        _ex_old = 0;
        _miss_launch_old = 0;

        // Reset the voice queue
        _v_queue.clear();
        _v_head = 0;
        _v_delay = 1.0;
        _v_enable = true;
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
    inline bool check_error() const
    {
        return _buffer.check_error();
    }
    inline size_t get_idle_drone_id()
    {
        // Output id
        size_t id = 0;

        // Scan for unused particle system
        for (size_t i = 0; i < _drone_limit; i++)
        {
            // Start at the oldest index
            const size_t index = (_drone_old %= _drone_limit)++;

            // If index is unused
            if (!drone_info(index).playing())
            {
                // Assign id for use
                id = index;

                // Break out since found
                break;
            }
        }

        return id;
    }
    inline size_t get_idle_miss_launch_id()
    {
        // Output id
        size_t id = 0;

        // Scan for unused particle system
        for (size_t i = 0; i < _miss_launch_limit; i++)
        {
            // Start at the oldest index
            const size_t index = (_miss_launch_old %= _miss_launch_limit)++;

            // If index is unused
            if (!miss_launch_info(index).playing())
            {
                // Assign id for use
                id = index;

                // Break out since found
                break;
            }
        }

        return id;
    }
    inline void play_bg(const bool flag)
    {
        _bg_enable = flag;
    }
    inline void play_voice(const bool flag)
    {
        _v_enable = flag;
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
    inline void play_drone(const size_t index, const min::vec3<float> &p)
    {
        sound_info &si = drone_info(index);

        // Set the fade and play flags
        si.set_fade_out(false);
        si.set_gain(_drone_gain);
        si.set_play(true);

        // Get the sound source
        const size_t s = si.source();

        // Set the sound position and gain
        _buffer.set_source_position(s, p);
        _buffer.set_source_gain(s, si.gain());

        // Play the sound asynchronously at full gain
        _buffer.play_async(s);
    }
    inline void stop_drone(const size_t index)
    {
        sound_info &si = drone_info(index);

        // Turn on fading
        si.set_fade_out(true);
    }
    inline void update_drone(const size_t index, const min::vec3<float> &p)
    {
        sound_info &si = drone_info(index);

        // Set the sound position
        _buffer.set_source_position(si.source(), p);
    }
    inline void play_blast_mono(const min::vec3<float> &p)
    {
        // Get the explode source
        const size_t s = blast_m_info().source();

        // Set the sound position
        _buffer.set_source_position(s, p);

        // Play the sound
        _buffer.play_async(s);
    }
    inline void play_blast_stereo(const min::vec3<float> &p)
    {
        // Get the explode source
        const size_t s = blast_s_info().source();

        // Set the sound position
        _buffer.set_source_position(s, p);

        // Play the sound
        _buffer.play_async(s);
    }
    inline void play_focus()
    {
        _buffer.play_async(focus_info().source());
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
    inline void play_explode(const min::vec3<float> &p)
    {
        // Increment next explode source
        const size_t index = (_ex_old %= _ex_limit)++;

        // Get the explode source
        const size_t s = ex_info(index).source();

        // Set the sound position
        _buffer.set_source_position(s, p);

        // Play the sound
        _buffer.play_async(s);
    }
    inline void play_miss_launch(const size_t index, const min::vec3<float> &p)
    {
        sound_info &si = miss_launch_info(index);

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
    inline void stop_miss_launch(const size_t index)
    {
        sound_info &si = miss_launch_info(index);

        // Turn on fading
        si.set_fade_out(true);
    }
    inline void update_miss_launch(const size_t index, const min::vec3<float> &p)
    {
        sound_info &si = miss_launch_info(index);

        // Set the sound position
        _buffer.set_source_position(si.source(), p);
    }
    inline void play_oxygen()
    {
        sound_info &si = oxygen_info();

        // Set the fade and play flags
        si.set_fade_in(true);
        si.set_fade_out(false);
        si.set_gain(_fade_in);
        si.set_play(true);

        // Play the sound asynchronously at full gain
        _buffer.set_source_gain(si.source(), si.gain());
        _buffer.play_async(si.source());
    }
    inline void stop_oxygen()
    {
        sound_info &si = oxygen_info();

        // Turn on fading
        si.set_fade_in(false);
        si.set_fade_out(true);
    }
    inline void play_pickup()
    {
        _buffer.play_async(pickup_info().source());
    }
    inline void play_shot()
    {
        _buffer.play_async(shot_info().source());
    }
    inline void play_shot_ex(const min::vec3<float> &p)
    {
        // Get the shot explode source
        const size_t s = shot_ex_info().source();

        // Set the sound position
        _buffer.set_source_position(s, p);

        // Play the sound
        _buffer.play_async(s);
    }
    inline void play_thrust()
    {
        _buffer.play_async(thrust_info().source());
    }
    inline void play_voice_comply()
    {
        _v_queue.push_back(v_comply());

        // Remove duplicates
        _v_queue.erase(std::unique(_v_queue.begin() + _v_head, _v_queue.end()), _v_queue.end());
    }
    inline void play_voice_critical()
    {
        _v_queue.push_back(v_critical());

        // Remove duplicates
        _v_queue.erase(std::unique(_v_queue.begin() + _v_head, _v_queue.end()), _v_queue.end());
    }
    inline void play_voice_level()
    {
        _v_queue.push_back(v_level());

        // Remove duplicates
        _v_queue.erase(std::unique(_v_queue.begin() + _v_head, _v_queue.end()), _v_queue.end());
    }
    inline void play_voice_portal_alert()
    {
        _v_queue.push_back(v_portal_alert());

        // Remove duplicates
        _v_queue.erase(std::unique(_v_queue.begin() + _v_head, _v_queue.end()), _v_queue.end());
    }
    inline void play_voice_power()
    {
        _v_queue.push_back(v_power());

        // Remove duplicates
        _v_queue.erase(std::unique(_v_queue.begin() + _v_head, _v_queue.end()), _v_queue.end());
    }
    inline void play_voice_repair()
    {
        _v_queue.push_back(v_repair());

        // Remove duplicates
        _v_queue.erase(std::unique(_v_queue.begin() + _v_head, _v_queue.end()), _v_queue.end());
    }
    inline void play_voice_low_power()
    {
        _v_queue.push_back(v_resource());

        // Remove duplicates
        _v_queue.erase(std::unique(_v_queue.begin() + _v_head, _v_queue.end()), _v_queue.end());
    }
    inline void play_voice_shutdown()
    {
        _v_queue.push_back(v_shutdown());

        // Remove duplicates
        _v_queue.erase(std::unique(_v_queue.begin() + _v_head, _v_queue.end()), _v_queue.end());
    }
    inline void play_voice_thrust_alert()
    {
        _v_queue.push_back(v_thrust_alert());

        // Remove duplicates
        _v_queue.erase(std::unique(_v_queue.begin() + _v_head, _v_queue.end()), _v_queue.end());
    }
    inline void play_zap()
    {
        _buffer.play_async(zap_info().source());
    }
    inline void reset_voice_queue()
    {
        _v_queue.clear();
        _v_head = 0;
    }
    inline void update(const min::camera<float> &cam, const min::vec3<float> &vel, const float dt)
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
        // CLICK == JET == LAND == PICKUP == SHOT == STEREO

        // If music is not disabled
        if (_bg_enable)
        {
            // If we are not playing any music
            sound_info &bg_si = bg_info();
            if (!bg_si.playing())
            {
                // Decrement delay timer
                _bg_delay -= dt;

                // If we are done waiting
                if (_bg_delay < 0.0)
                {
                    // Get a random music index
                    const size_t ran_index = _int_dist(_gen);

                    // Bind source to random music data
                    _buffer.bind(_music[ran_index], bg_info().source());

                    // Set the sound is playing
                    bg_si.set_fade_in(true);
                    bg_si.set_gain(_fade_in);
                    bg_si.set_play(true);

                    // Play the background music
                    _buffer.play_async(bg_si.source());

                    // Calculate a random delay
                    _bg_delay = _real_dist(_gen);
                }
            }
        }

        // Update voices
        if (_v_enable)
        {
            // If we are not playing any voices
            sound_info &v_si = voice_info();
            const size_t size = _v_queue.size();
            if (!v_si.playing() && size > 0)
            {
                // Decrement delay timer
                _v_delay -= dt;

                if (_v_delay < 0.0)
                {
                    // Flag that we are playing
                    v_si.set_play(true);

                    // Bind source to next voice track
                    _buffer.bind(_voice[_v_queue[_v_head]], v_si.source());

                    // Increment head
                    _v_head++;

                    // Reset queue buffer
                    if (_v_head == size)
                    {
                        reset_voice_queue();
                    }

                    // Play the voice sound music
                    _buffer.play_async(v_si.source());

                    // Calculate next delay
                    _v_delay = 1.0;
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
