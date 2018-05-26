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
#ifndef __UI_INFO__
#define __UI_INFO__

#include <cstdint>
#include <game/id.h>
#include <string>

namespace game
{

class ui_info
{
  private:
    const std::string *const _name;
    const std::string *const _info;
    const item _item;

  public:
    ui_info(const std::string &name, const std::string &info, const item it)
        : _name(&name), _info(&info), _item(it) {}

    const std::string &get_name() const
    {
        return *_name;
    }
    const std::string &get_info() const
    {
        return *_info;
    }
    uint_fast8_t primary() const
    {
        return _item.primary();
    }
    uint_fast8_t secondary() const
    {
        return _item.secondary();
    }
    item_type type() const
    {
        return _item.type();
    }
};
}

#endif
