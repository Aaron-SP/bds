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
    POTASSIUM = 30,
    TOMATO = 32,
    EGGPLANT = 33,
    RED_PEPPER = 34,
    GREEN_PEPPER = 35
};

enum class item_id : uint8_t
{
    BLK_GRASS1 = 17,
    BLK_GRASS2 = 18,
    BLK_DIRT1 = 19,
    BLK_DIRT2 = 20,
    BLK_SAND1 = 21,
    BLK_SAND2 = 22,
    BLK_FE = 23,
    BLK_SI = 24,
    BLK_WOOD1 = 25,
    BLK_WOOD2 = 26,
    BLK_LEAF1 = 27,
    BLK_LEAF2 = 28,
    BLK_LEAF3 = 29,
    BLK_LEAF4 = 30,
    BLK_MG = 31,
    BLK_AU = 32,
    BLK_STONE1 = 33,
    BLK_STONE2 = 34,
    BLK_CLAY1 = 35,
    BLK_CLAY2 = 36,
    BLK_STONE3 = 37,
    BLK_CU = 38,
    BLK_NA = 39,
    BLK_CA = 40,
    BLK_CRYS_R = 41,
    BLK_CRYS_P = 42,
    BLK_CRYS_B = 43,
    BLK_CRYS_G = 44,
    BLK_K = 47,
    BLK_TOM = 49,
    BLK_EGGP = 50,
    BLK_RED_PEP = 51,
    BLK_GR_PEP = 52,

    SHARD_R = 81,
    SHARD_P = 82,
    SHARD_B = 83,
    SHARD_G = 84,
    CAT_NH4 = 85,
    CAT_CA = 86,
    CAT_CU = 87,
    CAT_H = 88,
    CAT_FE = 89,
    CAT_MG = 90,
    CAT_K = 91,
    CAT_NA = 92,
    AN_CARB = 93,
    AN_CL = 94,
    AN_NO3 = 95,
    AN_PHOS = 96,
    AN_SULPH = 97,
    BAR_CA = 98,
    BAR_CU = 99,
    BAR_FE = 100,
    BAR_MG = 101,
    BAR_K = 102,
    BAR_NA = 103,
    BAR_AU = 104,
    BAR_SI = 105,
    ACID_HCL = 106,
    ACID_HNO3 = 107,
    ACID_H3PO4 = 108,
    ACID_H2SO4 = 109,
    POWD_BGUANO = 110,
    POWD_CAL_CARB = 111,
    POWD_CHARCOAL = 112,
    POWD_MAG_CARB = 113,
    POWD_KNO3 = 114,
    POWD_RED_PHOS = 115,
    POWD_RUST = 116,
    POWD_SALT = 117,
    POWD_SULPHUR = 118,
    POWD_UREA = 119,
    CONS_EGGP = 120,
    CONS_GR_PEP = 121,
    CONS_RED_PEP = 122,
    CONS_TOM = 123,
    CONS_BATTERY = 124,
    CONS_ETHER = 125,
    CONS_OXYGEN = 126,
    CONS_KEY = 127
};

enum class skill_id : uint8_t
{
    AUTO_BEAM = 1,
    BEAM = 2,
    CHARGE = 3,
    GRAPPLE = 4,
    GRENADE = 5,
    JET = 6,
    MISSILE = 7,
    PORTAL = 8,
    SCAN = 9,
    SCATTER = 10,
    SPEED = 11
};

inline constexpr int8_t id_value(const block_id id)
{
    return static_cast<int8_t>(id);
}
inline constexpr uint8_t id_value(const item_id id)
{
    return static_cast<uint8_t>(id);
}
inline constexpr uint8_t id_value(const skill_id id)
{
    return static_cast<uint8_t>(id);
}

enum class ui_type
{
    store,
    key,
    extend,
    cube,
    button
};

class ui_id
{
  private:
    uint8_t _id;

  public:
    ui_id() : _id(0) {}
    ui_id(const uint8_t id) : _id(id) {}
    inline bool operator==(const ui_id other) const
    {
        return _id == other.id();
    }
    inline bool operator!=(const ui_id other) const
    {
        return _id != other.id();
    }
    inline unsigned row3() const
    {
        return (_id - 40) / 3;
    }
    inline unsigned col3() const
    {
        return (_id - 40) % 3;
    }
    inline unsigned row8() const
    {
        return _id / 8;
    }
    inline unsigned col8() const
    {
        return _id & 7;
    }
    inline uint8_t id() const
    {
        return _id;
    }
    inline size_t index() const
    {
        return _id;
    }
    inline ui_id bg_store_index() const
    {
        return ui_id(_id + 9);
    }
    inline ui_id store_index() const
    {
        return ui_id(_id + 17);
    }
    inline ui_id bg_key_index() const
    {
        return ui_id(_id + 17);
    }
    inline ui_id key_index() const
    {
        return ui_id(_id + 25);
    }
    inline ui_id bg_ext_index() const
    {
        return ui_id(_id + 25);
    }
    inline ui_id ext_index() const
    {
        return ui_id(_id + 49);
    }
    inline ui_id bg_cube_index() const
    {
        return ui_id(_id + 50);
    }
    inline ui_id cube_index() const
    {
        return ui_id(_id + 59);
    }
    inline ui_id button_index() const
    {
        return ui_id(_id + 59);
    }
    inline ui_id to_key() const
    {
        return ui_id(_id + 8);
    }
    inline ui_type type() const
    {
        if (_id >= 49)
        {
            // Button type
            return ui_type::button;
        }
        else if (_id >= 40)
        {
            // Cube type
            return ui_type::cube;
        }
        else if (_id >= 16)
        {
            // Extended type
            return ui_type::extend;
        }
        else if (_id >= 8)
        {
            // Key type
            return ui_type::key;
        }

        // Store type
        return ui_type::store;
    }
};
}

#endif
