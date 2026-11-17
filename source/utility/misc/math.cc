#include "math.hh"
#include <stdexcept>
#include <cmath>

namespace xtaro::util
{
    int power(int base, int exponent) 
    {
        if (exponent < 0) throw std::invalid_argument{"util::power: Argument 'exponent' < 0"};
        int value = 1;
        for (int i = 0; i < exponent; ++i)
            value *= base;
        return value;
    }

    unsigned int fullBitsNumber(int size)
    {
        unsigned int value {0x00000000};
        for (int i = 0; i < size; ++i)
            value = (value << 1) + 1;
        return value;
    }

    double deviation(double v1, double v2) noexcept
    {
        if (v1 > v2) return v1 - v2;
        else return v2 - v1;
    }

    auto radians(double angle) -> double
    {
        return angle * 3.1415926 / 180.;
    }

}