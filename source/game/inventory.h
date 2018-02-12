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
        return inv_id(_id + 5);
    }
    inline inv_id store_index() const
    {
        return inv_id(_id + 13);
    }
    inline inv_id bg_key_index() const
    {
        return inv_id(_id + 13);
    }
    inline inv_id key_index() const
    {
        return inv_id(_id + 21);
    }
    inline inv_id bg_ex_index() const
    {
        return inv_id(_id + 21);
    }
    inline inv_id ex_index() const
    {
        return inv_id(_id + 45);
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
    static constexpr size_t _max_slots = 49;
    static constexpr size_t _max_strings = 111;
    std::array<item, _max_slots> _inv;
    std::array<std::string, _max_strings> _inv_desc;
    std::vector<inv_id> _update;

    inline bool decay(const uint8_t index, const uint8_t consume_id, const uint8_t add_id, uint8_t &count)
    {
        const uint8_t old_count = count;

        // Transform one item into another
        if (consume(index, consume_id, count))
        {
            // Reset count and add value
            count = 1;

            // If we couldn't add the item rollback consume
            if (!add(add_id, count))
            {
                _inv[index] = item(consume_id, old_count);
            }
        }

        return false;
    }
    inline void load_strings()
    {
        _inv_desc[0] = "Empty";
        _inv_desc[1] = "Charge Beam";
        _inv_desc[2] = "Missiles";
        _inv_desc[3] = "Grapple Hook";
        _inv_desc[4] = "Jet Pack";
        _inv_desc[5] = "Pending Scan";
        _inv_desc[6] = "Reserved";
        _inv_desc[7] = "Reserved";
        _inv_desc[8] = "Reserved";
        _inv_desc[9] = "Reserved";
        _inv_desc[10] = "Reserved";
        _inv_desc[11] = "Reserved";
        _inv_desc[12] = "Reserved";
        _inv_desc[13] = "Reserved";
        _inv_desc[14] = "Reserved";
        _inv_desc[15] = "Reserved";
        _inv_desc[16] = "Reserved";
        _inv_desc[17] = "Dense Grass";
        _inv_desc[18] = "Grass";
        _inv_desc[19] = "Fertile Soil";
        _inv_desc[20] = "Soil";
        _inv_desc[21] = "Yellow Sand";
        _inv_desc[22] = "White Sand";
        _inv_desc[23] = "Iron";
        _inv_desc[24] = "Silver";
        _inv_desc[25] = "Oak";
        _inv_desc[26] = "Pine";
        _inv_desc[27] = "Dark Foliage";
        _inv_desc[28] = "Light Vegetation";
        _inv_desc[29] = "Blooming Growth";
        _inv_desc[30] = "Flowery Growth";
        _inv_desc[31] = "Magnesium";
        _inv_desc[32] = "Gold";
        _inv_desc[33] = "Light Stone";
        _inv_desc[34] = "Dark Stone";
        _inv_desc[35] = "Light Clay";
        _inv_desc[36] = "Dark Clay";
        _inv_desc[37] = "Mossy Stone";
        _inv_desc[38] = "Copper";
        _inv_desc[39] = "Unstable Sodium";
        _inv_desc[40] = "Calcium";
        _inv_desc[41] = "Red Crystals";
        _inv_desc[42] = "Purple Crystals";
        _inv_desc[43] = "Blue Crystals";
        _inv_desc[44] = "Green Crystals";
        _inv_desc[45] = "???";
        _inv_desc[46] = "???";
        _inv_desc[47] = "Potassium";
        _inv_desc[48] = "???";
        _inv_desc[81] = "Red Crystal Shards";
        _inv_desc[82] = "Purple Crystal Shards";
        _inv_desc[83] = "Blue Crystal Shards";
        _inv_desc[84] = "Green Crystal Shards";
        _inv_desc[85] = "Ammonium [NH4+]";
        _inv_desc[86] = "Calcium [Ca+]";
        _inv_desc[87] = "Copper [Cu2+]";
        _inv_desc[88] = "Hydrogen [H+]";
        _inv_desc[89] = "Iron [Fe2+]";
        _inv_desc[90] = "Magnesium [Mg2+]";
        _inv_desc[91] = "Potassium [K+]";
        _inv_desc[92] = "Sodium [Na+]";
        _inv_desc[93] = "Chloride [Cl-]";
        _inv_desc[94] = "Nitrate [NO3-]";
        _inv_desc[95] = "Phosphate [(PO4)3-]";
        _inv_desc[96] = "Sulfate [(SO4)2-]";
        _inv_desc[97] = "Calcium Bar";
        _inv_desc[98] = "Copper Bar";
        _inv_desc[99] = "Iron Bar";
        _inv_desc[100] = "Magnesium Bar";
        _inv_desc[101] = "Potassium Bar";
        _inv_desc[102] = "Sodium Bar";
        _inv_desc[103] = "Gold Bar";
        _inv_desc[104] = "Silver Bar";
        _inv_desc[105] = "Hydrochloric Acid [HCl]";
        _inv_desc[106] = "Nitric Acid [HNO3]";
        _inv_desc[107] = "Phosphoric Acid [H3PO4]";
        _inv_desc[108] = "Sulfuric Acid [H2SO4]";
        _inv_desc[109] = "Sodium Battery";
        _inv_desc[110] = "Plasma Shield";
    }

  public:
    inventory() : _inv{}, _inv_desc{}
    {
        // Load inv` strings
        load_strings();

        // Reserve memory for update buffer
        _update.reserve(_max_slots);

        // Create items in the store
        _inv[begin_store()] = item(1, 1);
        _inv[begin_store() + 1] = item(2, 1);
        _inv[begin_store() + 2] = item(3, 1);
        _inv[begin_store() + 3] = item(4, 1);
        _inv[begin_store() + 4] = item(5, 1);
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
        for (size_t i = begin_key(); i < size; i++)
        {
            // Get the current item
            item &it = _inv[i];

            // If this slot empty?
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
            else if (it.id() == id)
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
        return 49;
    }
    inline static size_t size()
    {
        return _max_slots;
    }
    inline void clean()
    {
        _update.clear();
    }
    inline bool consume(const size_t index, const uint8_t id, uint8_t &count)
    {
        // Get the current item
        item &it = _inv[index];

        // Is this the item I want?
        if (it.id() == id && it.count() >= count)
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

            // Return that we consumed the resource
            return true;
        }

        return false;
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
    inline bool decay(const size_t index)
    {
        // Get the item
        const item &it = _inv[index];

        uint8_t count = 1;
        const item_id it_id = it.id_enum();
        switch (it_id)
        {
        case item_id::BLK_FE:
            return decay(index, id_value(item_id::BLK_FE), id_value(item_id::CAT_FE), count);
        case item_id::BLK_SI:
            return decay(index, id_value(item_id::BLK_SI), id_value(item_id::BAR_SI), count);
        case item_id::BLK_MG:
            return decay(index, id_value(item_id::BLK_MG), id_value(item_id::CAT_MG), count);
        case item_id::BLK_AU:
            return decay(index, id_value(item_id::BLK_AU), id_value(item_id::BAR_AU), count);
        case item_id::BLK_CU:
            return decay(index, id_value(item_id::BLK_CU), id_value(item_id::CAT_CU), count);
        case item_id::BLK_NA:
            return decay(index, id_value(item_id::BLK_NA), id_value(item_id::CAT_NA), count);
        case item_id::BLK_CA:
            return decay(index, id_value(item_id::BLK_CA), id_value(item_id::CAT_CA), count);
        case item_id::BLK_CRYS_R:
            return decay(index, id_value(item_id::BLK_CRYS_R), id_value(item_id::SHARD_R), count);
        case item_id::BLK_CRYS_P:
            return decay(index, id_value(item_id::BLK_CRYS_P), id_value(item_id::SHARD_P), count);
        case item_id::BLK_CRYS_B:
            return decay(index, id_value(item_id::BLK_CRYS_B), id_value(item_id::SHARD_B), count);
        case item_id::BLK_CRYS_G:
            return decay(index, id_value(item_id::BLK_CRYS_G), id_value(item_id::SHARD_G), count);
        case item_id::BLK_K:
            return decay(index, id_value(item_id::BLK_K), id_value(item_id::CAT_K), count);
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
        const size_t end = end_extend();
        for (size_t i = start; i < end; i++)
        {
            _inv[i] = inv[i];
        }
    }
    inline const std::string &get_string(const item it) const
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
        _inv[begin_store() + 1] = item(2, 1);
        _inv[begin_store() + 2] = item(3, 1);
        _inv[begin_store() + 3] = item(4, 1);
        _inv[begin_store() + 4] = item(5, 1);

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
        // Only swap if failed to stack
        if (!stack(one, two))
        {
            // Swap items
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
