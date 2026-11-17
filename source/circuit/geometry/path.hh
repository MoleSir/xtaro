#pragma once

#include "./geometry.hh"
#include "circuit/geometry/coord.hh"
#include "global/number/number.hh"
#include <span>
#include <vector>

namespace xtaro::circuit 
{

    class Path : public Geometry 
    {
    public:
        Path(std::span<const Coord> coords, const Number& width, Layer layer);

    public:
        virtual auto getBlockage(Layer layer) const -> std::optional<Blockage>  override;

    public:
        auto coords() const -> std::span<const Coord> { return this->_coords; }
        auto width() const -> const Number& { return this->_width; }

    private:
        std::vector<Coord> _coords;
        Number _width;
    };

}