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
        using digit = uint32_t;
        using superdigit = uint64_t;
        static constexpr digit max_digit = std::numeric_limits<digit>::max();
        using integer_digits = immer::vector<digit>;
        // Constructors.
        explicit integer(integer_digits digits, const bool is_negative)
            :digits(std::move(digits)), is_negative(is_negative)
        {
        }
        // Negate the integer.
        static integer negate(integer x)
        {
            x.is_negative = !x.is_negative;
            return x;
        }
        // Add the two integers.
        static integer add(const integer& x, const integer& y)
        {
            // Cover the cases for negative numbers.
            // -x + (-y) = -(x + y)
            if(x.is_negative and y.is_negative)
            {
                return negate(add(negate(x), negate(y)));
            }
            // -x + y = y - x
            else if(x.is_negative and !y.is_negative)
            {
                return sub(y, negate(x));
            }
            // x + (-y) = x - y
            else if(!x.is_negative and y.is_negative)
            {
                return sub(x, negate(y));
            }
            auto result = integer_digits().transient();
            int carry = 0;
            for(int i = 0; i < std::max(x.digits.size(), y.digits.size()); i++)
            {
                digit sum = get_digit(x, i) + get_digit(y, i) + carry;
                carry = sum < get_digit(x, i) ? 1 : 0; // Check for integer overflow.
                result.push_back(sum);
            }
            if(carry == 1) // Add another digit if carry is on.
            {
                result.push_back(1);
            }
            return integer(result.persistent(), false);
        }
        // Sub one integer from the other.
        static integer sub(const integer& x, const integer& y)
        {
            return integer(0, false);
        }
    private:
        // A vector of base-2^32 digits (little-endian).
        integer_digits digits;
        // Is the integer negative?
        bool is_negative;
        // Get a digit from an integer.
        static digit get_digit(const integer& x, const int index)
        {
            // Return the digit's value if present, else return 0 as a leading zero.
            return index < x.digits.size() ? x.digits[index] : 0;
        }
        // Get an inverse digit of a digit (the one with which it adds up to 10 base 2^32).
        static digit inverse_digit(const digit digit)
        {
            return static_cast<superdigit>(max_digit) + 1 - digit;
        }
    };
}

#endif //INTTITAN_INTEGER_H
