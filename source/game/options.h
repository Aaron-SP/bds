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

class options
{
  private:
    size_t _chunk;
    size_t _frames;
    size_t _grid;
    uint_fast8_t _mode;
    size_t _view;
    uint_fast16_t _width;
    uint_fast16_t _height;
    bool _resize;

  public:
    options() : _chunk(8), _frames(60), _grid(64), _mode(2), _view(5), _width(1024), _height(768), _resize(true) {}

    bool check_error() const
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
        else if (_mode > 2)
        {
            std::cout << "bds: '-hardcore' must be 0 or 1" << std::endl;
            return true;
        }

        // No errors
        return false;
    }
    size_t chunk() const
    {
        return _chunk;
    }
    size_t frames() const
    {
        return _frames;
    }
    size_t grid() const
    {
        return _grid;
    }
    size_t view() const
    {
        return _view;
    }
    uint_fast16_t width() const
    {
        return _width;
    }
    uint_fast16_t height() const
    {
        return _height;
    }
    uint_fast8_t mode() const
    {
        return _mode;
    }
    bool resize() const
    {
        return _resize;
    }
    void set_chunk(const size_t chunk)
    {
        _chunk = chunk;
    }
    void set_frames(const size_t frames)
    {
        _frames = frames;
    }
    void set_grid(const size_t grid)
    {
        _grid = grid;
    }
    void set_mode(const uint_fast8_t mode)
    {
        _mode = mode;
    }
    void set_view(const size_t view)
    {
        _view = view;
    }
    void set_width(const uint_fast16_t width)
    {
        _width = width;
    }
    void set_height(const uint_fast16_t height)
    {
        _height = height;
    }
    void set_resize(const bool flag)
    {
        _resize = flag;
    }
};
}

#endif
