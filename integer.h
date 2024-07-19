#ifndef INTTITAN_INTEGER_H
#define INTTITAN_INTEGER_H
#include <immer/vector.hpp>
#include <immer/vector_transient.hpp>
#include <utility>

namespace int_titan
{
    // This class represents the arbitrary-length integer type.
    class integer
    {
    public:
        using integer_digits = immer::vector<uint32_t>;
        // Constructors.
        explicit integer(integer_digits digits)
            :digits(std::move(digits))
        {
        }
        // Add the two integers.
        static integer add(const integer& x, const integer& y)
        {
            auto result = integer_digits().transient();
            int carry = 0;
            for(int i = 0; i < std::max(x.digits.size(), y.digits.size()); i++)
            {
                uint32_t sum = get_digit(x, i) + get_digit(y, i) + carry;
                carry = sum < get_digit(x, i) ? 1 : 0; // Check for integer overflow.
                result.push_back(sum);
            }
            if(carry == 1) // Add another digit if carry is on.
            {
                result.push_back(1);
            }
            return integer(result.persistent());
        }
    private:
        // A vector of base-2^32 digits (little-endian).
        integer_digits digits;
        // Get a digit from an integer.
        static uint32_t get_digit(const integer& x, const int index)
        {
            // Return the digit's value if present, else return 0 as a leading zero.
            return index < x.digits.size() ? x.digits[index] : 0;
        }
    };
}

#endif //INTTITAN_INTEGER_H
