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
#ifndef __LOAD_STATE__
#define __LOAD_STATE__

#include <cstdint>
#include <game/file.h>
#include <game/id.h>
#include <game/inventory.h>
#include <game/options.h>
#include <game/static_instance.h>
#include <game/stats.h>
#include <iostream>
#include <limits>
#include <min/vec3.h>
#include <stdexcept>
#include <vector>

namespace game
{

struct game_state
{
  public:
    min::vec3<float> position;
    min::vec3<float> look;
    min::vec3<float> up;
    std::vector<item> inventory;
    std::array<uint_fast16_t, stats::stat_str_size()> stat;
    uint_fast16_t stat_points;
    float energy;
    float exp;
    float health;
    float oxygen;
    std::vector<min::vec3<float>> chest;

  public:
    game_state(const min::vec3<float> &p, const min::vec3<float> &dir, const min::vec3<float> &u)
        : position(p), look(dir), up(u),
          stat_points(0), energy(0.0),
          exp(0.0), health(0.0), oxygen(0.0) {}
};

class load_state
{
  private:
    uint32_t _grid_size;
    min::vec3<float> _default_spawn;
    min::vec3<float> _default_look;
    min::vec3<float> _default_up;
    min::vec3<float> _top;
    int_fast8_t _game_mode;
    bool _new_game;
    game_state _state;

    inline void check_inside()
    {
        // Compute the min and max dimension of grid
        const float min = _grid_size * -1.0;
        const float max = _grid_size;

        // Compute the min and max bounds of grid
        const min::vec3<float> gmin(min, min, min);
        const min::vec3<float> gmax(max, max, max);

        // Test if point is inside the grid
        if (!_state.position.inside(gmin, gmax))
        {
            // Alert changing spawn
            std::cout << "load_state: spawn out of bounds: resetting spawn point" << std::endl;

            // Erase previous state files
            erase_file("save/state");

            // Set a valid spawn
            _state.position = _default_spawn;
            _state.look = _default_look;
            _state.up = _default_up;

            // Test if the spawn is valid
            if (!_state.position.inside(gmin, gmax))
            {
                throw std::runtime_error("load_state: failed to load a valid spawn point");
            }
        }
    }
    inline void reserve_memory()
    {
        const size_t start = inventory::begin_store();
        const size_t end = inventory::end_cube();
        _state.inventory.reserve(end - start);

        const size_t chest_size = static_instance::max_chests();
        _state.chest.reserve(chest_size);
    }
    inline void state_load_file()
    {
        // Create output stream for loading world
        std::vector<uint8_t> stream;

        // Load data into stream from file
        load_file("save/state", stream);

        // If load failed dont try to parse stream data
        if (stream.size() != 0)
        {
            // Flag that this is not a new game
            _new_game = false;

            // Character position
            size_t next = 0;

            // Read grid size from stream
            const uint32_t grid_size = min::read_le<uint32_t>(stream, next);
            if (grid_size != _grid_size)
            {
                // Warn user that grid sizes not compatible
                std::cout << "Resizing the grid: deleting old save caches" << std::endl;

                // Erase previous state files
                game::erase_file("save/state");
                game::erase_file("save/world.bmesh");

                // Early return
                return;
            }

            // Load the game mode
            const int_fast8_t game_mode = min::read_le<uint8_t>(stream, next);
            if (_game_mode == 2)
            {
                // Keep the loaded flag
                _game_mode = game_mode;
            }
            else if (_game_mode != game_mode)
            {
                // Alert mode switch to user
                if (_game_mode == 1)
                {
                    std::cout << "Switching game mode to HARDCORE!" << std::endl;
                }
                else
                {
                    std::cout << "Switching game mode to NORMAL!" << std::endl;
                }
            }

            // Read position from stream
            const float x = min::read_le<float>(stream, next);
            const float y = min::read_le<float>(stream, next);
            const float z = min::read_le<float>(stream, next);

            // Load position
            _state.position = min::vec3<float>(x, y, z);

            // Look direction
            const float lx = min::read_le<float>(stream, next);
            const float ly = min::read_le<float>(stream, next);
            const float lz = min::read_le<float>(stream, next);

            // Load look vector
            _state.look = min::vec3<float>(lx, ly, lz);

            // Up vector
            const float ux = min::read_le<float>(stream, next);
            const float uy = min::read_le<float>(stream, next);
            const float uz = min::read_le<float>(stream, next);

            // Load look vector
            _state.up = min::vec3<float>(ux, uy, uz);

            // Load inventory data from stream
            const size_t start = inventory::begin_store();
            const size_t end = inventory::end_cube();
            const uint32_t inv_size = end - start;
            const uint32_t read_inv_size = min::read_le<uint32_t>(stream, next);

            // Check inventory array size
            if (read_inv_size > inventory::size() || read_inv_size != inv_size)
            {
                throw std::runtime_error("load_state: incompatible inventory size");
            }

            // Copy data
            for (size_t i = start; i < end; i++)
            {
                const item_id id = static_cast<item_id>(min::read_le<uint8_t>(stream, next));
                const int_fast8_t count = min::read_le<uint8_t>(stream, next);
                const int_fast8_t prim = min::read_le<uint8_t>(stream, next);
                const int_fast8_t sec = min::read_le<uint8_t>(stream, next);
                const int_fast8_t level = min::read_le<uint8_t>(stream, next);
                _state.inventory.emplace_back(id, count, prim, sec, level);
            }

            // Load stats from stream
            const size_t stat_size = stats::stat_str_size();
            const uint32_t read_stat_size = min::read_le<uint32_t>(stream, next);

            // Check inventory array size
            if (read_stat_size != stat_size)
            {
                throw std::runtime_error("load_state: incompatible stat size");
            }

            // Copy data
            for (size_t i = 0; i < stat_size; i++)
            {
                _state.stat[i] = min::read_le<uint16_t>(stream, next);
            }

            // Load misc data
            _state.stat_points = min::read_le<uint16_t>(stream, next);
            _state.energy = min::read_le<float>(stream, next);
            _state.exp = min::read_le<float>(stream, next);
            _state.health = min::read_le<float>(stream, next);
            _state.oxygen = min::read_le<float>(stream, next);

            // Load the chest positions
            const size_t chest_size = min::read_le<uint32_t>(stream, next);
            if (chest_size > static_instance::max_chests())
            {
                throw std::runtime_error("load_state: incompatible chest size");
            }

            // Copy data
            for (size_t i = 0; i < chest_size; i++)
            {
                _state.chest.push_back(min::read_le_vec3<float>(stream, next));
            }
        }
        else
        {
            // Default to normal mode
            _game_mode = 0;
        }
    }

  public:
    load_state(const options &opt)
        : _grid_size(static_cast<uint32_t>(opt.grid())),
          _default_spawn(0.0, _grid_size * 0.75, 0.0),
          _default_look(1.0, _grid_size * 0.75, 0.0),
          _default_up(0.0, 1.0, 0.0),
          _top(0.0, _grid_size - 1.0, 0.0),
          _game_mode(opt.mode()),
          _new_game(true),
          _state(_default_spawn, _default_look, _default_up)
    {
        // Check for integer overflow
        if (opt.grid() > std::numeric_limits<uint32_t>::max())
        {
            throw std::runtime_error("load_state: integer overflow detected, aborting");
        }

        // Reserve memory
        reserve_memory();

        // Load state
        state_load_file();

        // Check that we loaded a valid point
        check_inside();
    }
    inline const min::vec3<float> &get_default_spawn() const
    {
        return _default_spawn;
    }
    inline const min::vec3<float> &get_default_look() const
    {
        return _default_look;
    }
    inline const min::vec3<float> &get_default_up() const
    {
        return _default_up;
    }
    inline const min::vec3<float> &get_top() const
    {
        return _top;
    }
    inline bool is_hardcore() const
    {
        return _game_mode == 1;
    }
    inline bool is_new_game() const
    {
        return _new_game;
    }
    inline const min::vec3<float> &get_position() const
    {
        return _state.position;
    }
    inline const min::vec3<float> &get_look_at() const
    {
        return _state.look;
    }
    inline const min::vec3<float> &get_up() const
    {
        return _state.up;
    }
    inline const std::vector<item> &get_inventory() const
    {
        return _state.inventory;
    }
    inline const std::array<uint_fast16_t, stats::stat_str_size()> &get_stats() const
    {
        return _state.stat;
    }
    inline uint_fast16_t get_stat_points() const
    {
        return _state.stat_points;
    }
    inline float get_energy() const
    {
        return _state.energy;
    }
    inline float get_exp() const
    {
        return _state.exp;
    }
    inline float get_health() const
    {
        return _state.health;
    }
    inline float get_oxygen() const
    {
        return _state.oxygen;
    }
    inline const std::vector<min::vec3<float>> &get_chests() const
    {
        return _state.chest;
    }
    inline void save_state()
    {
        // Create output stream for saving world
        std::vector<uint8_t> stream;

        // Cache the file size
        stream.reserve(438);

        // Write the grid size into stream
        min::write_le<uint32_t>(stream, _grid_size);

        // Write the game mode into stream
        min::write_le<uint8_t>(stream, _game_mode);

        // Write position into stream
        min::write_le<float>(stream, _state.position.x());
        min::write_le<float>(stream, _state.position.y());
        min::write_le<float>(stream, _state.position.z());

        // Get the camera look and write into stream
        min::write_le<float>(stream, _state.look.x());
        min::write_le<float>(stream, _state.look.y());
        min::write_le<float>(stream, _state.look.z());

        // Get the camera up and write into stream
        min::write_le<float>(stream, _state.up.x());
        min::write_le<float>(stream, _state.up.y());
        min::write_le<float>(stream, _state.up.z());

        // Write inventory data into stream
        const uint32_t inv_size = _state.inventory.size();
        min::write_le<uint32_t>(stream, inv_size);
        for (size_t i = 0; i < inv_size; i++)
        {
            const item &it = _state.inventory[i];
            min::write_le<uint8_t>(stream, static_cast<uint8_t>(it.id()));
            min::write_le<uint8_t>(stream, it.count());
            min::write_le<uint8_t>(stream, it.primary());
            min::write_le<uint8_t>(stream, it.secondary());
            min::write_le<uint8_t>(stream, it.level());
        }

        // Write stats into stream
        const uint32_t stat_size = _state.stat.size();
        min::write_le<uint32_t>(stream, stat_size);
        for (size_t i = 0; i < stat_size; i++)
        {
            min::write_le<uint16_t>(stream, _state.stat[i]);
        }

        // Save misc data
        min::write_le<uint16_t>(stream, _state.stat_points);
        min::write_le<float>(stream, _state.energy);
        min::write_le<float>(stream, _state.exp);
        min::write_le<float>(stream, _state.health);
        min::write_le<float>(stream, _state.oxygen);

        // Write chests into stream
        const size_t chest_size = _state.chest.size();
        min::write_le<uint32_t>(stream, static_cast<uint32_t>(chest_size));
        for (size_t i = 0; i < chest_size; i++)
        {
            // Save the chest locations
            const min::vec3<float> &p = _state.chest[i];

            // !!! - Undo chest adjustment, in world.h - !!!!
            min::write_le_vec3<float>(stream, min::vec3<float>(p.x(), p.y() + 1.0, p.z()));
        }

        // Write data to file
        save_file("save/state", stream);
    }
    inline void set_state(const min::vec3<float> &p, const min::camera<float> &camera, const inventory &inv, const stats &stat, const static_instance &si)
    {
        // Copy position
        _state.position = p;

        // Copy look
        _state.look = camera.project_point(1.0);

        // Copy up
        _state.up = camera.get_up();

        // Copy inventory
        const size_t start = inv.begin_store();
        const size_t end = inv.end_cube();
        _state.inventory.clear();
        for (size_t i = start; i < end; i++)
        {
            _state.inventory.push_back(inv[i]);
        }

        // Copy stats
        const uint32_t stat_size = stat.stat_str_size();
        for (size_t i = 0; i < stat_size; i++)
        {
            _state.stat[i] = stat.stat_value(i);
        }

        // Copy misc data
        _state.stat_points = stat.get_stat_points();
        _state.energy = stat.get_energy();
        _state.exp = stat.get_exp();
        _state.health = stat.get_health();
        _state.oxygen = stat.get_oxygen();

        // Copy chests
        const std::vector<min::mat4<float>> &chests = si.get_chest().get_in_matrix();
        const size_t chest_size = chests.size();
        _state.chest.clear();
        for (size_t i = 0; i < chest_size; i++)
        {
            _state.chest.push_back(chests[i].get_translation());
        }
    }
};
}

#endif
