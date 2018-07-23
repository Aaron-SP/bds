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
#ifndef _BDS_STATS_BDS_
#define _BDS_STATS_BDS_

#include <array>
#include <cmath>
#include <cstdint>
#include <game/def.h>
#include <game/item.h>
#include <string>

namespace game
{

enum class stat_alert
{
    none,
    level,
    dynamics
};

class stats
{
  private:
    static constexpr float _health_consume = 0.5;
    static constexpr float _energy_consume = 1.0;
    static constexpr float _oxygen_consume = 0.001;
    static constexpr float _health_regen = 2.5;
    static constexpr float _energy_regen = 5.0;
    static constexpr size_t _max_attr = 10;
    static constexpr size_t _max_attr_str = _max_attr - 2;
    static constexpr size_t _max_stats = 7;
    static constexpr size_t _max_level = 50;
    static constexpr float _per_second = 1.0 / _physics_frames;

    // Costs
    static constexpr float _beam_cost = 5.0;
    static constexpr float _charge_cost = 10.0;
    static constexpr float _grenade_cost = 10.0;
    static constexpr float _jet_cost = 0.05;
    static constexpr float _missile_cost = 10.0;
    static constexpr float _portal_cost = 0.5;
    static constexpr float _scatter_cost = 20.0;

    float _energy;
    bool _low_energy;
    float _max_exp;
    float _exp;
    float _health;
    bool _low_health;
    float _max_oxygen;
    float _oxygen;
    bool _low_oxygen;
    float _crit;
    float _gave_dmg;
    float _took_dmg;
    bool _dead;
    bool _dirty;
    stat_alert _alert;
    std::array<float, _max_attr> _attr;
    std::array<uint_fast16_t, _max_stats> _stat;
    uint_fast16_t _stat_points;
    item _equipped;
    float _sqrt_level;

    inline float calc_state_scale(const float A0, const float k, const float x) const
    {
        return A0 * (1.0 - std::exp(-k * x * x));
    }
    inline float calc_damage_mult() const
    {
        constexpr float A = 20.0;
        constexpr float B = 1.0;
        constexpr float C = 75000.0;
        const float x = power() + _equipped.primary();
        const float k = B / C;
        return 1.0 + calc_state_scale(A, k, x);
    }
    inline float calc_dynamics_consume() const
    {
        constexpr float A = 1.9;
        constexpr float B = 1.0;
        constexpr float C = 1250.0;
        const float x = dynamism() + _equipped.secondary();
        const float k = B / C;
        return 2.0 - calc_state_scale(A, k, x);
    }
    inline float calc_damage_reduc() const
    {
        constexpr float A = 0.95;
        constexpr float B = 1.0;
        constexpr float C = 5000.0;
        const float x = tenacity() + _equipped.primary();
        const float k = B / C;
        return calc_state_scale(A, k, x);
    }
    inline float calc_cooldown_reduc() const
    {
        constexpr float A = 0.9;
        constexpr float B = 1.0;
        constexpr float C = 2500.0;
        const float x = cooldown() + _equipped.secondary();
        const float k = B / C;
        return calc_state_scale(A, k, x);
    }
    inline float calc_health_regen() const
    {
        constexpr float A = 40.0 * _per_second;
        constexpr float B = 1.0;
        constexpr float C = 2500.0;
        const float x = regen() + _equipped.secondary();
        const float k = B / C;
        return 0.01 + calc_state_scale(A, k, x);
    }
    inline float calc_energy_regen() const
    {
        constexpr float A = 20.0 * _per_second;
        constexpr float B = 1.0;
        constexpr float C = 2500.0;
        const float x = regen() + _equipped.secondary();
        const float k = B / C;
        return 0.01 + calc_state_scale(A, k, x);
    }
    inline float calc_health_consume() const
    {
        return (_health_consume / _sqrt_level) * _per_second;
    }
    inline float calc_energy_consume() const
    {
        return (_energy_consume / _sqrt_level) * _per_second;
    }
    inline float calc_max_health() const
    {
        return std::log10(tenacity()) * (_sqrt_level * 100.0);
    }
    inline float calc_max_energy() const
    {
        return std::log10(cooldown()) * (_sqrt_level * 33.333);
    }
    inline float calc_max_exp() const
    {
        return std::floor(600.0 * std::exp(_sqrt_level * 1.2));
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
    inline float get_damage_mult() const
    {
        return _attr[0];
    }
    inline float get_dynamics_cost_frac() const
    {
        return _attr[1];
    }
    inline float get_damage_reduc() const
    {
        return _attr[2];
    }
    inline float get_cooldown_reduc() const
    {
        return _attr[3];
    }
    inline float get_health_regen() const
    {
        return _attr[4];
    }
    inline float get_energy_regen() const
    {
        return _attr[5];
    }
    inline float get_health_consume() const
    {
        return _attr[8];
    }
    inline float get_energy_consume() const
    {
        return _attr[9];
    }
    inline float get_beam_cost() const
    {
        return get_damage_mult() * _beam_cost;
    }
    inline float get_charge_cost() const
    {
        return get_damage_mult() * _charge_cost;
    }
    inline float get_grapple_cost() const
    {
        return get_max_energy() * get_dynamics_cost_frac();
    }
    inline float get_grenade_cost() const
    {
        return get_damage_mult() * _grenade_cost;
    }
    inline float get_jet_cost() const
    {
        // Cost per frame
        const float dynamics = get_dynamics_cost_frac();
        if (dynamics < 1.0)
        {
            return _jet_cost * dynamics;
        }

        // Needs to unlock dynamics before using
        return get_max_energy() * dynamics;
    }
    inline float get_missile_cost() const
    {
        return get_damage_mult() * _missile_cost;
    }
    inline float get_portal_cost() const
    {
        return get_max_energy() * _portal_cost;
    }
    inline float get_scatter_cost() const
    {
        return get_damage_mult() * _scatter_cost;
    }
    inline float get_dynamics_cost() const
    {
        return get_max_energy() * get_dynamics_cost_frac();
    }
    inline void set_energy(const float energy)
    {
        // Check above warning threshold
        const bool above = _energy >= 25.0;

        // Set energy
        _energy = energy;

        // Check for low energy
        if (above && is_low_energy())
        {
            _low_energy = true;
        }
    }
    inline void set_health(const float health)
    {
        // Check above warning threshold
        const bool above = _health >= 25.0;

        // Set health
        _health = health;

        // Check for low health
        if (_health <= 0.0)
        {
            _dead = true;
        }
        else if (above && is_low_health())
        {
            _low_health = true;
        }
    }
    inline void set_oxygen(const float oxygen)
    {
        // Check above warning threshold
        const bool above = _oxygen >= 25.0;

        // Set oxygen
        _oxygen = oxygen;

        // Check for low oxygen
        if (_oxygen <= 0.0)
        {
            _dead = true;
        }
        else if (above && is_low_oxygen())
        {
            _low_oxygen = true;
        }
    }
    inline void update_cache()
    {
        // Are dynamics offline?
        const bool dynamics_above = get_dynamics_cost_frac() > 1.0;

        // Cache the sqrt of level
        _sqrt_level = std::sqrt(level());

        // Update attributes
        _attr[0] = calc_damage_mult();
        _attr[1] = calc_dynamics_consume();
        _attr[2] = calc_damage_reduc();
        _attr[3] = calc_cooldown_reduc();
        _attr[4] = calc_health_regen();
        _attr[5] = calc_energy_regen();
        _attr[6] = calc_max_health();
        _attr[7] = calc_max_energy();
        _attr[8] = calc_health_consume();
        _attr[9] = calc_energy_consume();

        // Update max experience
        _max_exp = calc_max_exp();

        // Set dynamics alert if enabled
        if (dynamics_above && get_dynamics_cost_frac() <= 1.0)
        {
            _alert = stat_alert::dynamics;
        }
    }

  public:
    static std::array<std::string, _max_attr_str> _attr_str;
    static std::array<std::string, _max_stats> _stat_str;
    stats()
        : _energy(10.0), _low_energy(false),
          _max_exp(100.0), _exp(0.0),
          _health(70.0), _low_health(false),
          _max_oxygen(100.0), _oxygen(_max_oxygen), _low_oxygen(false),
          _crit(0.0), _gave_dmg(0.0), _took_dmg(0.0), _dead(false), _dirty(false), _alert(stat_alert::none),
          _attr{}, _stat{4, 3, 5, 2, 0, 3, 1}, _stat_points(0), _sqrt_level(1.0)
    {
        // Update the stat cache
        update_cache();
    }

    inline void add_energy(const float energy)
    {
        _energy += energy;
    }
    inline void add_exp(const float exp)
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
    inline void add_oxygen(const float oxy)
    {
        _oxygen += oxy;

        // If oxygen overflow
        if (_oxygen > _max_oxygen)
        {
            _oxygen = _max_oxygen;
        }
    }
    inline static constexpr size_t attr_str_size()
    {
        return _max_attr_str;
    }
    inline static const std::string &attr_str(const size_t index)
    {
        return _attr_str[index];
    }
    inline float attr_value(const size_t index)
    {
        // Formatting attr for display
        switch (index)
        {
        case 0:
            return _attr[index] * 100.0;
        case 1:
            return _attr[index] * 100.0;
        case 2:
            return _attr[index] * 100.0;
        case 3:
            return _attr[index] * 100.0;
        case 4:
            return _attr[index] * _physics_frames;
        case 5:
            return _attr[index] * _physics_frames;
        default:
            return _attr[index];
        }
    }
    inline void clean()
    {
        _dirty = false;
    }
    inline void clear_alert()
    {
        _alert = stat_alert::none;
    }
    inline void clear_crit()
    {
        _crit = _gave_dmg = 0.0;
    }
    inline void clear_gave_dmg()
    {
        _gave_dmg = 0.0;
    }
    inline void clear_took_dmg()
    {
        _took_dmg = 0.0;
    }
    inline void clear_low_energy_flag()
    {
        _low_energy = false;
    }
    inline void clear_low_health_flag()
    {
        _low_health = false;
    }
    inline void clear_low_oxygen_flag()
    {
        _low_oxygen = false;
    }
    inline bool can_consume_beam() const
    {
        return can_consume_energy(get_beam_cost());
    }
    inline bool can_consume_charge() const
    {
        return can_consume_energy(get_charge_cost());
    }
    inline bool can_consume_grapple() const
    {
        return can_consume_energy(get_grapple_cost());
    }
    inline bool can_consume_grenade() const
    {
        return can_consume_energy(get_grenade_cost());
    }
    inline bool can_consume_jet() const
    {
        return can_consume_energy(get_jet_cost());
    }
    inline bool can_consume_missile() const
    {
        return can_consume_energy(get_missile_cost());
    }
    inline bool can_consume_portal() const
    {
        return can_consume_energy(get_portal_cost());
    }
    inline bool can_consume_scatter() const
    {
        return can_consume_energy(get_scatter_cost());
    }
    inline bool can_consume_dynamics() const
    {
        return can_consume_energy(get_dynamics_cost());
    }
    inline void consume_beam()
    {
        consume_energy(get_beam_cost());
    }
    inline void consume_charge()
    {
        consume_energy(get_charge_cost());
    }
    inline void consume_grapple()
    {
        consume_energy(get_grapple_cost());
    }
    inline void consume_grenade()
    {
        consume_energy(get_grenade_cost());
    }
    inline void consume_jet()
    {
        consume_energy(get_jet_cost());
    }
    inline void consume_missile()
    {
        consume_energy(get_missile_cost());
    }
    inline void consume_portal()
    {
        consume_energy(get_portal_cost());
    }
    inline void consume_scatter()
    {
        consume_energy(get_scatter_cost());
    }
    inline void consume_dynamics()
    {
        consume_energy(get_dynamics_cost());
    }
    inline void consume_energy(const float energy)
    {
        set_energy(_energy - energy);
    }
    inline void consume_health(const float dmg)
    {
        // Calculate damage
        _took_dmg += dmg;

        set_health(_health - dmg);
    }
    inline void consume_oxygen()
    {
        // Consume oxygen
        set_oxygen(_oxygen - _oxygen_consume);

        // Take damage if less than 10% max oxygen
        const float threshold = _max_oxygen * 0.10;
        if (_oxygen <= threshold)
        {
            // Take damage per second because of low oxygen
            const float dmg = get_health_regen() * 1.5;
            set_health(_health - dmg);
        }
    }
    inline void damage(const float in)
    {
        // Calculate damage reduction
        const float reduc = get_damage_reduc();

        // Apply damage reduction
        consume_health((1.0 - reduc) * in);
    }
    inline float do_damage(const float dmg, const float crit_mult)
    {
        // Set crit and damage properties
        _crit = crit_mult;
        _gave_dmg = get_damage_mult() * dmg * crit_mult;

        // Return damage dealt
        return _gave_dmg;
    }
    inline void equip_item(const item it)
    {
        if (it.type() == item_type::skill)
        {
            _equipped = it;
        }
        else
        {
            _equipped = item();
        }

        // Update the stat cache
        update_cache();

        // Set dirty flag
        _dirty = true;
    }
    inline void fill(const std::array<uint_fast16_t, _max_stats> &stat, const float energy, const float exp, const float health, const float oxygen, const uint_fast16_t stats)
    {
        // Copy stats into stat array
        for (size_t i = 0; i < _max_stats; i++)
        {
            _stat[i] = stat[i];
        }

        // Update the stat cache
        update_cache();

        // Set the energy
        set_energy(energy);

        // Set the exp
        add_exp(exp);

        // Set the health
        set_health(health);

        // Set the oxygen
        set_oxygen(oxygen);

        // Set the stat points
        _stat_points = stats;
    }
    inline stat_alert get_alert() const
    {
        return _alert;
    }
    inline float get_cooldown_mult() const
    {
        return 1.0 - get_cooldown_reduc();
    }
    inline float get_gave_dmg() const
    {
        return _gave_dmg;
    }
    inline float get_drone_health() const
    {
        return 100.0 * _sqrt_level;
    }
    inline float get_drop_exp() const
    {
        return 25.0;
    }
    inline float get_energy() const
    {
        return _energy;
    }
    inline float get_energy_fraction() const
    {
        return _energy / get_max_energy();
    }
    inline float get_exp() const
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
        return _health / get_max_health();
    }
    inline float get_took_dmg() const
    {
        return _took_dmg;
    }
    inline float get_max_energy() const
    {
        return _attr[7];
    }
    inline float get_max_health() const
    {
        return _attr[6];
    }
    inline float get_mob_exp() const
    {
        return _max_exp / level();
    }
    inline float get_oxygen() const
    {
        return _oxygen;
    }
    inline float get_oxygen_fraction() const
    {
        return _oxygen / _max_oxygen;
    }
    inline uint_fast16_t get_stat_points() const
    {
        return _stat_points;
    }
    inline bool has_stat_points() const
    {
        return _stat_points > 0;
    }
    inline bool is_crit() const
    {
        return _crit > 1.5;
    }
    inline bool is_dead() const
    {
        return _dead;
    }
    inline bool is_dirty() const
    {
        return _dirty;
    }
    inline bool is_gave_dmg() const
    {
        return (_gave_dmg > 0.0);
    }
    inline bool is_took_dmg() const
    {
        return (_took_dmg > 0.0);
    }
    inline bool is_dynamics_online() const
    {
        return get_dynamics_cost_frac() < 1.0;
    }
    inline bool is_level_up() const
    {
        return _dirty;
    }
    inline bool is_low_energy() const
    {
        return _energy < get_max_energy() * 0.25;
    }
    inline bool is_low_energy_flag() const
    {
        return _low_energy;
    }
    inline bool is_low_health() const
    {
        return _health < get_max_health() * 0.25;
    }
    inline bool is_low_health_flag() const
    {
        return _low_health;
    }
    inline bool is_low_oxygen() const
    {
        const float threshold = _max_oxygen * 0.25;
        return _oxygen < threshold;
    }
    inline bool is_low_oxygen_flag() const
    {
        return _low_oxygen;
    }
    inline void regen_energy()
    {
        // Absorb this amount of energy if not full
        const float max_energy = get_max_energy();
        if (_energy < max_energy)
        {
            // Calculate energy change
            const float energy = get_energy_regen();
            _energy += energy;

            // Cap energy at max_energy
            if (_energy > max_energy)
            {
                _energy = max_energy;
            }
        }
        // Consume energy if over full
        else if (_energy > max_energy)
        {
            // Calculate energy change
            const float energy = get_energy_consume();
            _energy -= energy;

            // Cap energy at max_energy
            if (_energy < max_energy)
            {
                _energy = max_energy;
            }
        }
    }
    inline void regen_health()
    {
        // Absorb this amount of health if not full
        const float max_health = get_max_health();
        if (_health < max_health)
        {
            // Calculate health change
            const float health = get_health_regen();
            _health += health;

            // Cap health at max_health
            if (_health > max_health)
            {
                _health = max_health;
            }
        }
        // Consume health if over full
        else if (_health > max_health)
        {
            // Calculate health change
            const float health = get_health_consume();
            _health -= health;

            // Cap health at max_health
            if (_health < max_health)
            {
                _health = max_health;
            }
        }
    }
    inline void respawn()
    {
        // Reset energy
        _energy = get_max_energy();
        _low_energy = false;

        // Reset experience
        _exp = 0.0;

        // Reset health
        _health = get_max_health();
        _low_health = false;

        // Reset health
        _oxygen = _max_oxygen;
        _low_oxygen = false;
        _crit = 0.0;
        _gave_dmg = 0.0;
        _took_dmg = 0.0;
        _dead = false;
    }
    inline void set_point(const size_t index)
    {
        if (_stat_points > 0)
        {
            // Subtract stat point
            _stat_points--;

            // Allocate stat point
            _stat[index]++;

            // Update the stat cache
            update_cache();

            // Set dirty flag
            _dirty = true;
        }
    }
    inline static constexpr size_t stat_str_size()
    {
        return _max_stats;
    }
    inline static const std::string &stat_str(const size_t index)
    {
        return _stat_str[index];
    }
    inline uint_fast16_t stat_value(const size_t index) const
    {
        return _stat[index];
    }
    inline uint_fast16_t power() const
    {
        return _stat[0];
    }
    inline uint_fast16_t dynamism() const
    {
        return _stat[1];
    }
    inline uint_fast16_t tenacity() const
    {
        return _stat[2];
    }
    inline uint_fast16_t cooldown() const
    {
        return _stat[3];
    }
    inline uint_fast16_t vision() const
    {
        return _stat[4];
    }
    inline uint_fast16_t regen() const
    {
        return _stat[5];
    }
    inline uint_fast16_t level() const
    {
        return _stat[6];
    }
    inline void level_up()
    {
        // Don't level past the cap
        if (level() < _max_level)
        {
            // Update stats
            _stat[6]++;

            // Add stat points
            _stat_points += 5;

            // Update the stat cache
            update_cache();

            // Set level alert
            _alert = stat_alert::level;

            // Set max health and energy
            _health = get_max_health();
            _energy = get_max_energy();

            // Set dirty flag
            _dirty = true;
        }
    }
};

// Initialize public static string stats
std::array<std::string, stats::attr_str_size()> stats::_attr_str = {"Damage Boost (%)", "Dynamics Cost (%)", "Damage Reduction (%)", "Cooldown Reduction (%)", "Health Regen (/s)", "Energy Regen (/s)", "Max Health", "Max Energy"};
std::array<std::string, stats::stat_str_size()> stats::_stat_str = {"Power", "Dynamism", "Tenacity", "Tranquility", "Vision", "Zeal", "Level"};
}

#endif
