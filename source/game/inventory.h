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
#ifndef __INVENTORY__
#define __INVENTORY__

#include <array>
#include <game/id.h>
#include <game/item.h>
#include <limits>
#include <random>
#include <utility>
#include <vector>

namespace game
{

class inventory
{
  private:
    class craft_item
    {
      private:
        size_t _index;
        item _item;

      public:
        craft_item() : _index(0), _item() {}
        craft_item(const size_t index, const item &it) : _index(index), _item(it) {}
        inline size_t index() const
        {
            return _index;
        }
        inline const item &get_item() const
        {
            return _item;
        }
        inline bool operator<(const craft_item &other) const
        {
            return _item < other.get_item();
        }
    };

    static constexpr size_t _drop_count = 5;
    static constexpr size_t _max_slots = 49;
    static constexpr size_t _max_strings = 128;
    static constexpr size_t _cube_size = 9;
    std::array<item, _max_slots> _inv;
    std::vector<std::string> _inv_name;
    std::vector<std::string> _inv_desc;
    std::vector<ui_id> _update;
    std::array<craft_item, _cube_size> _craft;
    std::mt19937 _gen;
    std::uniform_int_distribution<uint_fast8_t> _drop_item;
    std::uniform_int_distribution<int_fast8_t> _item_mult;
    std::uniform_int_distribution<int_fast8_t> _level_offset;
    std::uniform_int_distribution<uint_fast8_t> _random_item;
    unsigned _player_level;

    inline std::pair<bool, item_id> decay(const uint_fast8_t index, const item_id consume_id, uint_fast8_t &count)
    {
        // Only consume item do not convert
        return std::make_pair(consume(index, consume_id, count), consume_id);
    }
    inline std::pair<bool, item_id> decay(const uint_fast8_t index, const item_id consume_id, const item_id add_id, uint_fast8_t &count, const uint_fast8_t add_count)
    {
        // Did we decay anything?
        bool status = false;

        // Cache item for reset
        item cache = _inv[index];

        // Transform one item into another
        if (consume(index, consume_id, count))
        {
            // Reset count and add value
            count = add_count;

            // If we couldn't add the item rollback consume
            if (!add(add_id, count))
            {
                _inv[index] = cache;
            }
            else
            {
                status = true;
            }
        }

        return std::make_pair(status, consume_id);
    }
    inline void consume_item(const size_t index, item &it, uint_fast8_t &count)
    {
        // Consume the inventory count
        it.consume(count);

        // Set the remaining count
        count = it.count();

        // If count is zero set empty ID
        if (it.count() == 0)
        {
            it.set_empty();
        }

        // Store update index
        _update.emplace_back(index);
    }
    inline bool consume2(
        const size_t index_1, const item_id id_1, uint_fast8_t &count_1,
        const size_t index_2, const item_id id_2, uint_fast8_t &count_2)
    {
        // Get the current item
        item &it_1 = _inv[index_1];
        item &it_2 = _inv[index_2];

        const bool it1_pass = (it_1.id() == id_1) && (it_1.count() >= count_1);
        const bool it2_pass = (it_2.id() == id_2) && (it_2.count() >= count_2);

        // Are these the items we want?
        if (it1_pass && it2_pass)
        {
            // Consume both resources
            consume_item(index_1, it_1, count_1);
            consume_item(index_2, it_2, count_2);

            // Return that we consumed the resource
            return true;
        }

        return false;
    }
    inline bool consume3(
        const size_t index_1, const item_id id_1, uint_fast8_t &count_1,
        const size_t index_2, const item_id id_2, uint_fast8_t &count_2,
        const size_t index_3, const item_id id_3, uint_fast8_t &count_3)
    {
        // Get the current item
        item &it_1 = _inv[index_1];
        item &it_2 = _inv[index_2];
        item &it_3 = _inv[index_3];

        const bool it1_pass = (it_1.id() == id_1) && (it_1.count() >= count_1);
        const bool it2_pass = (it_2.id() == id_2) && (it_2.count() >= count_2);
        const bool it3_pass = (it_3.id() == id_3) && (it_3.count() >= count_3);

        // Are these the items we want?
        if ((it1_pass && it2_pass) && it3_pass)
        {
            // Consume both resources
            consume_item(index_1, it_1, count_1);
            consume_item(index_2, it_2, count_2);
            consume_item(index_3, it_3, count_3);

            // Return that we consumed the resource
            return true;
        }

        return false;
    }
    inline void load_strings()
    {
        _inv_name[0] = "Empty";
        _inv_name[1] = "Automatic Beam";
        _inv_desc[1] = "A general purpose energy_weapon firing multiple beams";
        _inv_name[2] = "Beam";
        _inv_desc[2] = "A general purpose energy_weapon firing a singular beam";
        _inv_name[3] = "Charge Beam";
        _inv_desc[3] = "A general purpose energy_weapon firing a charged beam";
        _inv_name[4] = "Grappling Hook";
        _inv_desc[4] = "Ionized grappling device._Requires dynamics unlocked.";
        _inv_name[5] = "Grenade Launcher";
        _inv_desc[5] = "A cheap offensive projectile_weapon";
        _inv_name[6] = "Jet Pack";
        _inv_desc[6] = "Thermal vaporization device._Requires dynamics unlocked.";
        _inv_name[7] = "Missile Launcher";
        _inv_desc[7] = "An expensive offensive_projectile weapon";
        _inv_name[8] = "Portal Beam";
        _inv_desc[8] = "It's portal time baby!";
        _inv_name[9] = "Pending Scan";
        _inv_desc[9] = "Scan a block to retrieve_the block_type";
        _inv_name[10] = "Scatter Beam";
        _inv_desc[10] = "An energy weapon optimized_for killing drones";
        _inv_name[11] = "Speed Boots";
        _inv_desc[11] = "Sometimes you just need_to run for it!";
        _inv_name[12] = "Reserved";
        _inv_name[13] = "Reserved";
        _inv_name[14] = "Reserved";
        _inv_name[15] = "Reserved";
        _inv_name[16] = "Reserved";
        _inv_name[17] = "White Sand";
        _inv_desc[17] = "Right click to transform into Ether";
        _inv_name[18] = "Yellow Sand";
        _inv_desc[18] = "Right click to transform into_Ether";
        _inv_name[19] = "Soil";
        _inv_desc[19] = "Right click to transform into_Ether";
        _inv_name[20] = "Fertile Soil";
        _inv_desc[20] = "Right click to transform into_Ether";
        _inv_name[21] = "Light Clay";
        _inv_desc[21] = "Right click to transform into_Ether";
        _inv_name[22] = "Dark Clay";
        _inv_desc[22] = "Right click to transform into_Ether";
        _inv_name[23] = "Light Stone";
        _inv_desc[23] = "Right click to transform into_Ether";
        _inv_name[24] = "Dark Stone";
        _inv_desc[24] = "Right click to transform into_Ether";
        _inv_name[25] = "Mossy Stone";
        _inv_desc[25] = "Right click to transform into_Ether";
        _inv_name[26] = "Grass";
        _inv_desc[26] = "Right click to transform into_Ether";
        _inv_name[27] = "Dense Grass";
        _inv_desc[27] = "Right click to transform into_Ether";
        _inv_name[28] = "Oak";
        _inv_desc[28] = "Right click to transform into_Ether";
        _inv_name[29] = "Pine";
        _inv_desc[29] = "Right click to transform into_Ether";
        _inv_name[30] = "Dark Foliage";
        _inv_desc[30] = "Right click to transform into_Ether";
        _inv_name[31] = "Light Vegetation";
        _inv_desc[31] = "Right click to transform into_Ether";
        _inv_name[32] = "Blooming Growth";
        _inv_desc[32] = "Right click to transform into_Ether";
        _inv_name[33] = "Flowery Growth";
        _inv_desc[33] = "Right click to transform into_Ether";
        _inv_name[34] = "Tomatoes";
        _inv_desc[34] = "Right click to harvest";
        _inv_name[35] = "Eggplant";
        _inv_desc[35] = "Right click to harvest";
        _inv_name[36] = "Red Peppers";
        _inv_desc[36] = "Right click to harvest";
        _inv_name[37] = "Green Peppers";
        _inv_desc[37] = "Right click to harvest";
        _inv_name[38] = "???";
        _inv_name[39] = "???";
        _inv_name[40] = "???";
        _inv_name[41] = "Calcium";
        _inv_desc[41] = "Right click to ionize";
        _inv_name[42] = "Magnesium";
        _inv_desc[42] = "Right click to ionize";
        _inv_name[43] = "Copper";
        _inv_desc[43] = "Right click to ionize";
        _inv_name[44] = "Potassium";
        _inv_desc[44] = "Right click to ionize";
        _inv_name[45] = "Iron";
        _inv_desc[45] = "Right click to ionize";
        _inv_name[46] = "Unstable Sodium";
        _inv_desc[46] = "Right click to ionize";
        _inv_name[47] = "Iridium";
        _inv_desc[47] = "Unbreakable!";
        _inv_name[48] = "???";
        _inv_name[49] = "Silver";
        _inv_name[50] = "Gold";
        _inv_name[51] = "Red Crystals";
        _inv_desc[51] = "Right click to harvest";
        _inv_name[52] = "Purple Crystals";
        _inv_desc[52] = "Right click to harvest";
        _inv_name[53] = "Blue Crystals";
        _inv_desc[53] = "Right click to harvest";
        _inv_name[54] = "Green Crystals";
        _inv_desc[54] = "Right click to harvest";
        _inv_name[55] = "???";
        _inv_name[56] = "???";
        _inv_name[81] = "Red Crystal Shards";
        _inv_desc[81] = "Transforms raw compounds_into processed elements";
        _inv_name[82] = "Purple Crystal Shards";
        _inv_name[83] = "Blue Crystal Shards";
        _inv_desc[83] = "Transforms ionic compounds_into stable elements";
        _inv_name[84] = "Green Crystal Shards";
        _inv_desc[84] = "Upgrades block resources_to a higher form";
        _inv_name[85] = "Ammonium [NH4+]";
        _inv_desc[85] = "Decays into [NO3-]";
        _inv_name[86] = "Calcium [Ca+]";
        _inv_name[87] = "Copper [Cu2+]";
        _inv_name[88] = "Hydrogen [H+]";
        _inv_name[89] = "Iron [Fe2+]";
        _inv_name[90] = "Magnesium [Mg2+]";
        _inv_name[91] = "Potassium [K+]";
        _inv_name[92] = "Sodium [Na+]";
        _inv_name[93] = "Carbonate [(CO3)2-]";
        _inv_desc[93] = "Contains sequestered oxygen";
        _inv_name[94] = "Chloride [Cl-]";
        _inv_name[95] = "Nitrate [NO3-]";
        _inv_name[96] = "Phosphate [(PO4)3-]";
        _inv_name[97] = "Sulfate [(SO4)2-]";
        _inv_name[98] = "Calcium Bar";
        _inv_name[99] = "Copper Bar";
        _inv_name[100] = "Iron Bar";
        _inv_name[101] = "Magnesium Bar";
        _inv_name[102] = "Potassium Bar";
        _inv_name[103] = "Sodium Bar";
        _inv_name[104] = "Gold Bar";
        _inv_name[105] = "Silver Bar";
        _inv_name[106] = "Hydrochloric Acid";
        _inv_name[107] = "Nitric Acid";
        _inv_name[108] = "Phosphoric Acid";
        _inv_name[109] = "Sulfuric Acid";
        _inv_name[110] = "Bat Guano";
        _inv_name[111] = "Calcium Carbonate";
        _inv_desc[111] = "Contains oxygen";
        _inv_name[112] = "Charcoal";
        _inv_desc[112] = "High flame potential";
        _inv_name[113] = "Magnesium Carbonate";
        _inv_desc[113] = "Contains oxygen";
        _inv_name[114] = "Potassium Nitrate";
        _inv_desc[114] = "Fragile very explosive";
        _inv_name[115] = "Red Phosphorus";
        _inv_name[116] = "Rust";
        _inv_name[117] = "Salt";
        _inv_name[118] = "Sulphur";
        _inv_name[119] = "Urea";
        _inv_name[120] = "Eggplant";
        _inv_desc[120] = "Right click to eat._It's tender and juicy!";
        _inv_name[121] = "Green Pepper";
        _inv_desc[121] = "Right click to eat._It's tender and juicy!";
        _inv_name[122] = "Red Pepper";
        _inv_desc[122] = "Right click to eat._It's tender and juicy!";
        _inv_name[123] = "Tomato";
        _inv_desc[123] = "Right click to eat._It's tender and juicy!";
        _inv_name[124] = "Battery";
        _inv_desc[124] = "Right click to use._The power is overwhelming!";
        _inv_name[125] = "Ether";
        _inv_desc[125] = "The building block of all matter";
        _inv_name[126] = "Oxygen";
        _inv_desc[126] = "Vital for sustaining life!";
        _inv_name[127] = "Rusty Key";
        _inv_desc[127] = "It's old and rusty._Perhaps this opens_something!";
    }
    inline item make_item(const item_id id, const uint_fast8_t count)
    {
        // Make a standard item
        item it(id, count);

        if (it.type() == item_type::skill)
        {
            const int_fast8_t offset = _level_offset(_gen);
            const int_fast8_t base_level = _player_level + offset;

            // Scale primary and secondary stat
            const uint_fast8_t i_lvl = (base_level > 0) ? base_level : 1;
            const uint_fast8_t p_stat = (i_lvl * (_item_mult(_gen) / _item_mult(_gen))) + 1;
            const uint_fast8_t s_stat = (i_lvl * (_item_mult(_gen) / _item_mult(_gen))) + 1;

            // Return item
            return item(id, count, p_stat, s_stat, i_lvl);
        }

        // Return standard item
        return it;
    }
    inline item make_item(const item_id id, const uint_fast8_t count, const uint_fast8_t p_stat, const uint_fast8_t s_stat, const uint_fast8_t i_lvl)
    {
        // Return item
        return item(id, count, p_stat, s_stat, i_lvl);
    }
    inline void set_inventory()
    {
        _inv[begin_key()] = make_item(item_id::BAR_FE, 1);
        _inv[begin_key() + 1] = make_item(item_id::BAR_AU, 1);
        _inv[begin_key() + 2] = make_item(item_id::BAR_SI, 1);
    }
    inline void set_store() {}
    inline bool stack(const size_t one, const size_t two)
    {
        // Try to stack items if same type
        if (_inv[one].id() == _inv[two].id())
        {
            uint_fast8_t count = _inv[one].count();
            _inv[two].stack(count);

            // If items are left
            if (count > 0)
            {
                _inv[one].set_count(count);
            }
            else
            {
                // Discard empty item
                _inv[one].set_empty();
            }

            // Submit indices for updating
            _update.emplace_back(one);
            _update.emplace_back(two);

            // Return that we stacked
            return true;
        }

        // Return that we didn't stack
        return false;
    }

  public:
    inventory()
        : _inv{}, _inv_name(_max_strings), _inv_desc(_max_strings), _craft{},
          _gen(std::chrono::high_resolution_clock::now().time_since_epoch().count()),
          _drop_item(begin_key(), end_cube() - 1), _item_mult(1, 4),
          _level_offset(-10, 10),
          _random_item(id_value(item_id::AUTO_BEAM), id_value(item_id::SPEED)),
          _player_level(1)
    {
        // Load inv` strings
        load_strings();

        // Reserve memory for update buffer
        _update.reserve(_max_slots);

        // Create items in the inventory
        set_inventory();
    }
    inline const item &operator[](const size_t index) const
    {
        return _inv[index];
    }
    inline const item &get_cube(const size_t index) const
    {
        return _inv[index + begin_cube()];
    }
    inline const item &get_extend(const size_t index) const
    {
        return _inv[index + begin_extend()];
    }
    inline const item &get_key(const size_t index) const
    {
        return _inv[index + begin_key()];
    }
    inline const item &get_store(const size_t index) const
    {
        return _inv[index + begin_store()];
    }
    inline bool add(const item_id id, uint_fast8_t &count)
    {
        // Search for item of same id in slots
        const size_t size = end_cube();

        // First pass, prefer searching all slots for a stack
        for (size_t i = begin_key(); i < size; i++)
        {
            // Get the current item
            item &it = _inv[i];

            // Can we stack this item?
            if (it.id() == id)
            {
                // Try to stack the item
                it.stack(count);

                // Store update index
                _update.emplace_back(i);

                // If fully stacked early break
                if (count == 0)
                {
                    // Return added
                    return true;
                }
            }
        }

        // Second pass, get the first empty slot in inventory
        for (size_t i = begin_key(); i < size; i++)
        {
            // Get the current item
            item &it = _inv[i];

            // Is this slot empty?
            if (it.id() == item_id::EMPTY)
            {
                it = make_item(id, count);

                // Store update index
                _update.emplace_back(i);

                // Signal that we picked it up
                count = 0;

                // Return added
                return true;
            }
        }

        // Failed to add all items
        return false;
    }
    inline static constexpr size_t begin_store()
    {
        return 0;
    }
    inline static constexpr size_t end_store()
    {
        return 8;
    }
    inline static constexpr size_t begin_key()
    {
        return 8;
    }
    inline static constexpr size_t end_key()
    {
        return 16;
    }
    inline static constexpr size_t begin_extend()
    {
        return 16;
    }
    inline static constexpr size_t end_extend()
    {
        return 40;
    }
    inline static constexpr size_t begin_cube()
    {
        return 40;
    }
    inline static constexpr size_t end_cube()
    {
        return begin_cube() + _cube_size;
    }
    inline static constexpr size_t size()
    {
        return _max_slots;
    }
    inline void clean()
    {
        _update.clear();
    }
    inline bool consume(const item_id id, uint_fast8_t &count)
    {
        // Search for item of same id in slots
        const size_t size = end_cube();
        for (size_t i = begin_key(); i < size; i++)
        {
            // Try to consume this index
            if (consume(i, id, count))
            {
                return true;
            }
        }

        // Return that we failed to consume
        return false;
    }
    inline bool consume(const size_t index, const item_id id, uint_fast8_t &count)
    {
        // Get the current item
        item &it = _inv[index];

        // Is this the item I want?
        if (it.id() == id && it.count() >= count)
        {
            consume_item(index, it, count);

            // Return that we consumed the resource
            return true;
        }

        return false;
    }
    inline bool consume_multi(const item_id id, const unsigned count)
    {
        unsigned total = 0;

        // Count the number of items we got
        const size_t size = end_cube();
        for (size_t i = begin_key(); i < size; i++)
        {
            // Count the resource
            if (_inv[i].id() == id)
            {
                total += _inv[i].count();
            }
        }

        // If we have enough items
        if (total >= count)
        {
            // Reset total for countdown
            total = count;

            // Begin consuming
            for (size_t i = begin_key(); i < size; i++)
            {
                // Get the current item
                item &it = _inv[i];

                // Try to consume this index
                if (it.id() == id)
                {
                    // Get the item count
                    const uint_fast8_t available = it.count();
                    if (total > available)
                    {
                        // Consume resource
                        uint_fast8_t count = available;
                        consume_item(i, it, count);

                        // Decriment total consumed
                        total -= available;
                    }
                    else
                    {
                        // Consume resource
                        uint_fast8_t count = static_cast<uint_fast8_t>(total);
                        consume_item(i, it, count);

                        // Return that we finished consuming
                        return true;
                    }
                }
            }
        }

        // Return that we failed to consume
        return false;
    }
    inline bool random_item()
    {
        // Get a random skill ID
        const item_id id = static_cast<item_id>(_random_item(_gen));

        // Set the item count
        uint_fast8_t count = 1;

        // Create item in store
        switch (id)
        {
        case item_id::GRENADE:
        case item_id::MISSILE:
            count = 16;
            break;
        default:
            break;
        }

        // Add skill to inventory
        return add(id, count);
    }
    inline bool recipe_2(const uint_fast8_t mult)
    {
        // Sort the crafting array by ID
        std::sort(_craft.begin(), _craft.begin() + 2, std::less<craft_item>());

        // Get indices in sorted order
        const size_t lower = _craft[0].index();
        const size_t higher = _craft[1].index();
        uint_fast8_t low_count = mult;
        uint_fast8_t high_count = mult;
        uint_fast8_t high_count_16 = mult * 16;

        // How many items to craft
        uint_fast8_t add_count = mult;

        // Try to craft a recipe
        if (consume2(lower, item_id::BLK_FE, low_count, higher, item_id::CAT_H, high_count))
        {
            return add(item_id::POWD_RUST, add_count);
        }
        else if (consume2(lower, item_id::POWD_CHARCOAL, low_count, higher, item_id::POWD_KNO3, high_count))
        {
            add_count = 4 * mult;
            return add(item_id::GRENADE, add_count);
        }
        else if (consume2(lower, item_id::CAT_K, low_count, higher, item_id::AN_NO3, high_count))
        {
            return add(item_id::POWD_KNO3, add_count);
        }
        else if (consume2(lower, item_id::CAT_CA, low_count, higher, item_id::AN_CARB, high_count))
        {
            return add(item_id::POWD_CAL_CARB, add_count);
        }
        else if (consume2(lower, item_id::CAT_MG, low_count, higher, item_id::AN_CARB, high_count))
        {
            return add(item_id::POWD_MAG_CARB, add_count);
        }
        else if (consume2(lower, item_id::CAT_NA, low_count, higher, item_id::AN_CL, high_count))
        {
            return add(item_id::POWD_SALT, add_count);
        }
        else if (consume2(lower, item_id::CAT_H, low_count, higher, item_id::AN_CL, high_count))
        {
            return add(item_id::ACID_HCL, add_count);
        }
        else if (consume2(lower, item_id::CAT_H, low_count, higher, item_id::AN_NO3, high_count))
        {
            return add(item_id::ACID_HNO3, add_count);
        }
        else if (consume2(lower, item_id::CAT_H, low_count, higher, item_id::AN_PHOS, high_count))
        {
            return add(item_id::ACID_H3PO4, add_count);
        }
        else if (consume2(lower, item_id::CAT_H, low_count, higher, item_id::AN_SULPH, high_count))
        {
            return add(item_id::ACID_H2SO4, add_count);
        }

        // Blue shard recipes
        else if (consume2(lower, item_id::SHARD_B, low_count, higher, item_id::CAT_CA, high_count))
        {
            return add(item_id::BLK_CA, add_count);
        }
        else if (consume2(lower, item_id::SHARD_B, low_count, higher, item_id::CAT_CU, high_count))
        {
            return add(item_id::BLK_CU, add_count);
        }
        else if (consume2(lower, item_id::SHARD_B, low_count, higher, item_id::CAT_FE, high_count))
        {
            return add(item_id::BLK_FE, add_count);
        }
        else if (consume2(lower, item_id::SHARD_B, low_count, higher, item_id::CAT_MG, high_count))
        {
            return add(item_id::BLK_MG, add_count);
        }
        else if (consume2(lower, item_id::SHARD_B, low_count, higher, item_id::CAT_K, high_count))
        {
            return add(item_id::BLK_K, add_count);
        }
        else if (consume2(lower, item_id::SHARD_B, low_count, higher, item_id::CAT_NA, high_count))
        {
            return add(item_id::BLK_NA, add_count);
        }
        else if (consume2(lower, item_id::SHARD_B, low_count, higher, item_id::AN_NO3, high_count))
        {
            return add(item_id::CAT_NH4, add_count);
        }
        else if (consume2(lower, item_id::SHARD_B, low_count, higher, item_id::AN_PHOS, high_count))
        {
            return add(item_id::POWD_RED_PHOS, add_count);
        }
        else if (consume2(lower, item_id::SHARD_B, low_count, higher, item_id::AN_SULPH, high_count))
        {
            return add(item_id::POWD_SULPHUR, add_count);
        }

        // Green shard recipes
        else if (consume2(lower, item_id::BLK_CLAY1, low_count, higher, item_id::SHARD_G, high_count))
        {
            return add(item_id::BLK_STONE1, add_count);
        }
        else if (consume2(lower, item_id::BLK_CLAY2, low_count, higher, item_id::SHARD_G, high_count))
        {
            return add(item_id::BLK_STONE2, add_count);
        }
        else if (consume2(lower, item_id::BLK_STONE1, low_count, higher, item_id::SHARD_G, high_count))
        {
            return add(item_id::BLK_CA, add_count);
        }
        else if (consume2(lower, item_id::BLK_STONE2, low_count, higher, item_id::SHARD_G, high_count))
        {
            return add(item_id::BLK_CA, add_count);
        }
        else if (consume2(lower, item_id::BLK_CA, low_count, higher, item_id::SHARD_G, high_count))
        {
            return add(item_id::BLK_MG, add_count);
        }
        else if (consume2(lower, item_id::BLK_MG, low_count, higher, item_id::SHARD_G, high_count))
        {
            return add(item_id::BLK_K, add_count);
        }
        else if (consume2(lower, item_id::BLK_K, low_count, higher, item_id::SHARD_G, high_count))
        {
            return add(item_id::BLK_CU, add_count);
        }
        else if (consume2(lower, item_id::BLK_LEAF1, low_count, higher, item_id::SHARD_G, high_count))
        {
            return add(item_id::BLK_TOM, add_count);
        }
        else if (consume2(lower, item_id::BLK_LEAF2, low_count, higher, item_id::SHARD_G, high_count))
        {
            return add(item_id::BLK_EGGP, add_count);
        }
        else if (consume2(lower, item_id::BLK_LEAF3, low_count, higher, item_id::SHARD_G, high_count))
        {
            return add(item_id::BLK_GR_PEP, add_count);
        }
        else if (consume2(lower, item_id::BLK_LEAF4, low_count, higher, item_id::SHARD_G, high_count))
        {
            return add(item_id::BLK_RED_PEP, add_count);
        }
        else if (consume2(lower, item_id::BLK_SAND1, low_count, higher, item_id::SHARD_G, high_count))
        {
            return add(item_id::BLK_DIRT1, add_count);
        }
        else if (consume2(lower, item_id::BLK_SAND2, low_count, higher, item_id::SHARD_G, high_count))
        {
            return add(item_id::BLK_DIRT2, add_count);
        }
        else if (consume2(lower, item_id::BLK_DIRT1, low_count, higher, item_id::SHARD_G, high_count))
        {
            return add(item_id::BLK_GRASS1, add_count);
        }
        else if (consume2(lower, item_id::BLK_DIRT2, low_count, higher, item_id::SHARD_G, high_count))
        {
            return add(item_id::BLK_GRASS2, add_count);
        }
        else if (consume2(lower, item_id::BLK_GRASS1, low_count, higher, item_id::SHARD_G, high_count))
        {
            return add(item_id::BLK_WOOD1, add_count);
        }
        else if (consume2(lower, item_id::BLK_GRASS2, low_count, higher, item_id::SHARD_G, high_count))
        {
            return add(item_id::BLK_WOOD2, add_count);
        }
        else if (consume2(lower, item_id::BLK_WOOD1, low_count, higher, item_id::SHARD_G, high_count))
        {
            return add(item_id::BLK_FE, add_count);
        }
        else if (consume2(lower, item_id::BLK_WOOD2, low_count, higher, item_id::SHARD_G, high_count))
        {
            return add(item_id::BLK_FE, add_count);
        }
        else if (consume2(lower, item_id::BLK_FE, low_count, higher, item_id::SHARD_G, high_count))
        {
            return add(item_id::BLK_NA, add_count);
        }
        else if (consume2(lower, item_id::SHARD_G, low_count, higher, item_id::POWD_RUST, high_count))
        {
            add_count = 8 * mult;
            return add(item_id::CONS_OXYGEN, add_count);
        }
        else if (consume2(lower, item_id::SHARD_G, low_count, higher, item_id::POWD_CAL_CARB, high_count))
        {
            add_count = 8 * mult;
            return add(item_id::CONS_OXYGEN, add_count);
        }
        else if (consume2(lower, item_id::SHARD_G, low_count, higher, item_id::POWD_MAG_CARB, high_count))
        {
            add_count = 8 * mult;
            return add(item_id::CONS_OXYGEN, add_count);
        }

        // Red shard recipes
        else if (consume2(lower, item_id::BLK_WOOD1, low_count, higher, item_id::SHARD_R, high_count))
        {
            return add(item_id::POWD_CHARCOAL, add_count);
        }
        else if (consume2(lower, item_id::BLK_WOOD2, low_count, higher, item_id::SHARD_R, high_count))
        {
            return add(item_id::POWD_CHARCOAL, add_count);
        }
        else if (consume2(lower, item_id::BLK_CA, low_count, higher, item_id::SHARD_R, high_count))
        {
            return add(item_id::BAR_CA, add_count);
        }
        else if (consume2(lower, item_id::BLK_CU, low_count, higher, item_id::SHARD_R, high_count))
        {
            return add(item_id::BAR_CU, add_count);
        }
        else if (consume2(lower, item_id::BLK_FE, low_count, higher, item_id::SHARD_R, high_count))
        {
            return add(item_id::BAR_FE, add_count);
        }
        else if (consume2(lower, item_id::BLK_MG, low_count, higher, item_id::SHARD_R, high_count))
        {
            return add(item_id::BAR_MG, add_count);
        }
        else if (consume2(lower, item_id::BLK_K, low_count, higher, item_id::SHARD_R, high_count))
        {
            return add(item_id::BAR_K, add_count);
        }
        else if (consume2(lower, item_id::BLK_NA, low_count, higher, item_id::SHARD_R, high_count))
        {
            return add(item_id::BAR_NA, add_count);
        }
        else if (consume2(lower, item_id::BLK_AU, low_count, higher, item_id::SHARD_R, high_count))
        {
            return add(item_id::BAR_AU, add_count);
        }
        else if (consume2(lower, item_id::BLK_AG, low_count, higher, item_id::SHARD_R, high_count))
        {
            return add(item_id::BAR_SI, add_count);
        }

        // Purple shard recipes
        else if (consume2(lower, item_id::AUTO_BEAM, low_count, higher, item_id::SHARD_P, high_count_16))
        {
            return add(item_id::AUTO_BEAM, add_count);
        }
        else if (consume2(lower, item_id::BEAM, low_count, higher, item_id::SHARD_P, high_count_16))
        {
            return add(item_id::BEAM, add_count);
        }
        else if (consume2(lower, item_id::CHARGE, low_count, higher, item_id::SHARD_P, high_count_16))
        {
            return add(item_id::CHARGE, add_count);
        }
        else if (consume2(lower, item_id::GRAPPLE, low_count, higher, item_id::SHARD_P, high_count_16))
        {
            return add(item_id::GRAPPLE, add_count);
        }
        else if (consume2(lower, item_id::GRENADE, low_count, higher, item_id::SHARD_P, high_count_16))
        {
            return add(item_id::GRENADE, add_count);
        }
        else if (consume2(lower, item_id::JET, low_count, higher, item_id::SHARD_P, high_count_16))
        {
            return add(item_id::JET, add_count);
        }
        else if (consume2(lower, item_id::MISSILE, low_count, higher, item_id::SHARD_P, high_count_16))
        {
            return add(item_id::MISSILE, add_count);
        }
        else if (consume2(lower, item_id::PORTAL, low_count, higher, item_id::SHARD_P, high_count_16))
        {
            return add(item_id::PORTAL, add_count);
        }
        else if (consume2(lower, item_id::SCAN, low_count, higher, item_id::SHARD_P, high_count_16))
        {
            return add(item_id::SCAN, add_count);
        }
        else if (consume2(lower, item_id::SCATTER, low_count, higher, item_id::SHARD_P, high_count_16))
        {
            return add(item_id::SCATTER, add_count);
        }
        else if (consume2(lower, item_id::SPEED, low_count, higher, item_id::SHARD_P, high_count_16))
        {
            return add(item_id::SPEED, add_count);
        }

        // Missiles
        else if (consume2(lower, item_id::BAR_FE, low_count, higher, item_id::BAR_NA, high_count))
        {
            add_count = 4 * mult;
            return add(item_id::MISSILE, add_count);
        }
        // Keys
        else if (consume2(lower, item_id::BAR_FE, low_count, higher, item_id::BAR_SI, high_count))
        {
            return add(item_id::CONS_KEY, add_count);
        }

        // Failed to craft item
        return false;
    }
    inline bool recipe_3(const uint_fast8_t mult)
    {
        // Sort the crafting array by ID
        std::sort(_craft.begin(), _craft.begin() + 3, std::less<craft_item>());

        // Get indices in sorted order
        const size_t lower = _craft[0].index();
        const size_t middle = _craft[1].index();
        const size_t higher = _craft[2].index();
        uint_fast8_t low_count = mult;
        uint_fast8_t mid_count = mult;
        uint_fast8_t high_count = mult;
        uint_fast8_t low_count_4 = mult * 4;
        uint_fast8_t mid_count_4 = mult * 4;
        uint_fast8_t high_count_4 = mult * 4;
        uint_fast8_t up_count = mult;

        // How many items to craft
        uint_fast8_t add_count = mult;

        // Urea
        if (consume3(lower, item_id::SHARD_B, low_count_4,
                     middle, item_id::CAT_NH4, mid_count_4,
                     higher, item_id::POWD_CHARCOAL, high_count_4))
        {
            return add(item_id::POWD_UREA, add_count);
        }

        // Battery
        else if (consume3(lower, item_id::BAR_NA, low_count_4,
                          middle, item_id::ACID_H2SO4, mid_count_4,
                          higher, item_id::POWD_SALT, high_count_4))
        {
            add_count = mult * 2;
            return add(item_id::CONS_BATTERY, add_count);
        }

        // Beam
        else if (consume3(lower, item_id::BAR_FE, low_count,
                          middle, item_id::BAR_AU, mid_count,
                          higher, item_id::BAR_SI, high_count))
        {
            return add(item_id::BEAM, add_count);
        }

        // Auto beam
        else if (consume3(lower, item_id::BEAM, up_count,
                          middle, item_id::BAR_CU, mid_count_4,
                          higher, item_id::CONS_BATTERY, high_count_4))
        {
            return add(item_id::AUTO_BEAM, add_count);
        }

        // Charge beam
        else if (consume3(lower, item_id::BEAM, up_count,
                          middle, item_id::BAR_AU, mid_count_4,
                          higher, item_id::BAR_SI, high_count_4))
        {
            return add(item_id::CHARGE, add_count);
        }

        // Grappling Hook
        else if (consume3(lower, item_id::BAR_FE, low_count_4,
                          middle, item_id::BAR_AU, mid_count_4,
                          higher, item_id::POWD_RED_PHOS, high_count_4))
        {
            return add(item_id::GRAPPLE, add_count);
        }

        // Jet pack
        else if (consume3(lower, item_id::BAR_FE, low_count_4,
                          middle, item_id::POWD_KNO3, mid_count_4,
                          higher, item_id::POWD_UREA, high_count_4))
        {
            return add(item_id::JET, add_count);
        }

        // Scatter beam
        else if (consume3(lower, item_id::BEAM, up_count,
                          middle, item_id::BAR_FE, mid_count_4,
                          higher, item_id::POWD_UREA, high_count_4))
        {
            return add(item_id::SCATTER, add_count);
        }

        // Failed to craft item
        return false;
    }
    inline std::pair<bool, item_id> craft(const size_t index, const uint_fast8_t mult)
    {
        size_t craft_size = 0;
        const size_t begin = begin_cube();

        // Zero out crafting array
        const size_t size = _craft.size();
        for (size_t i = 0; i < size; i++)
        {
            _craft[i] = craft_item();
        }

        // Copy non-empty items into crafting slots
        for (size_t i = 0; i < size; i++)
        {
            const size_t craft_index = begin + i;
            const item &it = _inv[craft_index];
            if (it.id() != item_id::EMPTY)
            {
                _craft[craft_size++] = craft_item(craft_index, it);
            }
        }

        // First recipe
        switch (craft_size)
        {
        case 1:
            return decay(index, mult);
        case 2:
            return std::make_pair(recipe_2(mult), item_id::EMPTY);
        case 3:
            return std::make_pair(recipe_3(mult), item_id::EMPTY);
        default:
            break;
        }

        // Return that we failed to craft
        return std::make_pair(false, item_id::EMPTY);
    }
    inline std::pair<bool, item_id> decay(const size_t index, const uint_fast8_t mult)
    {
        // Get the item
        const item &it = _inv[index];

        uint_fast8_t count = mult;
        const item_id it_id = it.id();
        switch (it_id)
        {
        case item_id::BLK_GRASS1:
        case item_id::BLK_GRASS2:
        case item_id::BLK_DIRT1:
        case item_id::BLK_DIRT2:
        case item_id::BLK_SAND1:
        case item_id::BLK_SAND2:
        case item_id::BLK_WOOD1:
        case item_id::BLK_WOOD2:
        case item_id::BLK_LEAF1:
        case item_id::BLK_LEAF2:
        case item_id::BLK_LEAF3:
        case item_id::BLK_LEAF4:
        case item_id::BLK_STONE1:
        case item_id::BLK_STONE2:
        case item_id::BLK_CLAY1:
        case item_id::BLK_CLAY2:
        case item_id::BLK_STONE3:
            return decay(index, it_id, item_id::CONS_ETHER, count, mult);
        case item_id::BLK_FE:
            return decay(index, item_id::BLK_FE, item_id::CAT_FE, count, mult);
        case item_id::BLK_MG:
            return decay(index, item_id::BLK_MG, item_id::CAT_MG, count, mult);
        case item_id::BLK_CU:
            return decay(index, item_id::BLK_CU, item_id::CAT_CU, count, mult);
        case item_id::BLK_NA:
            return decay(index, item_id::BLK_NA, item_id::CAT_NA, count, mult);
        case item_id::BLK_CA:
            return decay(index, item_id::BLK_CA, item_id::CAT_CA, count, mult);
        case item_id::BLK_CRYS_R:
            return decay(index, item_id::BLK_CRYS_R, item_id::SHARD_R, count, mult * 4);
        case item_id::BLK_CRYS_P:
            return decay(index, item_id::BLK_CRYS_P, item_id::SHARD_P, count, mult * 4);
        case item_id::BLK_CRYS_B:
            return decay(index, item_id::BLK_CRYS_B, item_id::SHARD_B, count, mult * 4);
        case item_id::BLK_CRYS_G:
            return decay(index, item_id::BLK_CRYS_G, item_id::SHARD_G, count, mult * 4);
        case item_id::BLK_K:
            return decay(index, item_id::BLK_K, item_id::CAT_K, count, mult);
        case item_id::POWD_BGUANO:
            return decay(index, item_id::POWD_BGUANO, item_id::POWD_KNO3, count, mult * 4);
        case item_id::BLK_TOM:
            return decay(index, item_id::BLK_TOM, item_id::CONS_TOM, count, mult * 4);
        case item_id::BLK_EGGP:
            return decay(index, item_id::BLK_EGGP, item_id::CONS_EGGP, count, mult * 4);
        case item_id::BLK_RED_PEP:
            return decay(index, item_id::BLK_RED_PEP, item_id::CONS_RED_PEP, count, mult * 4);
        case item_id::BLK_GR_PEP:
            return decay(index, item_id::BLK_GR_PEP, item_id::CONS_GR_PEP, count, mult * 4);
        case item_id::CAT_NH4:
            return decay(index, item_id::CAT_NH4, item_id::AN_NO3, count, mult);
        case item_id::CONS_EGGP:
            count = 1;
            return decay(index, item_id::CONS_EGGP, item_id::AN_CL, count, 1);
        case item_id::CONS_GR_PEP:
            count = 1;
            return decay(index, item_id::CONS_GR_PEP, item_id::AN_CL, count, 1);
        case item_id::CONS_RED_PEP:
            count = 1;
            return decay(index, item_id::CONS_RED_PEP, item_id::CAT_H, count, 1);
        case item_id::CONS_TOM:
            count = 1;
            return decay(index, item_id::CONS_TOM, item_id::CAT_H, count, 1);
        case item_id::CONS_BATTERY:
            count = 1;
            return decay(index, item_id::CONS_BATTERY, count);
        case item_id::CONS_OXYGEN:
            count = 1;
            return decay(index, item_id::CONS_OXYGEN, count);
        default:
            break;
        }

        return std::make_pair(false, it.id());
    }
    inline bool is_dirty() const
    {
        return _update.size() > 0;
    }
    inline void drop(const size_t index)
    {
        _inv[index].set_empty();

        // Store update index
        _update.emplace_back(index);
    }
    inline void fill(const std::vector<item> &inv, const unsigned player_level)
    {
        // Write inventory data into stream
        const size_t start = begin_store();
        const size_t end = end_cube();
        for (size_t i = start; i < end; i++)
        {
            _inv[i] = inv[i];
        }

        // Cache player level
        _player_level = player_level;
    }
    inline const std::string &get_name(const item_id id) const
    {
        return _inv_name[id_value(id)];
    }
    inline const std::string &get_info(const item_id id) const
    {
        return _inv_desc[id_value(id)];
    }
    inline const std::vector<ui_id> &get_updates() const
    {
        return _update;
    }
    inline void respawn(const bool hardcore)
    {
        if (hardcore)
        {
            // Wipe inventory
            std::fill(_inv.begin(), _inv.end(), item());

            // Create items in the store
            set_store();

            // Flag all dirty
            _update.resize(_inv.size());
            std::iota(_update.begin(), _update.end(), 0);
        }
        else
        {
            // Drop 5 random item slots
            for (size_t i = 0; i < _drop_count; i++)
            {
                // Calculate random index to drop
                const size_t index = _drop_item(_gen);
                if (_inv[index].type() != item_type::empty)
                {
                    // Drop item
                    _inv[index].set_empty();

                    // Set update index
                    _update.push_back(index);
                }
            }
        }
    }
    inline void set_player_level(const unsigned level)
    {
        _player_level = level;
    }
    inline void swap(const size_t one, const size_t two)
    {
        // If swapping with store
        if (one >= begin_store() && one < end_store())
        {
            if (_inv[two].id() == item_id::EMPTY)
            {
                _inv[two] = _inv[one];

                // Store update index
                _update.emplace_back(two);
            }
        }
        else if (two >= begin_store() && two < end_store())
        {
            if (_inv[one].id() == item_id::EMPTY)
            {
                _inv[one] = _inv[two];

                // Store update index
                _update.emplace_back(one);
            }
        }
        else if (!stack(one, two))
        {
            // Only swap items if failed to stack
            const item swap = _inv[one];
            _inv[one] = _inv[two];
            _inv[two] = swap;

            // Store update index
            _update.emplace_back(one);
            _update.emplace_back(two);
        }
    }
};
}

#endif
