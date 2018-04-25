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

#include <game/file.h>
#include <iostream>
#include <min/vec3.h>
#include <stdexcept>
#include <vector>

namespace game
{
class load_state
{
  private:
    min::vec3<float> _default_look;
    min::vec3<float> _default_spawn;
    min::vec3<float> _look;
    min::vec3<float> _spawn;
    min::vec3<float> _top;
    std::vector<item> _inv;
    bool _loaded;

    inline void check_inside(const float grid_size)
    {
        // Compute the min and max dimension of grid
        const float min = grid_size * -1.0;
        const float max = grid_size;

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
    inline void state_load_file(const size_t grid_size)
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
            const size_t start = inventory::begin_key();
            const size_t end = inventory::end_cube();
            for (size_t i = start; i < end; i++)
            {
                const uint8_t id = min::read_le<uint8_t>(stream, next);
                const uint8_t count = min::read_le<uint8_t>(stream, next);
                _inv[i] = item(id, count);
            }
        }
    }
    inline void state_save_file(const inventory &inv, const min::camera<float> &camera, const min::vec3<float> &p)
    {
        // Create output stream for saving world
        std::vector<uint8_t> stream;

        // Write position into stream
        min::write_le<float>(stream, p.x());
        min::write_le<float>(stream, p.y());
        min::write_le<float>(stream, p.z());

        // Get the camera look position
        const min::vec3<float> look = camera.project_point(1.0);

        // Write look into stream
        min::write_le<float>(stream, look.x());
        min::write_le<float>(stream, look.y());
        min::write_le<float>(stream, look.z());

        // Write inventory data into stream
        const size_t start = inv.begin_key();
        const size_t end = inv.end_cube();
        for (size_t i = start; i < end; i++)
        {
            const item &it = inv[i];
            min::write_le<uint8_t>(stream, it.id());
            min::write_le<uint8_t>(stream, it.count());
        }

        // Write data to file
        save_file("bin/state", stream);
    }

  public:
    load_state(const float grid_size)
        : _default_look(1.0, grid_size * 0.75, 0.0),
          _default_spawn(0.0, grid_size * 0.75, 0.0),
          _look(_default_look), _spawn(_default_spawn),
          _top(0.0, grid_size - 1.0, 0.0),
          _inv(inventory::size()), _loaded(false)
    {
        // Check that we loaded a valid point
        check_inside(grid_size);

        // Load state
        state_load_file(grid_size);
    }
    inline const min::vec3<float> &get_default_look() const
    {
        return _default_look;
    }
    inline const min::vec3<float> &get_default_spawn() const
    {
        return _default_spawn;
    }
    inline const std::vector<item> &get_inventory() const
    {
        return _inv;
    }
    inline const min::vec3<float> &get_look() const
    {
        return _look;
    }
    inline const min::vec3<float> &get_spawn() const
    {
        return _spawn;
    }
    inline const min::vec3<float> &get_top() const
    {
        return _top;
    }
    inline bool is_loaded() const
    {
        return _loaded;
    }
    inline void save_state(const inventory &inv, const min::camera<float> &camera, const min::vec3<float> &p)
    {
        state_save_file(inv, camera, p);
    }
};
}

#endif
