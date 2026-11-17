#include "./label.hh"
#include "circuit/geometry/geometry.hh"
#include <optional>

namespace xtaro::circuit 
{

    Label::Label(std::string_view text, const Coord& offset, Layer layer) :
        Geometry{layer},
        _text{std::move(text)},
        _offset{offset}
    {
    }

    auto Label::getBlockage(Layer layer) const -> std::optional<Blockage> 
    {
        return std::nullopt;
    }

}