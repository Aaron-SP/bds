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

#include <vector>

namespace game
{

class inv_id
{
  private:
    uint8_t _id;

  public:
    inv_id() : _id(0) {}
    inv_id(const uint8_t id) : _id(id) {}
    inline uint8_t id() const
    {
        return _id;
    }
    inline size_t index() const
    {
        return _id;
    }
    inline inv_id bg_key_index() const
    {
        return inv_id(_id + 5);
    }
    inline inv_id key_index() const
    {
        return inv_id(_id + 13);
    }
    inline inv_id bg_inv_index() const
    {
        return inv_id(_id + 13);
    }
    inline inv_id inv_index() const
    {
        return inv_id(_id + 37);
    }
    inline size_t row() const
    {
        return 0;
    }
    inline size_t ext_row() const
    {
        return 1 + (_id / 8);
    }
    inline size_t col() const
    {
        return _id % 8;
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
    static constexpr size_t _max_slots = 32;
    std::vector<item> _inv;
    std::vector<inv_id> _update;

  public:
    inventory()
        : _inv(_max_slots)
    {
        // Reserve memory for update buffer
        _update.reserve(_max_slots);

        // Add a default beam
        _inv[0] = item(1, 1);
    }
    inline const item &operator[](const size_t index) const
    {
        return _inv[index];
    }
    inline bool add(const uint8_t id, uint8_t &count)
    {
        // Search for item of same id in slots
        const size_t size = _inv.size();
        for (size_t i = 0; i < size; i++)
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
    inline void clean()
    {
        _update.clear();
    }
    inline bool consume(const uint8_t id, uint8_t &count)
    {
        // Search for item of same id in slots
        const size_t size = _inv.size();
        for (size_t i = 0; i < size; i++)
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
    const std::vector<inv_id> &get_updates() const
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

        // Add a default beam
        _inv[0] = item(1, 1);

        // Flag all dirty
        _update.resize(_inv.size());
        std::iota(_update.begin(), _update.end(), 0);
    }
    inline size_t size() const
    {
        return _inv.size();
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
