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
#ifndef __SWATCH__
#define __SWATCH__

#include <array>
#include <game/id.h>

namespace game
{

class swatch
{
  private:
    static constexpr size_t _scale = 6;
    static constexpr size_t _size = _scale * _scale * _scale;
    std::vector<block_id> _b;
    min::vec3<unsigned> _length;
    min::vec3<int> _offset;

    inline size_t grid_key_pack(const std::tuple<size_t, size_t, size_t> &t) const
    {
        return min::vec3<float>::grid_key(t, _scale);
    }
    inline std::tuple<size_t, size_t, size_t> grid_key_unpack(const size_t key) const
    {
        return min::vec3<float>::grid_index(key, _scale);
    }

  public:
    swatch() : _b(_size, block_id::EMPTY) {}
    const min::vec3<unsigned> &get_length() const
    {
        return _length;
    }
    const min::vec3<int> &get_offset() const
    {
        return _offset;
    }
    block_id get(const size_t i, const size_t j, const size_t k) const
    {
        return _b[grid_key_pack(std::make_tuple(i, j, k))];
    }
    void reset()
    {
        for (size_t i = 0; i < _size; i++)
        {
            _b[i] = block_id::EMPTY;
        }
    }
    void set_length(const min::vec3<unsigned> &length)
    {
        _length = length;
    }
    void set_offset(const min::vec3<int> &offset)
    {
        _offset = offset;
    }
    void set(const size_t i, const size_t j, const size_t k, const block_id atlas)
    {
        _b[grid_key_pack(std::make_tuple(i, j, k))] = atlas;
    }
};
}

#endif
