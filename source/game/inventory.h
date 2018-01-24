/* Copyright [2013-2018] [Aaron Springstroh, Minimal Graphics Library]

This file is part of the Fractex.

Fractex is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Fractex is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Fractex.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __INVENTORY__
#define __INVENTORY__

#include <vector>

namespace game
{

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
    bool _dirty;

  public:
    inventory()
        : _inv(_max_slots), _dirty(false)
    {
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
        for (auto &it : _inv)
        {
            // If this slot empty?
            if (it.id() == 0)
            {
                it = item(id, count);

                // Flag dirty
                _dirty = true;

                // Signal that we picked it up
                count = 0;

                // Return added
                return true;
            }
            else if (it.id() == id)
            {
                // Try to stack the item
                it.stack(count);

                // Flag dirty
                _dirty = true;

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
        _dirty = false;
    }
    inline bool consume(const uint8_t id, uint8_t &count)
    {
        // Search for item to consume
        for (auto &it : _inv)
        {
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

                // Flag dirty
                _dirty = true;

                // Return that we consumed the resource
                return true;
            }
        }

        // Return that we failed to consume
        return false;
    }
    inline bool dirty() const
    {
        return _dirty;
    }
    inline void drop(const size_t index)
    {
        _inv[index].drop();

        // Flag dirty
        _dirty = true;
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

        // Flag dirty
        _dirty = true;
    }
    inline void swap(const size_t one, const size_t two)
    {
        // Swap items
        const item swap = _inv[one];
        _inv[one] = _inv[two];
        _inv[two] = swap;

        // Flag dirty
        _dirty = true;
    }
};
}

#endif
