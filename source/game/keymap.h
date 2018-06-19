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

#include <min/window.h>
#include <string>
#include <vector>

namespace game
{

class key_map
{
  private:
    static constexpr size_t _max_strings = 70;
    std::vector<std::string> _map;

    void load_strings()
    {
        _map[0] = "F1";
        _map[1] = "F2";
        _map[2] = "F3";
        _map[3] = "F4";
        _map[4] = "F5";
        _map[5] = "F6";
        _map[6] = "F7";
        _map[7] = "F8";
        _map[8] = "F9";
        _map[9] = "F10";
        _map[10] = "F11";
        _map[11] = "F12";
        _map[12] = "KEY 0";
        _map[13] = "KEY 1";
        _map[14] = "KEY 2";
        _map[15] = "KEY 3";
        _map[16] = "KEY 4";
        _map[17] = "KEY 5";
        _map[18] = "KEY 6";
        _map[19] = "KEY 7";
        _map[20] = "KEY 8";
        _map[21] = "KEY 9";
        _map[22] = "KEY A";
        _map[23] = "KEY B";
        _map[24] = "KEY C";
        _map[25] = "KEY D";
        _map[26] = "KEY E";
        _map[27] = "KEY F";
        _map[28] = "KEY G";
        _map[29] = "KEY H";
        _map[30] = "KEY I";
        _map[31] = "KEY J";
        _map[32] = "KEY K";
        _map[33] = "KEY L";
        _map[34] = "KEY M";
        _map[35] = "KEY N";
        _map[36] = "KEY O";
        _map[37] = "KEY P";
        _map[38] = "KEY Q";
        _map[39] = "KEY R";
        _map[40] = "KEY S";
        _map[41] = "KEY T";
        _map[42] = "KEY U";
        _map[43] = "KEY V";
        _map[44] = "KEY W";
        _map[45] = "KEY X";
        _map[46] = "KEY Y";
        _map[47] = "KEY Z";
        _map[48] = "NUM 0";
        _map[49] = "NUM 1";
        _map[50] = "NUM 2";
        _map[51] = "NUM 3";
        _map[52] = "NUM 4";
        _map[53] = "NUM 5";
        _map[54] = "NUM 6";
        _map[55] = "NUM 7";
        _map[56] = "NUM 8";
        _map[57] = "NUM 9";
        _map[58] = "LSHIFT";
        _map[59] = "RSHIFT";
        _map[60] = "LCONTROL";
        _map[61] = "RCONTROL";
        _map[62] = "TAB";
        _map[63] = "ENTER";
        _map[64] = "BACKSPACE";
        _map[65] = "SPACE";
        _map[66] = "LALT";
        _map[67] = "RALT";
        _map[68] = "ESCAPE";
        _map[69] = "Invalid Key";
    }

  public:
    key_map() : _map(_max_strings)
    {
        // Load the key strings
        load_strings();
    }
    const std::string &get_string(const min::window::key_type key) const
    {
        switch (key)
        {
        case min::window::key_code::F1:
            return _map[0];
        case min::window::key_code::F2:
            return _map[1];
        case min::window::key_code::F3:
            return _map[2];
        case min::window::key_code::F4:
            return _map[3];
        case min::window::key_code::F5:
            return _map[4];
        case min::window::key_code::F6:
            return _map[5];
        case min::window::key_code::F7:
            return _map[6];
        case min::window::key_code::F8:
            return _map[7];
        case min::window::key_code::F9:
            return _map[8];
        case min::window::key_code::F10:
            return _map[9];
        case min::window::key_code::F11:
            return _map[10];
        case min::window::key_code::F12:
            return _map[11];
        case min::window::key_code::KEY0:
            return _map[12];
        case min::window::key_code::KEY1:
            return _map[13];
        case min::window::key_code::KEY2:
            return _map[14];
        case min::window::key_code::KEY3:
            return _map[15];
        case min::window::key_code::KEY4:
            return _map[16];
        case min::window::key_code::KEY5:
            return _map[17];
        case min::window::key_code::KEY6:
            return _map[18];
        case min::window::key_code::KEY7:
            return _map[19];
        case min::window::key_code::KEY8:
            return _map[20];
        case min::window::key_code::KEY9:
            return _map[21];
        case min::window::key_code::KEYA:
            return _map[22];
        case min::window::key_code::KEYB:
            return _map[23];
        case min::window::key_code::KEYC:
            return _map[24];
        case min::window::key_code::KEYD:
            return _map[25];
        case min::window::key_code::KEYE:
            return _map[26];
        case min::window::key_code::KEYF:
            return _map[27];
        case min::window::key_code::KEYG:
            return _map[28];
        case min::window::key_code::KEYH:
            return _map[29];
        case min::window::key_code::KEYI:
            return _map[30];
        case min::window::key_code::KEYJ:
            return _map[31];
        case min::window::key_code::KEYK:
            return _map[32];
        case min::window::key_code::KEYL:
            return _map[33];
        case min::window::key_code::KEYM:
            return _map[34];
        case min::window::key_code::KEYN:
            return _map[35];
        case min::window::key_code::KEYO:
            return _map[36];
        case min::window::key_code::KEYP:
            return _map[37];
        case min::window::key_code::KEYQ:
            return _map[38];
        case min::window::key_code::KEYR:
            return _map[39];
        case min::window::key_code::KEYS:
            return _map[40];
        case min::window::key_code::KEYT:
            return _map[41];
        case min::window::key_code::KEYU:
            return _map[42];
        case min::window::key_code::KEYV:
            return _map[43];
        case min::window::key_code::KEYW:
            return _map[44];
        case min::window::key_code::KEYX:
            return _map[45];
        case min::window::key_code::KEYY:
            return _map[46];
        case min::window::key_code::KEYZ:
            return _map[47];
        case min::window::key_code::NUM0:
            return _map[48];
        case min::window::key_code::NUM1:
            return _map[49];
        case min::window::key_code::NUM2:
            return _map[50];
        case min::window::key_code::NUM3:
            return _map[51];
        case min::window::key_code::NUM4:
            return _map[52];
        case min::window::key_code::NUM5:
            return _map[53];
        case min::window::key_code::NUM6:
            return _map[54];
        case min::window::key_code::NUM7:
            return _map[55];
        case min::window::key_code::NUM8:
            return _map[56];
        case min::window::key_code::NUM9:
            return _map[57];
        case min::window::key_code::LSHIFT:
            return _map[58];
        case min::window::key_code::RSHIFT:
            return _map[59];
        case min::window::key_code::LCONTROL:
            return _map[60];
        case min::window::key_code::RCONTROL:
            return _map[61];
        case min::window::key_code::TAB:
            return _map[62];
        case min::window::key_code::ENTER:
            return _map[63];
        case min::window::key_code::BACKSPACE:
            return _map[64];
        case min::window::key_code::SPACE:
            return _map[65];
        case min::window::key_code::LALT:
            return _map[66];
        case min::window::key_code::RALT:
            return _map[67];
        case min::window::key_code::ESCAPE:
            return _map[68];
        default:
            return _map[69];
        }
    }
};
}

#endif
