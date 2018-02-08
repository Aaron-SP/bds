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

namespace game
{

class stats
{
  private:
    static constexpr float _health_regen = 1.0;
    static constexpr float _energy_regen = 2.0;
    float _cd;
    float _max_energy;
    float _energy;
    bool _low_energy;
    float _max_health;
    float _health;
    bool _low_health;
    bool _dead;
    float _power;
    float _range;
    float _regen;
    float _speed;
    float _vital;

  public:
    stats()
        : _cd(5.0),
          _max_energy(100.0), _energy(_max_energy), _low_energy(false),
          _max_health(100.0), _health(_max_health), _low_health(false), _dead(false),
          _power(5.0), _range(5.0), _regen(5.0),
          _speed(5.0), _vital(5.0) {}

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
    float cooldown() const
    {
        return _power;
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
    float power() const
    {
        return _power;
    }
    float range() const
    {
        return _range;
    }
    float regen() const
    {
        return _regen;
    }
    void regen_energy(const float dt)
    {
        add_energy(_energy_regen * _regen * dt);
    }
    void regen_health(const float dt)
    {
        add_health(_health_regen * _regen * dt);
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
    float speed() const
    {
        return _speed;
    }
    float vital() const
    {
        return _vital;
    }
};
}

#endif
