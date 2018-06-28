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
#ifndef __ITEM__
#define __ITEM__

#include <cstdint>
#include <game/id.h>
#include <string>

namespace game
{

class item
{
  private:
    uint_fast8_t _id;
    uint_fast8_t _count;
    uint_fast8_t _prim;
    uint_fast8_t _sec;
    uint_fast8_t _level;

  public:
    item()
        : _id(0), _count(0), _prim(1), _sec(1), _level(1) {}
    item(const item_id id, const uint_fast8_t count)
        : _id(id_value(id)), _count(count), _prim(1), _sec(1), _level(1) {}
    item(const item_id id, const uint_fast8_t count,
         const uint_fast8_t prim, const uint_fast8_t sec, const uint_fast8_t level)
        : _id(id_value(id)), _count(count),
          _prim(prim), _sec(sec), _level(level) {}

    inline bool operator<(const item &other) const
    {
        return _id < other._id;
    }
    inline uint_fast8_t to_block_id() const
    {
        return _id - 17;
    }
    inline uint_fast8_t to_item_id() const
    {
        return _id - 81;
    }
    inline void consume(const uint_fast8_t count)
    {
        _count -= count;
    }
    inline uint_fast8_t count() const
    {
        return _count;
    }
    inline item_id id() const
    {
        return static_cast<item_id>(_id);
    }
    inline uint_fast8_t level() const
    {
        return _level;
    }
    inline uint_fast8_t primary() const
    {
        return _prim;
    }
    inline uint_fast8_t secondary() const
    {
        return _sec;
    }
    inline void set_count(const uint_fast8_t count)
    {
        _count = count;
    }
    inline void set_empty()
    {
        _id = 0;
        _count = 0;
        _prim = 1;
        _sec = 1;
        _level = 1;
    }
    inline void stack(uint_fast8_t &count)
    {
        // Calculate sum of items
        const uint_fast16_t sum = _count + count;

        // Calculate left over items
        count = (sum / 255) * (sum % 255);

        // Set the stack item count
        _count = sum - count;
    }
    inline item_type type() const
    {
        if (_id == 0)
        {
            return item_type::empty;
        }
        else if (_id < 17)
        {
            return item_type::skill;
        }
        else if (_id < 81)
        {
            return item_type::block;
        }

        return item_type::item;
    }
};
}

#endif
