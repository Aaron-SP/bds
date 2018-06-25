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
#ifndef __OPTIONS__
#define __OPTIONS__

#include <cstdint>
#include <game/id.h>
#include <iostream>
#include <string>

namespace game
{

enum class key_map_type
{
    QWERTY,
    DVORAK
};

enum class game_type : uint_fast8_t
{
    NORMAL = 0,
    HARDCORE = 1,
    CREATIVE = 2
};

inline constexpr uint_fast8_t id_value(const game_type id)
{
    return static_cast<uint_fast8_t>(id);
}

class options
{
  private:
    size_t _chunk;
    size_t _frames;
    size_t _grid;
    game_type _mode;
    size_t _slot;
    size_t _view;
    uint_fast16_t _width;
    uint_fast16_t _height;
    key_map_type _map;
    bool _persist;
    bool _resize;

  public:
    options()
        : _chunk(8), _frames(60), _grid(64),
          _mode(game_type::NORMAL), _slot(0), _view(5),
          _width(1024), _height(768),
          _map(key_map_type::QWERTY), _persist(true), _resize(true) {}

    inline bool check_error() const
    {
        // Do some sanity checks on values
        if (_grid < 4)
        {
            std::cout << "bds: '-grid' must be atleast 4" << std::endl;
            return true;
        }
        else if (_chunk < 2)
        {
            std::cout << "bds: '-chunk' must be atleast 2" << std::endl;
            return true;
        }
        else if (_view < 3)
        {
            std::cout << "bds: '-view' must be atleast 3" << std::endl;
            return true;
        }

        // No errors
        return false;
    }
    inline size_t chunk() const
    {
        return _chunk;
    }
    inline size_t frames() const
    {
        return _frames;
    }
    inline game_type get_game_mode() const
    {
        return _mode;
    }
    inline size_t get_save_slot() const
    {
        return _slot;
    }
    inline size_t grid() const
    {
        return _grid;
    }
    inline size_t view() const
    {
        return _view;
    }
    inline uint_fast16_t width() const
    {
        return _width;
    }
    inline uint_fast16_t height() const
    {
        return _height;
    }
    inline bool is_key_map_dvorak() const
    {
        return _map == key_map_type::DVORAK;
    }
    inline bool is_key_map_persist() const
    {
        return _persist;
    }
    inline bool is_key_map_qwerty() const
    {
        return _map == key_map_type::QWERTY;
    }
    inline bool resize() const
    {
        return _resize;
    }
    inline void set_chunk(const size_t chunk)
    {
        _chunk = chunk;
    }
    inline void set_frames(const size_t frames)
    {
        _frames = frames;
    }
    inline void set_grid(const size_t grid)
    {
        _grid = grid;
    }
    inline void set_game_mode(const game_type mode)
    {
        _mode = mode;
    }
    inline void set_no_persist()
    {
        _persist = false;
    }
    inline void set_save_slot(const size_t slot)
    {
        _slot = slot;
    }
    inline void set_view(const size_t view)
    {
        _view = view;
    }
    inline void set_width(const uint_fast16_t width)
    {
        _width = width;
    }
    inline void set_height(const uint_fast16_t height)
    {
        _height = height;
    }
    inline void set_map(const key_map_type map)
    {
        _map = map;
    }
    inline void set_resize(const bool flag)
    {
        _resize = flag;
    }
};
}

#endif
