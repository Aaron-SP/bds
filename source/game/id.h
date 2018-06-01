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

enum class block_id : int_fast8_t
{
    INVALID = -2,
    EMPTY = -1,
    SAND1 = 0,
    SAND2 = 1,
    DIRT1 = 2,
    DIRT2 = 3,
    CLAY1 = 4,
    CLAY2 = 5,
    STONE1 = 6,
    STONE2 = 7,
    STONE3 = 8,
    GRASS1 = 9,
    GRASS2 = 10,
    WOOD1 = 11,
    WOOD2 = 12,
    LEAF1 = 13,
    LEAF2 = 14,
    LEAF3 = 15,
    LEAF4 = 16,
    TOMATO = 17,
    EGGPLANT = 18,
    RED_PEPPER = 19,
    GREEN_PEPPER = 20,

    CALCIUM = 24,
    MAGNESIUM = 25,
    COPPER = 26,
    POTASSIUM = 27,
    IRON = 28,
    SODIUM = 29,
    IRIDIUM = 30,

    SILVER = 32,
    GOLD = 33,
    CRYSTAL_R = 34,
    CRYSTAL_P = 35,
    CRYSTAL_B = 36,
    CRYSTAL_G = 37,
};

inline constexpr bool not_empty(const block_id id)
{
    return static_cast<int_fast8_t>(id) >= 0;
}

inline constexpr int ether_cost(const block_id id)
{
    return static_cast<int_fast8_t>(id) + 1;
}

enum class item_type
{
    empty,
    skill,
    block,
    item
};

enum class item_id : uint_fast8_t
{
    EMPTY = 0,
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
    SPEED = 11,

    BLK_SAND1 = 17,
    BLK_SAND2 = 18,
    BLK_DIRT1 = 19,
    BLK_DIRT2 = 20,
    BLK_CLAY1 = 21,
    BLK_CLAY2 = 22,
    BLK_STONE1 = 23,
    BLK_STONE2 = 24,
    BLK_STONE3 = 25,
    BLK_GRASS1 = 26,
    BLK_GRASS2 = 27,
    BLK_WOOD1 = 28,
    BLK_WOOD2 = 29,
    BLK_LEAF1 = 30,
    BLK_LEAF2 = 31,
    BLK_LEAF3 = 32,
    BLK_LEAF4 = 33,
    BLK_TOM = 34,
    BLK_EGGP = 35,
    BLK_RED_PEP = 36,
    BLK_GR_PEP = 37,

    BLK_CA = 41,
    BLK_MG = 42,
    BLK_CU = 43,
    BLK_K = 44,
    BLK_FE = 45,
    BLK_NA = 46,
    BLK_IR = 47,

    BLK_AG = 49,
    BLK_AU = 50,
    BLK_CRYS_R = 51,
    BLK_CRYS_P = 52,
    BLK_CRYS_B = 53,
    BLK_CRYS_G = 54,

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

inline constexpr int_fast8_t id_value(const block_id id)
{
    return static_cast<int_fast8_t>(id);
}
inline constexpr uint_fast8_t id_value(const item_id id)
{
    return static_cast<uint_fast8_t>(id);
}

inline static constexpr block_id id_to_atlas(const item_id id)
{
    // Convert inventory id to cube atlas
    return static_cast<block_id>(id_value(id) - 17);
}
inline static constexpr item_id id_from_atlas(const block_id id)
{
    // Convert from atlas to inventory id
    return static_cast<item_id>(id_value(id) + 17);
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
    uint_fast8_t _id;

  public:
    ui_id() : _id(0) {}
    ui_id(const uint_fast8_t id) : _id(id) {}
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
    inline uint_fast8_t id() const
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
    inline ui_id fg_store_index() const
    {
        return ui_id(_id + 17);
    }
    inline ui_id bg_key_index() const
    {
        return ui_id(_id + 17);
    }
    inline ui_id fg_key_index() const
    {
        return ui_id(_id + 25);
    }
    inline ui_id bg_ext_index() const
    {
        return ui_id(_id + 25);
    }
    inline ui_id fg_ext_index() const
    {
        return ui_id(_id + 49);
    }
    inline ui_id bg_menu_index() const
    {
        return ui_id(_id + 41);
    }
    inline ui_id fg_menu_index() const
    {
        return ui_id(_id + 46);
    }
    inline ui_id bg_cube_index() const
    {
        return ui_id(_id + 50);
    }
    inline ui_id fg_cube_index() const
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
