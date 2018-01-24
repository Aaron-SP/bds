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
#ifndef __MEMORY_MAP__
#define __MEMORY_MAP__

#include <min/mem_chunk.h>
namespace game
{

// Global memory chunk to load all game data
class memory_map
{
  public:
    static min::mem_chunk memory;
};

// Load the memory mapped file
min::mem_chunk memory_map::memory("data/data.sky");
}

#endif
