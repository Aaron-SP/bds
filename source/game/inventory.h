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
#include <vector>

namespace game
{

enum class inv_type
{
    store,
    key,
    extend,
    cube
};

class inv_id
{
  private:
    uint8_t _id;

  public:
    inv_id() : _id(0) {}
    inv_id(const uint8_t id) : _id(id) {}
    inline bool operator==(const inv_id other) const
    {
        return _id == other.id();
    }
    inline bool operator!=(const inv_id other) const
    {
        return _id != other.id();
    }
    inline size_t row3() const
    {
        return (_id - 40) / 3;
    }
    inline size_t col3() const
    {
        return (_id - 40) % 3;
    }
    inline size_t row8() const
    {
        return _id / 8;
    }
    inline size_t col8() const
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
    inline inv_id bg_store_index() const
    {
        return inv_id(_id + 6);
    }
    inline inv_id store_index() const
    {
        return inv_id(_id + 14);
    }
    inline inv_id bg_key_index() const
    {
        return inv_id(_id + 14);
    }
    inline inv_id key_index() const
    {
        return inv_id(_id + 22);
    }
    inline inv_id bg_ex_index() const
    {
        return inv_id(_id + 22);
    }
    inline inv_id ex_index() const
    {
        return inv_id(_id + 46);
    }
    inline inv_id bg_cube_index() const
    {
        return inv_id(_id + 47);
    }
    inline inv_id cube_index() const
    {
        return inv_id(_id + 56);
    }
    inline inv_id to_key() const
    {
        return inv_id(_id + 8);
    }
    inline inv_type type() const
    {
        if (_id >= 40)
        {
            // Cube type
            return inv_type::cube;
        }
        if (_id >= 16)
        {
            // Extended type
            return inv_type::extend;
        }
        else if (_id >= 8)
        {
            // Key type
            return inv_type::key;
        }

        // Store type
        return inv_type::store;
    }
};

enum class item_type
{
    empty,
    skill,
    block,
    item
};

class item
{
  private:
    uint8_t _id;
    uint8_t _count;

  public:
    item() : _id(0), _count(0) {}
    item(const uint8_t id, const uint8_t count)
        : _id(id), _count(count) {}

    inline uint8_t to_block_id() const
    {
        return _id - 17;
    }
    inline uint8_t to_item_id() const
    {
        return _id - 81;
    }
    inline void consume(const uint8_t count)
    {
        _count -= count;
    }
    inline uint8_t count() const
    {
        return _count;
    }
    inline void drop()
    {
        _id = 0;
        _count = 0;
    }
    inline void empty()
    {
        _id = 0;
    }
    inline uint8_t id() const
    {
        return _id;
    }
    inline item_id id_enum() const
    {
        return static_cast<item_id>(_id);
    }
    inline void stack(uint8_t &count)
    {
        // Calculate sum of items
        const uint16_t sum = _count + count;

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
        const size_t index() const
        {
            return _index;
        }
        const item &get_item() const
        {
            return _item;
        }
        bool operator<(const craft_item &other) const
        {
            return (_item.id() < other.get_item().id());
        }
    };

    static constexpr size_t _max_slots = 49;
    static constexpr size_t _max_strings = 121;
    static constexpr size_t _cube_size = 9;
    std::array<item, _max_slots> _inv;
    std::vector<std::string> _inv_name;
    std::vector<std::string> _inv_desc;
    std::vector<inv_id> _update;
    std::array<craft_item, _cube_size> _craft;

    inline bool decay(const uint8_t index, const uint8_t consume_id, const uint8_t add_id, uint8_t &count)
    {
        const uint8_t old_count = count;

        // Did we decay anything?
        bool status = false;

        // Transform one item into another
        if (consume(index, consume_id, count))
        {
            // Reset count and add value
            count = 4;

            // If we couldn't add the item rollback consume
            if (!add(add_id, count))
            {
                _inv[index] = item(consume_id, old_count);
            }
            else
            {
                status = true;
            }
        }

        return status;
    }
    inline void consume(const size_t index, item &it, uint8_t &count)
    {
        // Consume the inventory count
        it.consume(count);

        // Set the remaining count
        count = it.count();

        // If count is zero set empty ID
        if (it.count() == 0)
        {
            it.empty();
        }

        // Store update index
        _update.emplace_back(index);
    }
    inline bool consume2(
        const size_t index_1, const uint8_t id_1, uint8_t &count_1,
        const size_t index_2, const uint8_t id_2, uint8_t &count_2)
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
            consume(index_1, it_1, count_1);
            consume(index_2, it_2, count_2);

            // Return that we consumed the resource
            return true;
        }

        return false;
    }
    inline void load_strings()
    {
        _inv_name[0] = "Empty";
        _inv_name[1] = "Charge Beam";
        _inv_desc[1] = "An energy weapon_used for modifying_terrain";
        _inv_name[2] = "Missile Launcher";
        _inv_desc[2] = "An expensive_offensive projectile_weapon";
        _inv_name[3] = "Grappling Hook";
        _inv_desc[3] = "An energy beam_used for swinging_in the air";
        _inv_name[4] = "Jet Pack";
        _inv_desc[4] = "Transforms energy_into vertical thrust";
        _inv_name[5] = "Pending Scan";
        _inv_desc[5] = "Scan a block to_retrieve the block_type";
        _inv_name[6] = "Grenade Launcher";
        _inv_desc[6] = "An cheap offensive_projectile weapon";
        _inv_name[7] = "Reserved";
        _inv_name[8] = "Reserved";
        _inv_name[9] = "Reserved";
        _inv_name[10] = "Reserved";
        _inv_name[11] = "Reserved";
        _inv_name[12] = "Reserved";
        _inv_name[13] = "Reserved";
        _inv_name[14] = "Reserved";
        _inv_name[15] = "Reserved";
        _inv_name[16] = "Reserved";
        _inv_name[17] = "Dense Grass";
        _inv_name[18] = "Grass";
        _inv_name[19] = "Fertile Soil";
        _inv_name[20] = "Soil";
        _inv_name[21] = "Yellow Sand";
        _inv_name[22] = "White Sand";
        _inv_name[23] = "Iron";
        _inv_name[24] = "Silver";
        _inv_name[25] = "Oak";
        _inv_name[26] = "Pine";
        _inv_name[27] = "Dark Foliage";
        _inv_name[28] = "Light Vegetation";
        _inv_name[29] = "Blooming Growth";
        _inv_name[30] = "Flowery Growth";
        _inv_name[31] = "Magnesium";
        _inv_name[32] = "Gold";
        _inv_name[33] = "Light Stone";
        _inv_name[34] = "Dark Stone";
        _inv_name[35] = "Light Clay";
        _inv_name[36] = "Dark Clay";
        _inv_name[37] = "Mossy Stone";
        _inv_name[38] = "Copper";
        _inv_name[39] = "Unstable Sodium";
        _inv_name[40] = "Calcium";
        _inv_name[41] = "Red Crystals";
        _inv_desc[41] = "Right click to harvest";
        _inv_name[42] = "Purple Crystals";
        _inv_desc[42] = "Right click to harvest";
        _inv_name[43] = "Blue Crystals";
        _inv_desc[43] = "Right click to harvest";
        _inv_name[44] = "Green Crystals";
        _inv_desc[44] = "Right click to harvest";
        _inv_name[45] = "???";
        _inv_name[46] = "???";
        _inv_name[47] = "Potassium";
        _inv_name[48] = "???";
        _inv_name[49] = "Tomatoes";
        _inv_desc[49] = "Right click to harvest";
        _inv_name[50] = "Eggplant";
        _inv_desc[50] = "Right click to harvest";
        _inv_name[51] = "Red Peppers";
        _inv_desc[51] = "Right click to harvest";
        _inv_name[52] = "Green Peppers";
        _inv_desc[52] = "Right click to harvest";
        _inv_name[81] = "Red Crystal Shards";
        _inv_desc[81] = "Transforms raw_compounds into_processed elements";
        _inv_name[82] = "Purple Crystal Shards";
        _inv_name[83] = "Blue Crystal Shards";
        _inv_desc[83] = "Transforms ionic_compounds into_stable elements";
        _inv_name[84] = "Green Crystal Shards";
        _inv_name[85] = "Ammonium [NH4+]";
        _inv_name[86] = "Calcium [Ca+]";
        _inv_name[87] = "Copper [Cu2+]";
        _inv_name[88] = "Hydrogen [H+]";
        _inv_name[89] = "Iron [Fe2+]";
        _inv_name[90] = "Magnesium [Mg2+]";
        _inv_name[91] = "Potassium [K+]";
        _inv_name[92] = "Sodium [Na+]";
        _inv_name[93] = "Chloride [Cl-]";
        _inv_name[94] = "Nitrate [NO3-]";
        _inv_name[95] = "Phosphate [(PO4)3-]";
        _inv_name[96] = "Sulfate [(SO4)2-]";
        _inv_name[97] = "Calcium Bar";
        _inv_name[98] = "Copper Bar";
        _inv_name[99] = "Iron Bar";
        _inv_name[100] = "Magnesium Bar";
        _inv_name[101] = "Potassium Bar";
        _inv_name[102] = "Sodium Bar";
        _inv_name[103] = "Gold Bar";
        _inv_name[104] = "Silver Bar";
        _inv_name[105] = "Hydrochloric Acid [HCl]";
        _inv_name[106] = "Nitric Acid [HNO3]";
        _inv_name[107] = "Phosphoric Acid [H3PO4]";
        _inv_name[108] = "Sulfuric Acid [H2SO4]";
        _inv_name[109] = "Potassium Nitrate";
        _inv_name[110] = "Rust";
        _inv_name[111] = "Sulphur";
        _inv_name[112] = "Salt";
        _inv_name[113] = "Charcoal";
        _inv_name[114] = "Red Phosphorus";
        _inv_name[115] = "Urea";
        _inv_name[116] = "Bat Guano";
        _inv_desc[116] = "Right click to harvest";
        _inv_name[117] = "Tomato";
        _inv_desc[117] = "It's tender and juicy";
        _inv_name[118] = "Eggplant";
        _inv_desc[118] = "It's tender and juicy";
        _inv_name[119] = "Red Pepper";
        _inv_desc[119] = "It's tender and juicy";
        _inv_name[120] = "Green Pepper";
        _inv_desc[120] = "It's tender and juicy";
    }

  public:
    inventory() : _inv{}, _inv_name(_max_strings), _inv_desc(_max_strings), _craft{}
    {
        // Load inv` strings
        load_strings();

        // Reserve memory for update buffer
        _update.reserve(_max_slots);

        // Create items in the store
        _inv[begin_store()] = item(1, 1);
        _inv[begin_store() + 1] = item(3, 1);
        _inv[begin_store() + 2] = item(4, 1);
        _inv[begin_store() + 3] = item(5, 1);
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
    inline bool add(const uint8_t id, uint8_t &count)
    {
        // Search for item of same id in slots
        const size_t size = _inv.size();

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
            if (it.id() == 0)
            {
                it = item(id, count);

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
    inline static size_t begin_store()
    {
        return 0;
    }
    inline static size_t end_store()
    {
        return 8;
    }
    inline static size_t begin_key()
    {
        return 8;
    }
    inline static size_t end_key()
    {
        return 16;
    }
    inline static size_t begin_extend()
    {
        return 16;
    }
    inline static size_t end_extend()
    {
        return 40;
    }
    inline static size_t begin_cube()
    {
        return 40;
    }
    inline static size_t end_cube()
    {
        return begin_cube() + _cube_size;
    }
    inline static size_t size()
    {
        return _max_slots;
    }
    inline void clean()
    {
        _update.clear();
    }
    inline bool consume(const uint8_t id, uint8_t &count)
    {
        // Search for item of same id in slots
        const size_t size = _inv.size();
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
    inline bool consume(const size_t index, const uint8_t id, uint8_t &count)
    {
        // Get the current item
        item &it = _inv[index];

        // Is this the item I want?
        if (it.id() == id && it.count() >= count)
        {
            consume(index, it, count);

            // Return that we consumed the resource
            return true;
        }

        return false;
    }
    inline bool recipe_2()
    {
        // Sort the crafting array by ID
        std::sort(_craft.begin(), _craft.begin() + 2, std::less<craft_item>());

        // Get indices in sorted order
        const size_t lower = _craft[0].index();
        const size_t higher = _craft[1].index();
        uint8_t low_count = 4;
        uint8_t high_count = 4;

        // Try to craft a recipe
        if (consume2(lower, id_value(item_id::BLK_FE), low_count, higher, id_value(item_id::CAT_H), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(item_id::POWD_RUST), add_count);
        }
        else if (consume2(lower, id_value(item_id::POWD_KNO3), low_count, higher, id_value(item_id::POWD_CHARCOAL), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(skill_id::GRENADE), add_count);
        }
        else if (consume2(lower, id_value(item_id::CAT_K), low_count, higher, id_value(item_id::AN_NO3), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(item_id::POWD_KNO3), add_count);
        }
        else if (consume2(lower, id_value(item_id::CAT_NA), low_count, higher, id_value(item_id::AN_CL), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(item_id::POWD_SALT), add_count);
        }

        // Blue shard recipes
        else if (consume2(lower, id_value(item_id::SHARD_B), low_count, higher, id_value(item_id::CAT_NH4), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(item_id::POWD_UREA), add_count);
        }
        else if (consume2(lower, id_value(item_id::SHARD_B), low_count, higher, id_value(item_id::AN_NO3), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(item_id::POWD_UREA), add_count);
        }
        else if (consume2(lower, id_value(item_id::SHARD_B), low_count, higher, id_value(item_id::CAT_CA), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(item_id::BLK_CA), add_count);
        }
        else if (consume2(lower, id_value(item_id::SHARD_B), low_count, higher, id_value(item_id::CAT_CU), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(item_id::BLK_CU), add_count);
        }
        else if (consume2(lower, id_value(item_id::SHARD_B), low_count, higher, id_value(item_id::CAT_FE), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(item_id::BLK_FE), add_count);
        }
        else if (consume2(lower, id_value(item_id::SHARD_B), low_count, higher, id_value(item_id::CAT_MG), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(item_id::BLK_MG), add_count);
        }
        else if (consume2(lower, id_value(item_id::SHARD_B), low_count, higher, id_value(item_id::CAT_K), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(item_id::BLK_K), add_count);
        }
        else if (consume2(lower, id_value(item_id::SHARD_B), low_count, higher, id_value(item_id::CAT_NA), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(item_id::BLK_NA), add_count);
        }
        else if (consume2(lower, id_value(item_id::SHARD_B), low_count, higher, id_value(item_id::AN_PHOS), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(item_id::POWD_RED_PHOS), add_count);
        }
        else if (consume2(lower, id_value(item_id::SHARD_B), low_count, higher, id_value(item_id::AN_SULPH), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(item_id::POWD_SULPHUR), add_count);
        }

        // Red shard recipes
        else if (consume2(lower, id_value(item_id::BLK_WOOD1), low_count, higher, id_value(item_id::SHARD_R), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(item_id::POWD_CHARCOAL), add_count);
        }
        else if (consume2(lower, id_value(item_id::BLK_WOOD2), low_count, higher, id_value(item_id::SHARD_R), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(item_id::POWD_CHARCOAL), add_count);
        }
        else if (consume2(lower, id_value(item_id::BLK_CA), low_count, higher, id_value(item_id::SHARD_R), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(item_id::BAR_CA), add_count);
        }
        else if (consume2(lower, id_value(item_id::BLK_CU), low_count, higher, id_value(item_id::SHARD_R), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(item_id::BAR_CU), add_count);
        }
        else if (consume2(lower, id_value(item_id::BLK_FE), low_count, higher, id_value(item_id::SHARD_R), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(item_id::BAR_FE), add_count);
        }
        else if (consume2(lower, id_value(item_id::BLK_MG), low_count, higher, id_value(item_id::SHARD_R), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(item_id::BAR_MG), add_count);
        }
        else if (consume2(lower, id_value(item_id::BLK_K), low_count, higher, id_value(item_id::SHARD_R), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(item_id::BAR_K), add_count);
        }
        else if (consume2(lower, id_value(item_id::BLK_NA), low_count, higher, id_value(item_id::SHARD_R), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(item_id::BAR_NA), add_count);
        }
        else if (consume2(lower, id_value(item_id::BLK_AU), low_count, higher, id_value(item_id::SHARD_R), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(item_id::BAR_AU), add_count);
        }
        else if (consume2(lower, id_value(item_id::BLK_SI), low_count, higher, id_value(item_id::SHARD_R), high_count))
        {
            uint8_t add_count = 4;
            return add(id_value(item_id::BAR_SI), add_count);
        }

        // Missiles
        else if (consume2(lower, id_value(item_id::BAR_FE), low_count, higher, id_value(item_id::BAR_NA), high_count))
        {
            uint8_t add_count = 32;
            return add(id_value(skill_id::MISSILE), add_count);
        }

        // Failed to craft item
        return false;
    }
    inline bool craft(const size_t hover_index)
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
            if (it.id() != 0)
            {
                _craft[craft_size++] = craft_item(craft_index, it);
            }
        }

        // First recipe
        switch (craft_size)
        {
        case 1:
            return decay(hover_index);
        case 2:
            return recipe_2();
            break;
        }

        // Return that we failed to craft
        return false;
    }
    inline bool decay(const size_t hover_index)
    {
        // Get the item
        const item &it = _inv[hover_index];

        uint8_t count = 1;
        const item_id it_id = it.id_enum();
        switch (it_id)
        {
        case item_id::BLK_FE:
            return decay(hover_index, id_value(item_id::BLK_FE), id_value(item_id::CAT_FE), count);
        case item_id::BLK_MG:
            return decay(hover_index, id_value(item_id::BLK_MG), id_value(item_id::CAT_MG), count);
        case item_id::BLK_CU:
            return decay(hover_index, id_value(item_id::BLK_CU), id_value(item_id::CAT_CU), count);
        case item_id::BLK_NA:
            return decay(hover_index, id_value(item_id::BLK_NA), id_value(item_id::CAT_NA), count);
        case item_id::BLK_CA:
            return decay(hover_index, id_value(item_id::BLK_CA), id_value(item_id::CAT_CA), count);
        case item_id::BLK_CRYS_R:
            return decay(hover_index, id_value(item_id::BLK_CRYS_R), id_value(item_id::SHARD_R), count);
        case item_id::BLK_CRYS_P:
            return decay(hover_index, id_value(item_id::BLK_CRYS_P), id_value(item_id::SHARD_P), count);
        case item_id::BLK_CRYS_B:
            return decay(hover_index, id_value(item_id::BLK_CRYS_B), id_value(item_id::SHARD_B), count);
        case item_id::BLK_CRYS_G:
            return decay(hover_index, id_value(item_id::BLK_CRYS_G), id_value(item_id::SHARD_G), count);
        case item_id::BLK_K:
            return decay(hover_index, id_value(item_id::BLK_K), id_value(item_id::CAT_K), count);
        case item_id::POWD_GUANO:
            return decay(hover_index, id_value(item_id::POWD_GUANO), id_value(item_id::POWD_KNO3), count);
        case item_id::BLK_TOM:
            return decay(hover_index, id_value(item_id::BLK_TOM), id_value(item_id::FOOD_TOM), count);
        case item_id::BLK_EGGP:
            return decay(hover_index, id_value(item_id::BLK_EGGP), id_value(item_id::FOOD_EGGP), count);
        case item_id::BLK_RED_PEP:
            return decay(hover_index, id_value(item_id::BLK_RED_PEP), id_value(item_id::FOOD_RED_PEP), count);
        case item_id::BLK_GR_PEP:
            return decay(hover_index, id_value(item_id::BLK_GR_PEP), id_value(item_id::FOOD_GR_PEP), count);
        default:
            return false;
        }

        return false;
    }
    inline bool dirty() const
    {
        return _update.size() > 0;
    }
    inline void drop(const size_t index)
    {
        _inv[index].drop();

        // Store update index
        _update.emplace_back(index);
    }
    inline void fill(const std::vector<item> &inv)
    {
        // Write inventory data into stream
        const size_t start = begin_key();
        const size_t end = end_cube();
        for (size_t i = start; i < end; i++)
        {
            _inv[i] = inv[i];
        }
    }
    inline const std::string &get_name(const item &it) const
    {
        return _inv_name[it.id()];
    }
    inline const std::string &get_info(const item &it) const
    {
        return _inv_desc[it.id()];
    }
    inline const std::vector<inv_id> &get_updates() const
    {
        return _update;
    }
    inline static constexpr int8_t id_to_atlas(const uint8_t id)
    {
        // Convert inventory id to cube atlas
        return id - 17;
    }
    inline static constexpr uint8_t id_from_atlas(const int8_t id)
    {
        // Convert from atlas to inventory id
        return id + 17;
    }
    inline static constexpr int8_t id_to_item(const uint8_t id)
    {
        // Convert inventory id to cube atlas
        return id - 81;
    }
    inline void respawn()
    {
        std::fill(_inv.begin(), _inv.end(), item());

        // Create items in the store
        _inv[begin_store()] = item(1, 1);
        _inv[begin_store() + 1] = item(3, 1);
        _inv[begin_store() + 2] = item(4, 1);
        _inv[begin_store() + 3] = item(5, 1);

        // Flag all dirty
        _update.resize(_inv.size());
        std::iota(_update.begin(), _update.end(), 0);
    }
    inline bool stack(const size_t one, const size_t two)
    {
        // Try to stack items if same type
        const uint8_t id = _inv[one].id();
        if (id == _inv[two].id())
        {
            uint8_t count = _inv[one].count();
            _inv[two].stack(count);

            // If items are left
            if (count > 0)
            {
                _inv[one] = item(id, count);
            }
            else
            {
                // Discard empty item
                _inv[one].drop();
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
    inline void swap(const size_t one, const size_t two)
    {
        // If swapping with store
        if (one >= begin_store() && one < end_store())
        {
            if (_inv[two].id() == 0)
            {
                _inv[two] = _inv[one];

                // Store update index
                _update.emplace_back(two);
            }
        }
        else if (two >= begin_store() && two < end_store())
        {
            if (_inv[one].id() == 0)
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
