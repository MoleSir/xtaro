#include "./blockage.hh"
#include "circuit/geometry/rectangle.hh"
#include "global/tech/layer.hh"
#include <cassert>

namespace xtaro::circuit 
{

    auto Blockage::transform(const Coord& offset, MirrorType mirror, double angle) const -> Blockage 
    {
        // Yet we not support path, use rect to transform
        auto rect = Rectangle{this->leftdown, this->rightup, Layer::Metal1};
        rect.transform(offset, mirror, angle);
        auto blockage = rect.getBlockage(Layer::Metal1);
        assert(blockage.has_value());
        return *blockage;
    }

}