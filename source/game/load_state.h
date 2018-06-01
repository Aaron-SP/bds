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
#include <game/static_instance.h>
#include <game/stats.h>
#include <iostream>
#include <limits>
#include <min/vec3.h>
#include <stdexcept>
#include <vector>

namespace game
{
class load_state
{
  private:
    uint32_t _grid_size;
    min::vec3<float> _default_look;
    min::vec3<float> _default_spawn;
    min::vec3<float> _look;
    min::vec3<float> _spawn;
    min::vec3<float> _top;
    std::vector<item> _inv;
    std::array<uint_fast16_t, stats::stat_str_size()> _stat;
    float _energy;
    float _exp;
    float _health;
    float _oxygen;
    uint_fast16_t _stat_points;
    std::vector<min::vec3<float>> _chests;
    bool _new_game;
    int_fast8_t _game_mode;

    inline void check_inside()
    {
        // Compute the min and max dimension of grid
        const float min = _grid_size * -1.0;
        const float max = _grid_size;

        // Compute the min and max bounds of grid
        const min::vec3<float> gmin(min, min, min);
        const min::vec3<float> gmax(max, max, max);

        // Test if point is inside the grid
        if (!_spawn.inside(gmin, gmax))
        {
            // Alert changing spawn
            std::cout << "load_state: spawn out of bounds: resetting spawn point" << std::endl;

            // Erase previous state files
            erase_file("bin/state");

            // Set a valid spawn
            _spawn = _default_spawn;
            _look = _default_look;

            // Test if the spawn is valid
            if (!_spawn.inside(gmin, gmax))
            {
                throw std::runtime_error("load_state: failed to load a valid spawn point");
            }
        }
    }
    inline void state_load_file()
    {
        // Create output stream for loading world
        std::vector<uint8_t> stream;

        // Load data into stream from file
        load_file("bin/state", stream);

        // If load failed dont try to parse stream data
        if (stream.size() != 0)
        {
            // Character position
            size_t next = 0;

            // Read grid size from stream
            const uint32_t grid_size = min::read_le<uint32_t>(stream, next);
            if (grid_size != _grid_size)
            {
                // Warn user that grid sizes not compatible
                std::cout << "Resizing the grid: deleting old save caches" << std::endl;

                // Erase previous state files
                game::erase_file("bin/state");
                game::erase_file("bin/world.bmesh");

                // Early return
                return;
            }

            // Read position from stream
            const float x = min::read_le<float>(stream, next);
            const float y = min::read_le<float>(stream, next);
            const float z = min::read_le<float>(stream, next);

            // Load position
            _spawn = min::vec3<float>(x, y, z);

            // Look direction
            const float lx = min::read_le<float>(stream, next);
            const float ly = min::read_le<float>(stream, next);
            const float lz = min::read_le<float>(stream, next);

            // Load look vector
            _look = min::vec3<float>(lx, ly, lz);

            // Load inventory data from stream
            const size_t start = inventory::begin_store();
            const size_t end = inventory::end_cube();
            const uint32_t inv_length = end - start;
            const uint32_t read_inv_size = min::read_le<uint32_t>(stream, next);

            // Check inventory array size
            if (read_inv_size > inventory::size() || read_inv_size != inv_length)
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
                _inv[i] = item(id, count, prim, sec, level);
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
                _stat[i] = min::read_le<uint16_t>(stream, next);
            }

            // Load energy from stream
            _energy = min::read_le<float>(stream, next);

            // Load exp from stream
            _exp = min::read_le<float>(stream, next);

            // Load health from stream
            _health = min::read_le<float>(stream, next);

            // Load oxygen from stream
            _oxygen = min::read_le<float>(stream, next);

            // Load stats from stream
            _stat_points = min::read_le<uint16_t>(stream, next);

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

            // Load the chest positions
            const size_t chest_size = min::read_le<uint32_t>(stream, next);
            if (chest_size > static_instance::max_chests())
            {
                throw std::runtime_error("load_state: incompatible chest size");
            }

            // Copy data
            for (size_t i = 0; i < chest_size; i++)
            {
                _chests.push_back(min::read_le_vec3<float>(stream, next));
            }

            // Flag that this is not a new game
            _new_game = false;
        }
        else
        {
            // Default to normal mode
            _game_mode = 0;
        }
    }
    inline void state_save_file(const static_instance &si, const inventory &inv, const stats &stat, const min::camera<float> &camera, const min::vec3<float> &p)
    {
        // Create output stream for saving world
        std::vector<uint8_t> stream;

        // Write the grid size into stream
        min::write_le<uint32_t>(stream, _grid_size);

        // Write position into stream
        min::write_le<float>(stream, p.x());
        min::write_le<float>(stream, p.y());
        min::write_le<float>(stream, p.z());

        // Get the camera look position and write into stream
        const min::vec3<float> look = camera.project_point(1.0);
        min::write_le<float>(stream, look.x());
        min::write_le<float>(stream, look.y());
        min::write_le<float>(stream, look.z());

        // Write inventory data into stream
        const size_t start = inv.begin_store();
        const size_t end = inv.end_cube();
        const uint32_t inv_length = end - start;
        min::write_le<uint32_t>(stream, inv_length);
        for (size_t i = start; i < end; i++)
        {
            const item &it = inv[i];
            min::write_le<uint8_t>(stream, static_cast<uint8_t>(it.id()));
            min::write_le<uint8_t>(stream, it.count());
            min::write_le<uint8_t>(stream, it.primary());
            min::write_le<uint8_t>(stream, it.secondary());
            min::write_le<uint8_t>(stream, it.level());
        }

        // Write stats into stream
        const uint32_t stat_size = stat.stat_str_size();
        min::write_le<uint32_t>(stream, stat_size);
        for (size_t i = 0; i < stat_size; i++)
        {
            min::write_le<uint16_t>(stream, stat.stat_value(i));
        }

        // Save the player energy
        min::write_le<float>(stream, stat.get_energy());

        // Save the player experience
        min::write_le<float>(stream, stat.get_exp());

        // Save the player health
        min::write_le<float>(stream, stat.get_health());

        // Save the player oxygen
        min::write_le<float>(stream, stat.get_oxygen());

        // Save the player stats
        min::write_le<uint16_t>(stream, stat.get_stat_points());

        // Save the game mode
        min::write_le<uint8_t>(stream, _game_mode);

        // Get the chests
        const std::vector<min::mat4<float>> &chests = si.get_chest().get_in_matrix();

        // Write chests into stream
        const size_t chest_size = chests.size();
        min::write_le<uint32_t>(stream, static_cast<uint32_t>(chest_size));
        for (size_t i = 0; i < chest_size; i++)
        {
            // Save the chest locations
            const min::vec3<float> p = chests[i].get_translation();

            // !!! - Undo chest adjustment, in world.h - !!!!
            min::write_le_vec3<float>(stream, min::vec3<float>(p.x(), p.y() + 1.0, p.z()));
        }

        // Write data to file
        save_file("bin/state", stream);
    }

  public:
    load_state(const size_t grid_size, const int_fast8_t game_mode)
        : _grid_size(static_cast<uint32_t>(grid_size)),
          _default_look(1.0, _grid_size * 0.75, 0.0),
          _default_spawn(0.0, _grid_size * 0.75, 0.0),
          _look(_default_look), _spawn(_default_spawn),
          _top(0.0, _grid_size - 1.0, 0.0),
          _inv(inventory::size()),
          _stat{}, _energy(0.0), _exp(0.0), _health(0.0), _oxygen(0.0), _stat_points(0),
          _new_game(true), _game_mode(game_mode)
    {
        // Check for integer overflow
        if (grid_size > std::numeric_limits<uint32_t>::max())
        {
            throw std::runtime_error("load_state: integer overflow detected, aborting");
        }

        // Check that we loaded a valid point
        check_inside();

        // Load state
        state_load_file();
    }
    inline const std::vector<min::vec3<float>> &get_chests() const
    {
        return _chests;
    }
    inline const min::vec3<float> &get_default_look() const
    {
        return _default_look;
    }
    inline const min::vec3<float> &get_default_spawn() const
    {
        return _default_spawn;
    }
    inline float get_energy() const
    {
        return _energy;
    }
    inline float get_exp() const
    {
        return _exp;
    }
    inline float get_health() const
    {
        return _health;
    }
    inline const std::vector<item> &get_inventory() const
    {
        return _inv;
    }
    inline const min::vec3<float> &get_look() const
    {
        return _look;
    }
    inline float get_oxygen() const
    {
        return _oxygen;
    }
    inline const min::vec3<float> &get_spawn() const
    {
        return _spawn;
    }
    inline uint_fast16_t get_stat_points() const
    {
        return _stat_points;
    }
    inline const std::array<uint_fast16_t, stats::stat_str_size()> &get_stats() const
    {
        return _stat;
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
    inline void save_state(const static_instance &si, const inventory &inv, const stats &stat, const min::camera<float> &cam, const min::vec3<float> &p)
    {
        state_save_file(si, inv, stat, cam, p);
    }
};
}

#endif
