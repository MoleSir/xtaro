#pragma once
#include <random>

namespace xtaro::util
{

    extern std::mt19937 randomEngine; 

    template <typename T>
    auto random(T low, T high) -> T 
    {
        return std::uniform_int_distribution<T>{low, high}(randomEngine);
    }
}