#ifndef INTTITAN_INTEGER_H
#define INTTITAN_INTEGER_H
#include <immer/vector.hpp>
#include <immer/vector_transient.hpp>
#include <immer/flex_vector.hpp>
#include <immer/flex_vector_transient.hpp>
#include <utility>
#include <sstream>

namespace int_titan
{
    // This class represents the arbitrary-length integer type.
    class integer
    {
    public:
        using digit = uint32_t;
        using superdigit = uint64_t;
        static constexpr digit max_digit = std::numeric_limits<digit>::max();
        using integer_digits = immer::flex_vector<digit>;
        // From base 2^32 digits (native representation).
        static integer create(const integer_digits& digits, const bool is_negative)
        {
            integer x;
            x.digits = digits;
            x.is_negative = is_negative;
            return x;
        }
        // From string representation in decimal or hexadecimal.
        static integer create(std::string_view str, const bool is_hex = true)
        {
            bool is_negative = false;
            if(!str.empty() and (str[0] == '-' or str[0] == '+'))
            {
                is_negative = str[0] == '-';
                str = str.substr(1);
            }
            // Currently, only hex is supported.
            assert(is_hex);
            return create(digits_from_string(str, is_hex), is_negative);
        }
        // Zero value.
        static const integer zero;
        // Convert integer to string.
        static std::string to_string(const integer& x, const bool is_hex = true, const bool uppercase = true)
        {
            // Currently, only hex is supported.
            assert(is_hex);
            return string_from_integer(x, is_hex, uppercase);
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
                return subtract(y, negate(x));
            }
            // x + (-y) = x - y
            else if(!x.is_negative and y.is_negative)
            {
                return subtract(x, negate(y));
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
        // Subtract one integer from the other.
        static integer subtract(const integer& x, const integer& y)
        {
            // Cover the case where x is less than y.
            // x - y = -(y - x)
            if (is_less_than(x, y))
            {
                return negate(subtract(y, x));
            }
            // Cover the cases for negative numbers.
            // -x - (-y) = y - x
            if (x.is_negative and y.is_negative)
            {
                return negate(subtract(negate(y), negate(x)));
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
        // Shift left (multiply by 10^amount, base 2^32), basically adding 'amount' zeroes.
        static integer shift_left(integer x, const int amount)
        {
            for(int i = 0; i < amount; i++)
            {
                x.digits = x.digits.push_front(0);
            }
            return create(x.digits.push_front(0), x.is_negative);
        }
        // Shift right (divide by 10^amount, base 2^32), basically removing 'amount' digits from the right.
        static integer shift_right(const integer& x, const int amount)
        {
            return create(x.digits.drop(amount), x.is_negative);
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
        // Get value of a digit character (e.g. value of '0' is 0, value of 'D' is 13).
        static int get_digit_character_value(char d)
        {
            d = static_cast<char>(std::tolower(d));
            assert((d >= '0' and d <= '9') or (d >= 'a' and d <= 'z'));
            if(d >= '0' and d <= '9')
            {
                return d - '0';
            }
            return 10 + d - 'a';
        }
        // Get digit character for value.
        static char get_digit_character(const int value, const bool uppercase = true)
        {
            assert(value < (1 + '9' - '0') + (1 + 'Z' - 'A'));
            if(value < 10)
            {
                return static_cast<char>('0' + value);
            }
            char a = uppercase ? 'A' : 'a';
            return static_cast<char>(a + value - 10);
        }
        // Read integers from strings (decimal and hex).
        static integer_digits digits_from_string(const std::string_view str, const bool is_hex)
        {
            // Only hex is currently supported.
            assert(is_hex);
            if(is_hex)
            {
                return digits_from_hex_string(str);
            }
            throw std::exception();
        }
        // Convert integers to strings (decimal and hex).
        static std::string string_from_integer(const integer& x, const bool is_hex = true, const bool uppercase = true)
        {
            // Only hex is currently supported.
            assert(is_hex);
            if(is_hex)
            {
                return hex_string_from_integer(x, uppercase);
            }
            throw std::exception();
        }
        // Read integers from hex strings.
        static integer_digits digits_from_hex_string(const std::string_view str)
        {
            const int bits_per_hex_digit = 4; // How many bits are contained in a hex digit.
            const int characters_per_digit = 32 / bits_per_hex_digit; // How many hex digits in str fill up a base-2^32 digit.
            auto result = integer_digits().transient();
            digit current_digit = 0;
            for(int counter = 0, i = static_cast<int>(str.size() - 1); i >= 0; i--)
            {
                const int current_character = get_digit_character_value(str[i]);
                current_digit |= (current_character << (bits_per_hex_digit * counter));
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
        // Convert integers to hex strings.
        static std::string hex_string_from_integer(const integer& x, const bool uppercase = true)
        {
            const int bits_per_hex_digit = 4; // How many bits are contained in a hex digit.
            std::stringstream ss;
            if(x.is_negative)
            {
                ss << '-';
            }
            const auto& digits = x.digits;
            int current_bits = 0;
            bool has_at_least_one_character = false; // Used to avoid leading zeroes.
            // Reads the digits from the most significant to the least significant.
            for(int counter = 0, i = static_cast<int>(digits.size() - 1); i >= 0; i--)
            {
                digit current_digit = digits[i];
                // Reads individual bits from the most significant to the least significant.
                for(int bit = 31; bit >= 0; bit--)
                {
                    bool is_set = !!(current_digit & (1 << bit));
                    current_bits |= (is_set << (bits_per_hex_digit - 1 - (counter % bits_per_hex_digit)));
                    if((++counter %= bits_per_hex_digit) == 0)
                    {
                        if(has_at_least_one_character or current_bits != 0)
                        {
                            ss << get_digit_character(current_bits, uppercase);
                            has_at_least_one_character = true;
                            current_bits = 0;
                        }
                    }
                }
            }
            return ss.str();
        }
    };
}

const int_titan::integer int_titan::integer::zero = int_titan::integer::create(integer_digits(), false);

#endif //INTTITAN_INTEGER_H
