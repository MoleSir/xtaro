#pragma once

namespace xtaro::util
{
    int power(int base, int exponent);
    unsigned int fullBitsNumber(int size);
    double deviation(double v1, double v2) noexcept;
    auto radians(double angle) -> double;
}