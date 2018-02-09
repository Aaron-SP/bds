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
    static constexpr float _health_regen = 1.0 / 180.0;
    static constexpr float _energy_regen = 2.0 / 180.0;
    static constexpr size_t _max_stats = 6;
    float _max_energy;
    float _energy;
    bool _low_energy;
    float _max_health;
    float _health;
    bool _low_health;
    bool _dead;
    std::array<uint16_t, _max_stats> _stats;

  public:
    static std::array<std::string, _max_stats> stat_str;
    stats()
        : _max_energy(100.0), _energy(_max_energy), _low_energy(false),
          _max_health(100.0), _health(_max_health), _low_health(false), _dead(false),
          _stats{5, 5, 5, 5, 5, 5} {}

    inline void add_energy(const float energy)
    {
        // Absorb this amount of energy if not full
        if (_energy < _max_energy)
        {
            _energy += energy;

            // Cap energy at max_energy
            if (_energy > _max_energy)
            {
                _energy = _max_energy;
            }
        }
    }
    inline void add_health(const float health)
    {
        if (_health < _max_health)
        {
            _health += health;

            // Cap health at health_cap;
            if (_health > _max_health)
            {
                _health = _max_health;
            }
        }
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
    inline float get_energy() const
    {
        return _energy;
    }
    inline float get_energy_percent() const
    {
        return _energy / _max_energy;
    }
    inline float get_health() const
    {
        return _health;
    }
    inline float get_health_percent() const
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
        add_energy(_energy_regen * regen());
    }
    void regen_health()
    {
        add_health(_health_regen * regen());
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
};

// Initialize public static string stats
std::array<std::string, stats::str_size()> stats::stat_str = {"Force", "Dynamism", "Tenacity", "Tranquility", "Vision", "Zeal"};
}

#endif
