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
#ifndef __KEY_MAP__
#define __KEY_MAP__

#include <game/file.h>
#include <game/options.h>
#include <min/serial.h>
#include <min/strtoken.h>
#include <min/window.h>
#include <string>
#include <vector>

namespace game
{

class key_map
{
  private:
    static constexpr size_t _max_prefix = 26;
    static constexpr size_t _max_keys = 75;
    std::vector<std::string> _prefix;
    std::vector<std::string> _key;
    std::vector<min::window::key_code> _key_code;
    std::vector<min::window::key_code> _keymap;
    bool _persist;

    inline void load_prefix_strings()
    {
        _prefix[0] = "For";
        _prefix[1] = "Back";
        _prefix[2] = "Left";
        _prefix[3] = "Right";
        _prefix[4] = "Reset";
        _prefix[5] = "ScaleX";
        _prefix[6] = "ScaleY";
        _prefix[7] = "ScaleZ";
        _prefix[8] = "Item1";
        _prefix[9] = "Item2";
        _prefix[10] = "Item3";
        _prefix[11] = "Item4";
        _prefix[12] = "Item5";
        _prefix[13] = "Item6";
        _prefix[14] = "Item7";
        _prefix[15] = "Item8";
        _prefix[16] = "Jump";
        _prefix[17] = "Dash";
        _prefix[18] = "Inv";
        _prefix[19] = "Debug";
        _prefix[20] = "Wire";
        _prefix[21] = "MusicUp";
        _prefix[22] = "MusicDown";
        _prefix[23] = "Menu";
        _prefix[24] = "Use";
        _prefix[25] = "Drop";
    }
    inline void load_key_strings()
    {
        _key[0] = "F1";
        _key[1] = "F2";
        _key[2] = "F3";
        _key[3] = "F4";
        _key[4] = "F5";
        _key[5] = "F6";
        _key[6] = "F7";
        _key[7] = "F8";
        _key[8] = "F9";
        _key[9] = "F10";
        _key[10] = "F11";
        _key[11] = "F12";
        _key[12] = "0";
        _key[13] = "1";
        _key[14] = "2";
        _key[15] = "3";
        _key[16] = "4";
        _key[17] = "5";
        _key[18] = "6";
        _key[19] = "7";
        _key[20] = "8";
        _key[21] = "9";
        _key[22] = "A";
        _key[23] = "B";
        _key[24] = "C";
        _key[25] = "D";
        _key[26] = "E";
        _key[27] = "F";
        _key[28] = "G";
        _key[29] = "H";
        _key[30] = "I";
        _key[31] = "J";
        _key[32] = "K";
        _key[33] = "L";
        _key[34] = "M";
        _key[35] = "N";
        _key[36] = "O";
        _key[37] = "P";
        _key[38] = "Q";
        _key[39] = "R";
        _key[40] = "S";
        _key[41] = "T";
        _key[42] = "U";
        _key[43] = "V";
        _key[44] = "W";
        _key[45] = "X";
        _key[46] = "Y";
        _key[47] = "Z";
        _key[48] = "NUM 0";
        _key[49] = "NUM 1";
        _key[50] = "NUM 2";
        _key[51] = "NUM 3";
        _key[52] = "NUM 4";
        _key[53] = "NUM 5";
        _key[54] = "NUM 6";
        _key[55] = "NUM 7";
        _key[56] = "NUM 8";
        _key[57] = "NUM 9";
        _key[58] = "LSHIFT";
        _key[59] = "RSHIFT";
        _key[60] = "LCONTROL";
        _key[61] = "RCONTROL";
        _key[62] = "TAB";
        _key[63] = "ENTER";
        _key[64] = "BACKSPACE";
        _key[65] = "SPACE";
        _key[66] = "LALT";
        _key[67] = "RALT";
        _key[68] = "ESC";
        _key[69] = "APOST";
        _key[70] = "QUOTE";
        _key[71] = ",";
        _key[72] = ".";
        _key[73] = ";";
        _key[74] = "???";
    }
    inline void load_key_codes()
    {
        _key_code[0] = min::window::key_code::F1;
        _key_code[1] = min::window::key_code::F2;
        _key_code[2] = min::window::key_code::F3;
        _key_code[3] = min::window::key_code::F4;
        _key_code[4] = min::window::key_code::F5;
        _key_code[5] = min::window::key_code::F6;
        _key_code[6] = min::window::key_code::F7;
        _key_code[7] = min::window::key_code::F8;
        _key_code[8] = min::window::key_code::F9;
        _key_code[9] = min::window::key_code::F10;
        _key_code[10] = min::window::key_code::F11;
        _key_code[11] = min::window::key_code::F12;
        _key_code[12] = min::window::key_code::KEY0;
        _key_code[13] = min::window::key_code::KEY1;
        _key_code[14] = min::window::key_code::KEY2;
        _key_code[15] = min::window::key_code::KEY3;
        _key_code[16] = min::window::key_code::KEY4;
        _key_code[17] = min::window::key_code::KEY5;
        _key_code[18] = min::window::key_code::KEY6;
        _key_code[19] = min::window::key_code::KEY7;
        _key_code[20] = min::window::key_code::KEY8;
        _key_code[21] = min::window::key_code::KEY9;
        _key_code[22] = min::window::key_code::KEYA;
        _key_code[23] = min::window::key_code::KEYB;
        _key_code[24] = min::window::key_code::KEYC;
        _key_code[25] = min::window::key_code::KEYD;
        _key_code[26] = min::window::key_code::KEYE;
        _key_code[27] = min::window::key_code::KEYF;
        _key_code[28] = min::window::key_code::KEYG;
        _key_code[29] = min::window::key_code::KEYH;
        _key_code[30] = min::window::key_code::KEYI;
        _key_code[31] = min::window::key_code::KEYJ;
        _key_code[32] = min::window::key_code::KEYK;
        _key_code[33] = min::window::key_code::KEYL;
        _key_code[34] = min::window::key_code::KEYM;
        _key_code[35] = min::window::key_code::KEYN;
        _key_code[36] = min::window::key_code::KEYO;
        _key_code[37] = min::window::key_code::KEYP;
        _key_code[38] = min::window::key_code::KEYQ;
        _key_code[39] = min::window::key_code::KEYR;
        _key_code[40] = min::window::key_code::KEYS;
        _key_code[41] = min::window::key_code::KEYT;
        _key_code[42] = min::window::key_code::KEYU;
        _key_code[43] = min::window::key_code::KEYV;
        _key_code[44] = min::window::key_code::KEYW;
        _key_code[45] = min::window::key_code::KEYX;
        _key_code[46] = min::window::key_code::KEYY;
        _key_code[47] = min::window::key_code::KEYZ;
        _key_code[48] = min::window::key_code::NUM0;
        _key_code[49] = min::window::key_code::NUM1;
        _key_code[50] = min::window::key_code::NUM2;
        _key_code[51] = min::window::key_code::NUM3;
        _key_code[52] = min::window::key_code::NUM4;
        _key_code[53] = min::window::key_code::NUM5;
        _key_code[54] = min::window::key_code::NUM6;
        _key_code[55] = min::window::key_code::NUM7;
        _key_code[56] = min::window::key_code::NUM8;
        _key_code[57] = min::window::key_code::NUM9;
        _key_code[58] = min::window::key_code::LSHIFT;
        _key_code[59] = min::window::key_code::RSHIFT;
        _key_code[60] = min::window::key_code::LCONTROL;
        _key_code[61] = min::window::key_code::RCONTROL;
        _key_code[62] = min::window::key_code::TAB;
        _key_code[63] = min::window::key_code::ENTER;
        _key_code[64] = min::window::key_code::BACKSPACE;
        _key_code[65] = min::window::key_code::SPACE;
        _key_code[66] = min::window::key_code::LALT;
        _key_code[67] = min::window::key_code::RALT;
        _key_code[68] = min::window::key_code::ESCAPE;
        _key_code[69] = min::window::key_code::APOSTROPHE;
        _key_code[70] = min::window::key_code::QUOTE;
        _key_code[71] = min::window::key_code::COMMA;
        _key_code[72] = min::window::key_code::PERIOD;
        _key_code[73] = min::window::key_code::SEMICOLON;
    }
    inline void load_default_key_map(const options &opt)
    {
        // Manually load the keymap
        if (opt.is_key_map_qwerty())
        {
            // Alert user which key map is loaded
            std::cout << "Key map type: QWERTY" << std::endl;

            // Set default to QWERTY
            _keymap[0] = min::window::key_code::KEYW;
            _keymap[1] = min::window::key_code::KEYS;
            _keymap[2] = min::window::key_code::KEYA;
            _keymap[3] = min::window::key_code::KEYD;
            _keymap[4] = min::window::key_code::KEYR;
            _keymap[5] = min::window::key_code::KEYZ;
            _keymap[6] = min::window::key_code::KEYX;
            _keymap[7] = min::window::key_code::KEYC;
            _keymap[8] = min::window::key_code::KEY1;
            _keymap[9] = min::window::key_code::KEY2;
            _keymap[10] = min::window::key_code::KEY3;
            _keymap[11] = min::window::key_code::KEY4;
            _keymap[12] = min::window::key_code::KEY5;
            _keymap[13] = min::window::key_code::KEY6;
            _keymap[14] = min::window::key_code::KEY7;
            _keymap[15] = min::window::key_code::KEY8;
            _keymap[16] = min::window::key_code::SPACE;
            _keymap[17] = min::window::key_code::LSHIFT;
            _keymap[18] = min::window::key_code::TAB;
            _keymap[19] = min::window::key_code::F1;
            _keymap[20] = min::window::key_code::F2;
            _keymap[21] = min::window::key_code::F3;
            _keymap[22] = min::window::key_code::F4;
            _keymap[23] = min::window::key_code::ESCAPE;
            _keymap[24] = min::window::key_code::KEYE;
            _keymap[25] = min::window::key_code::KEYQ;
        }
        else if (opt.is_key_map_dvorak())
        {
            // Alert user which key map is loaded
            std::cout << "Key map type: DVORAK" << std::endl;

            // Set default to DVORAK
            _keymap[0] = min::window::key_code::COMMA;
            _keymap[1] = min::window::key_code::KEYO;
            _keymap[2] = min::window::key_code::KEYA;
            _keymap[3] = min::window::key_code::KEYE;
            _keymap[4] = min::window::key_code::KEYP;
            _keymap[5] = min::window::key_code::SEMICOLON;
            _keymap[6] = min::window::key_code::KEYQ;
            _keymap[7] = min::window::key_code::KEYJ;
            _keymap[8] = min::window::key_code::KEY1;
            _keymap[9] = min::window::key_code::KEY2;
            _keymap[10] = min::window::key_code::KEY3;
            _keymap[11] = min::window::key_code::KEY4;
            _keymap[12] = min::window::key_code::KEY5;
            _keymap[13] = min::window::key_code::KEY6;
            _keymap[14] = min::window::key_code::KEY7;
            _keymap[15] = min::window::key_code::KEY8;
            _keymap[16] = min::window::key_code::SPACE;
            _keymap[17] = min::window::key_code::LSHIFT;
            _keymap[18] = min::window::key_code::TAB;
            _keymap[19] = min::window::key_code::F1;
            _keymap[20] = min::window::key_code::F2;
            _keymap[21] = min::window::key_code::F3;
            _keymap[22] = min::window::key_code::F4;
            _keymap[23] = min::window::key_code::ESCAPE;
            _keymap[24] = min::window::key_code::PERIOD;
            _keymap[25] = min::window::key_code::QUOTE;
        }
    }
    inline void load_key_map(const std::string _file)
    {
        // Override with config file
        std::ifstream file(_file, std::ios::in | std::ios::binary | std::ios::ate);
        if (file.is_open())
        {
            // Get the size of the file
            const auto size = file.tellg();

            // Adjust file pointer to beginning
            file.seekg(0, std::ios::beg);

            // Allocate space for new file
            std::string data(size, 0);

            // Read bytes and close the file
            file.read(&data[0], size);

            // Close the file
            file.close();

            // Process the file
            load(data);
        }
        else
        {
            // Alert that keymap not loaded
            std::cout << "file: could not load file '" << _file << "'" << std::endl;
        }
    }
    inline void load(const std::string &data)
    {
        // Alert user which key map is loaded
        std::cout << "Key map override: enabled" << std::endl;

        // Get locations of all lines in string buffer
        auto lines = min::read_lines(data);

        // Read line by line
        for (auto &position : lines)
        {
            // Read line and trim the line whitespace
            std::string line = data.substr(position.first, position.second);
            min::trim(line);

            // Skip empty line size in bytes
            if (line.size() == 0)
            {
                continue;
            }

            // Split on equals, verify two columns, "=+"
            std::vector<std::string> columns = min::split_equal(line);
            if (columns.size() != 2)
            {
                throw std::runtime_error("key_map: wrong column size, invalid format '" + line + "'");
            }

            // Find the index for this line
            int type_index = -1;
            const size_t prefix_size = _max_prefix;
            for (size_t i = 0; i < prefix_size; i++)
            {
                // If we found a match
                if (columns[0].compare(_prefix[i]) == 0)
                {
                    // Set the variable index
                    type_index = i;
                    break;
                }
            }

            // If no match was found
            if (type_index == -1)
            {
                std::cout << "key_map: unknown variable '" << columns[0] << "'" << std::endl;
                continue;
            }

            // Find the key code for this line
            int key_index = -1;
            const size_t key_size = _max_keys - 1;
            for (size_t i = 0; i < key_size; i++)
            {
                // If we found a match
                if (columns[1].compare(_key[i]) == 0)
                {
                    key_index = i;
                    break;
                }
            }

            // If no match was found
            if (key_index == -1)
            {
                std::cout << "key_map: unknown key '" << columns[1] << "'" << std::endl;
                continue;
            }

            // Set the keymap
            _keymap[type_index] = _key_code[key_index];
        }
    }

  public:
    key_map(const options &opt)
        : _prefix(_max_prefix, "?"),
          _key(_max_keys), _key_code(_max_keys),
          _keymap(_max_prefix, min::window::key_code::APOSTROPHE),
          _persist(opt.is_key_map_persist())
    {
        // Load the key strings
        load_prefix_strings();

        // Load the key strings
        load_key_strings();

        // Load the key codes
        load_key_codes();

        // Load default key map
        load_default_key_map(opt);
    }
    inline const std::string &get_prefix_string(const size_t index) const
    {
        return _prefix[index];
    }
    const std::string &get_key_string(const min::window::key_type key) const
    {
        switch (key)
        {
        case min::window::key_code::F1:
            return _key[0];
        case min::window::key_code::F2:
            return _key[1];
        case min::window::key_code::F3:
            return _key[2];
        case min::window::key_code::F4:
            return _key[3];
        case min::window::key_code::F5:
            return _key[4];
        case min::window::key_code::F6:
            return _key[5];
        case min::window::key_code::F7:
            return _key[6];
        case min::window::key_code::F8:
            return _key[7];
        case min::window::key_code::F9:
            return _key[8];
        case min::window::key_code::F10:
            return _key[9];
        case min::window::key_code::F11:
            return _key[10];
        case min::window::key_code::F12:
            return _key[11];
        case min::window::key_code::KEY0:
            return _key[12];
        case min::window::key_code::KEY1:
            return _key[13];
        case min::window::key_code::KEY2:
            return _key[14];
        case min::window::key_code::KEY3:
            return _key[15];
        case min::window::key_code::KEY4:
            return _key[16];
        case min::window::key_code::KEY5:
            return _key[17];
        case min::window::key_code::KEY6:
            return _key[18];
        case min::window::key_code::KEY7:
            return _key[19];
        case min::window::key_code::KEY8:
            return _key[20];
        case min::window::key_code::KEY9:
            return _key[21];
        case min::window::key_code::KEYA:
            return _key[22];
        case min::window::key_code::KEYB:
            return _key[23];
        case min::window::key_code::KEYC:
            return _key[24];
        case min::window::key_code::KEYD:
            return _key[25];
        case min::window::key_code::KEYE:
            return _key[26];
        case min::window::key_code::KEYF:
            return _key[27];
        case min::window::key_code::KEYG:
            return _key[28];
        case min::window::key_code::KEYH:
            return _key[29];
        case min::window::key_code::KEYI:
            return _key[30];
        case min::window::key_code::KEYJ:
            return _key[31];
        case min::window::key_code::KEYK:
            return _key[32];
        case min::window::key_code::KEYL:
            return _key[33];
        case min::window::key_code::KEYM:
            return _key[34];
        case min::window::key_code::KEYN:
            return _key[35];
        case min::window::key_code::KEYO:
            return _key[36];
        case min::window::key_code::KEYP:
            return _key[37];
        case min::window::key_code::KEYQ:
            return _key[38];
        case min::window::key_code::KEYR:
            return _key[39];
        case min::window::key_code::KEYS:
            return _key[40];
        case min::window::key_code::KEYT:
            return _key[41];
        case min::window::key_code::KEYU:
            return _key[42];
        case min::window::key_code::KEYV:
            return _key[43];
        case min::window::key_code::KEYW:
            return _key[44];
        case min::window::key_code::KEYX:
            return _key[45];
        case min::window::key_code::KEYY:
            return _key[46];
        case min::window::key_code::KEYZ:
            return _key[47];
        case min::window::key_code::NUM0:
            return _key[48];
        case min::window::key_code::NUM1:
            return _key[49];
        case min::window::key_code::NUM2:
            return _key[50];
        case min::window::key_code::NUM3:
            return _key[51];
        case min::window::key_code::NUM4:
            return _key[52];
        case min::window::key_code::NUM5:
            return _key[53];
        case min::window::key_code::NUM6:
            return _key[54];
        case min::window::key_code::NUM7:
            return _key[55];
        case min::window::key_code::NUM8:
            return _key[56];
        case min::window::key_code::NUM9:
            return _key[57];
        case min::window::key_code::LSHIFT:
            return _key[58];
        case min::window::key_code::RSHIFT:
            return _key[59];
        case min::window::key_code::LCONTROL:
            return _key[60];
        case min::window::key_code::RCONTROL:
            return _key[61];
        case min::window::key_code::TAB:
            return _key[62];
        case min::window::key_code::ENTER:
            return _key[63];
        case min::window::key_code::BACKSPACE:
            return _key[64];
        case min::window::key_code::SPACE:
            return _key[65];
        case min::window::key_code::LALT:
            return _key[66];
        case min::window::key_code::RALT:
            return _key[67];
        case min::window::key_code::ESCAPE:
            return _key[68];
        case min::window::key_code::APOSTROPHE:
            return _key[69];
        case min::window::key_code::QUOTE:
            return _key[70];
        case min::window::key_code::COMMA:
            return _key[71];
        case min::window::key_code::PERIOD:
            return _key[72];
        case min::window::key_code::SEMICOLON:
            return _key[73];
        default:
            return _key[74];
        }
    }
    inline void load()
    {
        // Load the key map
        if (_persist)
        {
            load_key_map("save/keymap");
        }
    }
    inline void save(const min::window &win)
    {
        // Create output stream for saving keymap
        std::vector<uint8_t> stream;

        // Get access to the keyboard
        const auto &keyboard = win.get_keyboard();

        // Get the active keys
        const std::vector<min::window::key_type> &keys = keyboard.get_active_keys();

        // Create the keymap config file
        const size_t size = keys.size();
        for (size_t i = 0; i < size; i++)
        {
            // Write the prefix string
            {
                const std::string &str = get_prefix_string(i);
                const size_t str_size = str.size();
                for (size_t j = 0; j < str_size; j++)
                {
                    min::write_le<uint8_t>(stream, str[j]);
                }
            }

            // Write separator
            min::write_le<uint8_t>(stream, '=');

            // Write the key string
            {
                // Get the string
                const std::string &str = get_key_string(keys[i]);
                const size_t str_size = str.size();
                for (size_t j = 0; j < str_size; j++)
                {
                    min::write_le<uint8_t>(stream, str[j]);
                }
            }

            // Write newline
            min::write_le<uint8_t>(stream, '\n');
        }

        // Write data to file
        save_file("save/keymap", stream);
    }
    inline min::window::key_code operator[](const size_t i) const
    {
        return _keymap[i];
    }
    static inline size_t size()
    {
        return _max_prefix;
    }
};
}

#endif
