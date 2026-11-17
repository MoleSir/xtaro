#include "./pin.hh"
#include "circuit/geometry/rectangle.hh"

namespace xtaro::circuit
{

    Pin::Pin(std::string_view name, const Coord& offset, const Number& width, const Number& height, Layer layer) :
        Rectangle{offset, width, height, layer},
        _name{name}
    {
    }

    Pin::Pin(std::string_view name, const Coord& leftdown, const Coord& rightup, Layer layer) :
        Rectangle{leftdown, rightup, layer},
        _name{name}
    {
    }

    Pin::Pin(std::string_view name, const Rectangle& rect) :
        Rectangle{rect},
        _name{name}
    {
    }

}