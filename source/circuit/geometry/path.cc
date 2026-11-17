#include "./path.hh"
#include "circuit/geometry/geometry.hh"
#include "utility/debug/debug.hh"
#include <vector>

namespace xtaro::circuit 
{

    Path::Path(std::span<const Coord> coords, const Number& width, Layer layer) :
        Geometry{layer},
        _coords{},
        _width{width}
    {
        for (auto& coord : coords) 
            this->_coords.emplace_back(coord.snapToGrid());
    }

    auto Path::getBlockage(Layer layer) const -> std::optional<Blockage> 
    {
        debug::unimplement("Path::getBlockages");
    }

}