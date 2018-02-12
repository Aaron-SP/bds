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
#ifndef __ITEM_BLOCK_ID__
#define __ITEM_BLOCK_ID__

#include <cstdint>

namespace game
{

enum class block_id : int8_t
{
    GRASS1 = 0,
    GRASS2 = 1,
    DIRT1 = 2,
    DIRT2 = 3,
    SAND1 = 4,
    SAND2 = 5,
    IRON = 6,
    SILVER = 7,
    WOOD1 = 8,
    WOOD2 = 9,
    LEAF1 = 10,
    LEAF2 = 11,
    LEAF3 = 12,
    LEAF4 = 13,
    MAGNESIUM = 14,
    GOLD = 15,
    STONE1 = 16,
    STONE2 = 17,
    CLAY1 = 18,
    CLAY2 = 19,
    STONE3 = 20,
    COPPER = 21,
    SODIUM = 22,
    CALCIUM = 23,
    CRYSTAL_R = 24,
    CRYSTAL_P = 25,
    CRYSTAL_B = 26,
    CRYSTAL_G = 27,
    POTASSIUM = 30
};

enum class item_id : uint8_t
{
    BLK_FE = 23,
    BLK_SI = 24,
    BLK_MG = 31,
    BLK_AU = 32,
    BLK_CU = 38,
    BLK_NA = 39,
    BLK_CA = 40,
    BLK_CRYS_R = 41,
    BLK_CRYS_P = 42,
    BLK_CRYS_B = 43,
    BLK_CRYS_G = 44,
    BLK_K = 47,

    SHARD_R = 81,
    SHARD_P = 82,
    SHARD_B = 83,
    SHARD_G = 84,
    CAT_CA = 86,
    CAT_CU = 87,
    CAT_FE = 89,
    CAT_MG = 90,
    CAT_K = 91,
    CAT_NA = 92,
    BAR_AU = 103,
    BAR_SI = 104
};

inline constexpr int8_t id_value(const block_id id)
{
    return static_cast<int8_t>(id);
}

inline constexpr uint8_t id_value(const item_id id)
{
    return static_cast<uint8_t>(id);
}
}

#endif
