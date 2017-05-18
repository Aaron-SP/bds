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