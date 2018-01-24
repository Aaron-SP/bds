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
#ifndef __TESTUTIL__
#define __TESTUTIL__

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <utility>

bool compare(double one, double two, double threshold)
{
    // Compare double
    return std::abs(one - two) <= threshold;
}

bool compare(int one, int two)
{
    // Compare integers
    return one == two;
}

bool compare(const std::string &s1, const std::string &s2)
{
    // Compare string
    return s1 == s2;
}

std::string ask(const std::string &q)
{
    // Ask a question to the tester
    std::string s;
    std::cout << q << std::endl;
    std::cout << "answer(y/n) >> ";
    std::cin >> s;

    // Transform answer to lowercase
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);

    // Return the answer
    return s;
}

#endif