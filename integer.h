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
        // From base 2^32 digits (native representation).
        static integer create(const integer_digits& digits, const bool is_negative)
        {
            integer x;
            x.digits = digits;
            x.is_negative = is_negative;
            return x;
        }
        // From string representation in a certain base
        static integer create(std::string_view str, const int base)
        {
            bool is_negative = false;
            if(!str.empty() and (str[0] == '-' or str[0] == '+'))
            {
                is_negative = str[0] == '-';
                str = str.substr(1);
            }
            // Currently, only power of 2 bases are supported.
            assert(is_pow2(base));
            constexpr int max_base = (1 + '9' - '0') + (1 + 'Z' - 'A'); // Decimal digits + alphabet.
            assert(base > 1 and base <= max_base);
            // A more performant approach is available for power of 2 bases.
            if(is_pow2(base))
            {
                return create(digits_from_pow2_base(str, base), is_negative);
            }
            // TODO : arbitrary bases
            throw std::exception();
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
            return create(result.persistent(), false);
        }
        // Sub one integer from the other.
        static integer sub(const integer& x, const integer& y)
        {
            // Cover the case where x is less than y.
            // x - y = -(y - x)
            if (is_less_than(x, y))
            {
                return negate(sub(y, x));
            }
            // Cover the cases for negative numbers.
            // -x - (-y) = y - x
            if (x.is_negative and y.is_negative)
            {
                return negate(sub(negate(y), negate(x)));
            }
            // -x - y = -(x + y)
            else if (x.is_negative and !y.is_negative)
            {
                return negate(add(negate(x), y));
            }
            // x - (-y) = x + y
            else if (!x.is_negative and y.is_negative)
            {
                return add(x, negate(y));
            }
            auto result = integer_digits().transient();
            int borrow = 0;
            for (int i = 0; i < std::max(x.digits.size(), y.digits.size()); i++)
            {
                digit diff;
                if (get_digit(x, i) - borrow >= get_digit(y, i)) // No underflow.
                {
                    diff = get_digit(x, i) - borrow - get_digit(y, i);
                    borrow = 0;
                }
                else // Underflow. Borrow set to 1 and inverse digit applied.
                {
                    diff = inverse_digit(get_digit(y, i) - (get_digit(x, i) - borrow));
                    borrow = 1;
                }
                result.push_back(diff);
            }
            // Remove leading 0s.
            while (!result.empty() and result[result.size() - 1] == 0)
            {
                result.take(result.size() - 1);
            }
            return create(result.persistent(), false);
        }
        // Is x less than y?
        static bool is_less_than(const integer& x, const integer& y)
        {
            if(x.digits.size() != y.digits.size())
            {
                return x.digits.size() < y.digits.size();
            }
            for(int i = static_cast<int>(x.digits.size() - 1); i >= 0; i--)
            {
                if(get_digit(x, i) != get_digit(y, i))
                {
                    return get_digit(x, i) < get_digit(y, i);
                }
            }
            return false;
        }
    private:
        // A vector of base-2^32 digits (little-endian).
        integer_digits digits;
        // Is the integer negative?
        bool is_negative = false;
        // Get a digit from an integer.
        static digit get_digit(const integer& x, const int index)
        {
            // Return the digit's value if present, else return 0 as a leading zero.
            return index < x.digits.size() ? x.digits[index] : 0;
        }
        // Get an inverse digit of a digit (the one with which it adds up to 10 base 2^32).
        static digit inverse_digit(const digit digit)
        {
            assert(digit != 0); // 0 does not have an inverse.
            return static_cast<superdigit>(max_digit) + 1 - digit;
        }
        // Check if an integer is a power of 2.
        static bool is_pow2(int x)
        {
            if(x == 0)
            {
                return false;
            }
            while(x > 1)
            {
                if(x % 2)
                {
                    return false;
                }
                x /= 2;
            }
            return true;
        }
        // Get log2 of an integer that is a power of 2.
        static int log2_pow2(int x)
        {
            assert(is_pow2(x));
            int count = 0;
            while(x /= 2)
            {
                count++;
            }
            return count;
        }
        // Get value of a digit character.
        static int get_digit_character_value(char d)
        {
            d = static_cast<char>(std::tolower(d));
            assert((d >= '0' and d <= '9') or (d >= 'a' and d <= 'z'));
            if(d >= '0' and d <= '9')
            {
                return d - '0';
            }
            return d - 'a';
        }
        // An optimized approach to read integers from power of 2 bases.
        static integer_digits digits_from_pow2_base(const std::string_view str, const int base)
        {
            assert(is_pow2(base));
            const int bit_count = log2_pow2(base);
            const int characters_per_digit = 32 / bit_count; // How many characters in str fill up a base-2^32 digit.
            auto result = integer_digits().transient();
            digit current_digit = 0;
            for(int counter = 0, i = static_cast<int>(str.size() - 1); i >= 0; i--)
            {
                const int current_character = get_digit_character_value(str[i]);
                current_digit |= (current_character << (bit_count * counter));
                if ((++counter %= characters_per_digit) == 0)
                {
                    result.push_back(current_digit);
                    current_digit = 0;
                }
            }
            if(current_digit != 0)
            {
                result.push_back(current_digit);
            }
            return result.persistent();
        }
    };
}

#endif //INTTITAN_INTEGER_H
