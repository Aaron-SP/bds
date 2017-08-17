/* Copyright [2013-2016] [Aaron Springstroh, Minimal Graphics Library]

This file is part of the MGLCraft.

MGLCraft is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MGLCraft is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MGLCraft.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __FILE_UTILS__
#define __FILE_UTILS__

#include <fstream>
#include <vector>

namespace game
{

void load_file(const std::string &file_name, std::vector<uint8_t> &stream)
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
void save_file(const std::string &file_name, const std::vector<uint8_t> &stream)
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
