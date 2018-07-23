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
#ifndef _BDS_FILE_UTILS_BDS_
#define _BDS_FILE_UTILS_BDS_

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace game
{

class file
{
  private:
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
#define HOME_KEYMAP "/.bds-game/save/keymap."
#define HOME_STATE "/.bds-game/save/state."
#define HOME_WORLD "/.bds-game/save/world."
    static std::ostringstream _ss;
    static inline void clear_stream()
    {
        _ss.clear();
        _ss.str(std::string());
    }

  public:
    static inline std::string get_keymap_file(const size_t save_slot)
    {
        clear_stream();
        const char *home = std::getenv("HOME");
        if (home == nullptr)
        {
            _ss << SAVE_KEYMAP;
            _ss << save_slot;
        }
        else
        {
            _ss << home;
            _ss << HOME_KEYMAP;
            _ss << save_slot;
        }
        return _ss.str();
    }
    static inline std::string get_state_file(const size_t save_slot)
    {
        clear_stream();
        const char *home = std::getenv("HOME");
        if (home == nullptr)
        {
            _ss << SAVE_STATE;
            _ss << save_slot;
        }
        else
        {
            _ss << home;
            _ss << HOME_STATE;
            _ss << save_slot;
        }
        return _ss.str();
    }
    static inline std::string get_world_file(const size_t save_slot)
    {
        clear_stream();
        const char *home = std::getenv("HOME");
        if (home == nullptr)
        {
            _ss << SAVE_WORLD;
            _ss << save_slot;
        }
        else
        {
            _ss << home;
            _ss << HOME_WORLD;
            _ss << save_slot;
        }
        return _ss.str();
    }
    static inline bool erase_file(const std::string &file_name)
    {
        clear_stream();
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
    static inline bool erase_save(const size_t index)
    {
        const bool k = erase_file(get_keymap_file(index));
        const bool s = erase_file(get_state_file(index));
        const bool w = erase_file(get_world_file(index));

        // Did we delete any saves?
        return k || s || w;
    }
    static inline bool exists_file(const std::string &file_name)
    {
        std::ifstream f(file_name);
        return f.good();
    }
    static inline void load_file(const std::string &file_name, std::vector<uint8_t> &stream)
    {
        // read bytes from file
        std::ifstream file(file_name, std::ios::in | std::ios::binary | std::ios::ate);
        if (file.is_open())
        {
            // Print diagnostic message
            std::cout << "file: loading from " << file_name << std::endl;

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
    static inline void save_file(const std::string &file_name, const std::vector<uint8_t> &stream)
    {
        // Save bytes to file
        std::ofstream file(file_name, std::ios::out | std::ios::binary);
        if (file.is_open())
        {
            // Print diagnostic message
            std::cout << "file: saving to " << file_name << std::endl;

            file.write(reinterpret_cast<const char *>(&stream[0]), stream.size());
            file.close();
        }
        else
        {
            std::cout << "file: could not save file '" << file_name << "'" << std::endl;
        }
    }
};
std::ostringstream file::_ss;
}

#endif
