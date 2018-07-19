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
#ifndef __FILE_UTILS__
#define __FILE_UTILS__

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#ifdef DATA_PATH
#define DATA_FILE       \
    TOSTRING(DATA_PATH) \
    "/data.sky"
#define DATA_FONTS      \
    TOSTRING(DATA_PATH) \
    "/fonts/"
#else
#define DATA_FILE "data/data.sky"
#define DATA_FONTS "data/fonts/"
#endif

#ifdef SAVE_PATH
#define SAVE_KEYMAP     \
    TOSTRING(SAVE_PATH) \
    "/save/keymap."
#define SAVE_STATE      \
    TOSTRING(SAVE_PATH) \
    "/save/state."
#define SAVE_WORLD      \
    TOSTRING(SAVE_PATH) \
    "/save/world."
#else
#define SAVE_KEYMAP "save/keymap."
#define SAVE_STATE "save/state."
#define SAVE_WORLD "save/world."
#endif

namespace game
{

inline bool erase_file(const std::string &file_name)
{
    // Erase file
    const int ret = std::remove(file_name.c_str());
    const bool saved = (ret == 0);
    if (!saved)
    {
        std::cout << "file: could not erase file '" << file_name << "'" << std::endl;
    }
    else
    {
        std::cout << "file: erased file '" << file_name << "'" << std::endl;
    }

    return saved;
}
inline bool erase_save(const size_t index)
{
    const bool k = erase_file(SAVE_KEYMAP + std::to_string(index));
    const bool s = erase_file(SAVE_STATE + std::to_string(index));
    const bool w = erase_file(SAVE_WORLD + std::to_string(index));

    // Did we delete any saves?
    return k || s || w;
}
inline bool exists_file(const std::string &file_name)
{
    std::ifstream f(file_name);
    return f.good();
}
inline void load_file(const std::string &file_name, std::vector<uint8_t> &stream)
{
    // read bytes from file
    std::ifstream file(file_name, std::ios::in | std::ios::binary | std::ios::ate);
    if (file.is_open())
    {
        // Get the size of the file
        std::streampos size = file.tellg();

        // Reserve space for the bytes
        stream.resize(size, 0);

        // Adjust file pointer to beginning
        file.seekg(0, std::ios::beg);

        // Read bytes and close the file
        file.read(reinterpret_cast<char *>(&stream[0]), size);
        file.close();
    }
    else
    {
        std::cout << "file: could not load file '" << file_name << "'" << std::endl;
    }
}
inline void save_file(const std::string &file_name, const std::vector<uint8_t> &stream)
{
    // Save bytes to file
    std::ofstream file(file_name, std::ios::out | std::ios::binary);
    if (file.is_open())
    {
        file.write(reinterpret_cast<const char *>(&stream[0]), stream.size());
        file.close();
    }
    else
    {
        std::cout << "file: could not save file '" << file_name << "'" << std::endl;
    }
}
}

#endif
