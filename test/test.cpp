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
#include <iostream>
#include <tcgrid.h>
#include <tpath.h>
#include <tthread_map.h>
#include <tthread_pool.h>

int main()
{
    try
    {
        bool out = true;
        out = out && test_cgrid();
        out = out && test_path();
        out = out && test_thread_pool();
        out = out && test_thread_map();
        if (out)
        {
            std::cout << "Game tests passed!" << std::endl;
            return 0;
        }
    }
    catch (std::exception &ex)
    {
        std::cout << ex.what() << std::endl;
    }

    std::cout << "Game tests failed!" << std::endl;
    return -1;
}
