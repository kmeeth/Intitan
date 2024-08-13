#include "integer.h"
#include <iostream>
#include <unordered_map>

using int_titan::integer;

void free_calculator()
{
    while(true)
    {
        std::cout << "Enter radix:" << std::endl;
        int radix;
        std::cin >> radix;
        std::cout << "Enter two numbers to add:" << std::endl;
        std::string x, y;
        std::cin >> x >> y;
        integer a = integer::create(x, radix);
        integer b = integer::create(y, radix);
        integer r = integer::add(a, b);
    }
}

int main()
{
    std::cout << "Pick the desired option:" << std::endl;
    std::cout << "1. Free calculator." << std::endl;
    while(true)
    {
        int selected;
        std::cin >> selected;
        std::unordered_map<int, decltype(&free_calculator)> options = {{ 1, &free_calculator }};
        if (options.find(selected) != options.end())
        {
            options[selected]();
        }
        else
        {
            std::cout << "No such option. Try again:" << std::endl;
        }
    }
}