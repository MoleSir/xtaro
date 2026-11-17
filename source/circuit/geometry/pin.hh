#pragma once

#include "./coord.hh"
#include "./rectangle.hh"
#include "global/number/number.hh"
#include <string_view>

namespace xtaro::circuit 
{

    class Pin : public Rectangle
    {
    public:
        Pin(std::string_view name, const Coord& offset, const Number& width, const Number& height, Layer layer);
        Pin(std::string_view name, const Coord& leftdown, const Coord& rightup, Layer layer);
        Pin(std::string_view name, const Rectangle& rect);

    public:
        auto name() const -> std::string_view { return this->_name; }
        auto setName(std::string_view name) -> void { this->_name = name; } 

    private:
        std::string_view _name;
    };

}