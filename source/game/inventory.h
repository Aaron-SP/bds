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
#include <vector>

namespace game
{

enum inv_type
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

class item
{
  private:
    uint8_t _id;
    uint8_t _count;

  public:
    item() : _id(0), _count(0) {}
    item(const uint8_t id, const uint8_t count)
        : _id(id), _count(count) {}

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
    inline void stack(uint8_t &count)
    {
        // Calculate sum of items
        const uint16_t sum = _count + count;

        // Calculate left over items
        count = (sum / 255) * (sum % 255);

        // Set the stack item count
        _count = sum - count;
    }
};

class inventory
{
  private:
    static constexpr size_t _max_slots = 49;
    static constexpr size_t _max_strings = 37;
    std::array<item, _max_slots> _inv;
    std::array<std::string, _max_strings> _inv_desc;
    std::vector<inv_id> _update;

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
        _inv_desc[9] = "Dense Grass";
        _inv_desc[10] = "Grass";
        _inv_desc[11] = "Fertile Soil";
        _inv_desc[12] = "Soil";
        _inv_desc[13] = "Yellow Sand";
        _inv_desc[14] = "White Sand";
        _inv_desc[15] = "???";
        _inv_desc[16] = "???";
        _inv_desc[17] = "Oak";
        _inv_desc[18] = "Pine";
        _inv_desc[19] = "Dark Foliage";
        _inv_desc[20] = "Light Vegetation";
        _inv_desc[21] = "Blooming Growth";
        _inv_desc[22] = "Flowery Growth";
        _inv_desc[23] = "???";
        _inv_desc[24] = "???";
        _inv_desc[25] = "Light Stone";
        _inv_desc[26] = "Dark Stone";
        _inv_desc[27] = "Light Clay";
        _inv_desc[28] = "Dark Clay";
        _inv_desc[29] = "Mossy Stone";
        _inv_desc[30] = "Unstable Sodium";
        _inv_desc[31] = "???";
        _inv_desc[32] = "???";
        _inv_desc[33] = "Red Crystals";
        _inv_desc[34] = "Purple Crystals";
        _inv_desc[35] = "Blue Crystals";
        _inv_desc[36] = "Green Crystals";
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
    inline bool consume(const uint8_t id, uint8_t &count)
    {
        // Search for item of same id in slots
        const size_t size = _inv.size();
        for (size_t i = begin_key(); i < size; i++)
        {
            // Get the current item
            item &it = _inv[i];

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
                _update.emplace_back(i);

                // Return that we consumed the resource
                return true;
            }
        }

        // Return that we failed to consume
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
        return id - 9;
    }
    inline static constexpr uint8_t id_from_atlas(const int8_t id)
    {
        // Convert from atlas to inventory id
        return id + 9;
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
    inline void swap(const size_t one, const size_t two)
    {
        // Swap items
        const item swap = _inv[one];
        _inv[one] = _inv[two];
        _inv[two] = swap;

        // Store update index
        _update.emplace_back(one);
        _update.emplace_back(two);
    }
};
}

#endif
