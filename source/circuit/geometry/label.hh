#pragma once

#include "./geometry.hh"
#include "circuit/geometry/coord.hh"
#include <string_view>

namespace xtaro::circuit 
{

    class Label : public Geometry 
    {
    public:
        Label(std::string_view text, const Coord& offset, Layer layer);

    public:
        virtual auto getBlockage(Layer layer) const -> std::optional<Blockage> override;

    public:
        auto text() const -> std::string_view { return this->_text; }
        auto offset() const -> const Coord& { return this->_offset; }

    private:
        std::string_view _text;
        Coord _offset;
    };

}