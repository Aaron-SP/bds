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
#ifndef __STATS__
#define __STATS__

#include <array>
#include <string>

namespace game
{

class stats
{
  private:
    static constexpr float _health_consume = 0.1 / 180.0;
    static constexpr float _energy_consume = 0.1 / 180.0;
    static constexpr float _health_regen = 1.0 / 180.0;
    static constexpr float _energy_regen = 2.0 / 180.0;
    static constexpr size_t _max_stats = 7;
    float _max_energy;
    float _energy;
    bool _low_energy;
    float _max_exp;
    float _exp;
    float _max_health;
    float _health;
    bool _low_health;
    bool _dead;
    bool _dirty;
    std::array<uint16_t, _max_stats> _stats;

  public:
    static std::array<std::string, _max_stats> stat_str;
    stats()
        : _max_energy(100.0), _energy(_max_energy), _low_energy(false), _max_exp(1000.0), _exp(0.0),
          _max_health(100.0), _health(_max_health), _low_health(false), _dead(false), _dirty(false),
          _stats{5, 5, 5, 5, 5, 5, 1} {}

    inline void add_energy(const float energy)
    {
        _energy += energy;
    }
    inline void add_experience(const float exp)
    {
        _exp += exp;

        // If experience overflow
        if (_exp >= _max_exp)
        {
            _exp -= _max_exp;

            // Level up
            level_up();
        }
    }
    inline void add_health(const float health)
    {
        _health += health;
    }
    inline void consume_health(const float health)
    {
        // Check above warning threshold
        const bool above = _health >= 25.0;

        _health -= health;
        if (_health <= 0.0)
        {
            _dead = true;
        }
        else if (above && _health < 25.0)
        {
            _low_health = true;
        }
    }
    inline bool can_consume_energy(const float energy) const
    {
        // Try to consume energy
        if (_energy >= energy)
        {
            return true;
        }

        // Not enough energy
        return false;
    }
    inline void consume_energy(const float energy)
    {
        // Check above warning threshold
        const bool above = _energy >= 25.0;

        // Consume energy
        _energy -= energy;

        // Check for low energy
        if (above && _energy < 25.0)
        {
            _low_energy = true;
        }
    }
    inline void clean()
    {
        _dirty = false;
    }
    inline bool dirty() const
    {
        return _dirty;
    }
    inline float get_energy() const
    {
        return _energy;
    }
    inline float get_energy_fraction() const
    {
        return _energy / _max_energy;
    }
    inline float get_experience() const
    {
        return _exp;
    }
    inline float get_experience_fraction() const
    {
        return _exp / _max_exp;
    }
    inline float get_health() const
    {
        return _health;
    }
    inline float get_health_fraction() const
    {
        return _health / _max_health;
    }
    inline bool is_dead() const
    {
        return _dead;
    }
    inline bool is_low_energy() const
    {
        return _low_energy;
    }
    inline bool is_low_health() const
    {
        return _low_health;
    }
    void regen_energy()
    {
        // Absorb this amount of energy if not full
        if (_energy < _max_energy)
        {
            // Calculate energy change
            const float energy = _energy_regen * regen();
            _energy += energy;

            // Cap energy at max_energy
            if (_energy > _max_energy)
            {
                _energy = _max_energy;
            }
        }
        // Consume energy if over full
        else if (_energy > _max_energy)
        {
            // Calculate energy change
            const float energy = _energy_consume * regen();
            _energy -= energy;

            // Cap energy at max_energy
            if (_energy < _max_energy)
            {
                _energy = _max_energy;
            }
        }
    }
    void regen_health()
    {
        // Absorb this amount of health if not full
        if (_health < _max_health)
        {
            // Calculate health change
            const float health = _health_regen * regen();
            _health += health;

            // Cap health at max_health
            if (_health > _max_health)
            {
                _health = _max_health;
            }
        }
        // Consume health if over full
        else if (_health > _max_health)
        {
            // Calculate health change
            const float health = _health_consume * regen();
            _health -= health;

            // Cap health at max_health
            if (_health < _max_health)
            {
                _health = _max_health;
            }
        }
    }
    inline void reset_low_energy()
    {
        _low_energy = false;
    }
    inline void reset_low_health()
    {
        _low_health = false;
    }
    inline void respawn()
    {
        // Reset energy
        _energy = _max_energy;
        _low_energy = false;

        // Reset experience
        _exp = 0.0;

        // Reset health
        _health = _max_health;
        _low_health = false;
        _dead = false;
    }
    static constexpr size_t str_size()
    {
        return _max_stats;
    }
    static const std::string &str(const size_t index)
    {
        return stat_str[index];
    }
    uint16_t value(const size_t index)
    {
        return _stats[index];
    }
    uint16_t power() const
    {
        return _stats[0];
    }
    uint16_t speed() const
    {
        return _stats[1];
    }
    uint16_t vital() const
    {
        return _stats[2];
    }
    uint16_t cooldown() const
    {
        return _stats[3];
    }
    uint16_t range() const
    {
        return _stats[4];
    }
    uint16_t regen() const
    {
        return _stats[5];
    }
    uint16_t level() const
    {
        return _stats[6];
    }
    void level_up()
    {
        _stats[6]++;

        // Set dirty flag
        _dirty = true;
    }
};

// Initialize public static string stats
std::array<std::string, stats::str_size()> stats::stat_str = {"Force", "Dynamism", "Tenacity", "Tranquility", "Vision", "Zeal", "Level"};
}

#endif
