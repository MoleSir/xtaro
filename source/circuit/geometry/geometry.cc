#include "./geometry.hh"
#include "global/tech/tech.hh"
#include <utility/debug/debug.hh>

namespace xtaro::circuit 
{

    Geometry::Geometry(Layer layer) :
        _layer{layer},
        _layerMask{tech->layerMask(layer)}
    {
    }

    auto Geometry::sameLayer(const Geometry& other) const -> bool
    {
        return this->_layer == other._layer;
    }

}