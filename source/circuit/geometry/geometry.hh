#pragma once

#include "./blockage.hh"
#include <global/tech/layer.hh>
#include <cstdint>
#include <optional>

namespace xtaro::circuit 
{

    class Geometry 
    {
    public:
        Geometry(Layer layer);

    public:
        virtual auto getBlockage(Layer layer) const -> std::optional<Blockage> = 0;

    public:
        auto sameLayer(const Geometry& other) const -> bool;
        auto layer() const -> Layer { return this->_layer; }
        auto layerMask() const -> std::int16_t { return this->_layerMask; }

    private:
        Layer _layer;
        std::int16_t _layerMask;
    };

}