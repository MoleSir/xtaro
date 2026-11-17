#pragma once 

#include <cstdint>

namespace xtaro::parse 
{

    template <typename Value>
    struct GDSIICoord 
    {
        GDSIICoord(Value x, Value y) :
            x{x}, y{y} 
        {
        }

        GDSIICoord() = default;


        Value x{};
        Value y{};
    };

    template <typename Coord>
    struct GDSIIRect 
    {
        GDSIIRect(const Coord& ld, const Coord ru) :
            leftdown{ld},
            rightup{ru}
        {
        }

        GDSIIRect() = default;

        auto contains(const Coord& coord) const -> bool 
        {
            auto [x, y] = coord;
            auto [minX, minY] = this->leftdown;
            auto [maxX, maxY] = this->rightup; 

            return 
                minX < x &&
                maxX > x &&
                minY < y &&
                maxY > y; 
        }

        Coord leftdown{};
        Coord rightup{};
    };
    
    using GDSIIDBCoord = GDSIICoord<std::int32_t>;
    using GDSIIDBRect = GDSIIRect<GDSIIDBCoord>;

    using GDSIIMicroCoord = GDSIICoord<double>;
    using GDSIIMicroRect = GDSIIRect<GDSIIMicroCoord>;

}

