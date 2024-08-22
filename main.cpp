#include "integer.h"
#include <iostream>
#include <unordered_map>
#include <regex>

using int_titan::integer;

void free_calculator()
{
    while(true)
    {
        std::cout << "Enter radix:" << std::endl;
        int radix;
        std::cin >> radix;
        std::cout << "Enter the expression (end with '='):" << std::endl;
        std::string expression;
        std::cin.ignore();
        std::getline(std::cin, expression, '=');
        auto split = [](const std::string& str, const std::string& regex) -> std::vector<std::string>
        {
            std::regex r(regex);
            return {std::sregex_token_iterator(str.begin(), str.end(), r, -1), std::sregex_token_iterator()};
        };
        std::vector<std::string> terms = split(expression, R"([+\-\n])");
        std::vector<char> operations;
        if(expression[0] != '+' and expression[0] != '-')
        {
            operations.emplace_back('+');
        }
        for(char c : expression)
        {
            if(c == '+' or c == '-')
            {
                operations.emplace_back(c);
            }
        }
        auto compute = [](const std::vector<std::string>& terms, const std::vector<char>& operations, const int radix) -> integer
        {
            assert(terms.size() == operations.size());
            integer sum = integer::create("0", radix);
            for(int i = 0; i < terms.size(); i++)
            {
                integer num = integer::create(terms[i], radix);
                if(operations[i] == '-')
                {
                    num = integer::negate(num);
                }
                sum = integer::add(sum, num);
            }
            return sum;
        };
        integer result = compute(terms, operations, radix);
        std::cout << integer::to_string(result, radix, true) << std::endl;
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