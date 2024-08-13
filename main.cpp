#include "integer.h"
#include <iostream>
#include <unordered_map>

void free_calculator()
{
    while(true)
    {

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