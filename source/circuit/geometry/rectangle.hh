#pragma once

#include "./geometry.hh"
#include "circuit/geometry/coord.hh"
#include "global/number/number.hh"
#include <span>

namespace xtaro::circuit 
{

    class Rectangle : public Geometry 
    {
    public:
        Rectangle(const Coord& offset, const Number& width, const Number& height, Layer layer);
        Rectangle(const Coord& leftdown, const Coord& rightup, Layer layer);
        
    public:
        virtual auto getBlockage(Layer layer) const -> std::optional<Blockage>  override;

    public:
        auto inflated(const Number& spacing) const -> Rectangle;
        auto intersection(const Rectangle& other) const -> std::optional<Rectangle>;
    
        auto xoverlaps(const Rectangle& other) const -> bool;
        auto yoverlaps(const Rectangle& other) const -> bool;
        auto overlaps(const Rectangle& other) const -> bool;

        auto xcontains(const Rectangle& other) const -> bool;
        auto ycontains(const Rectangle& other) const -> bool;
        auto contains(const Rectangle& other) const -> bool;
        auto contains(std::span<const Rectangle> pins) const -> bool;

        auto area() const -> Number;

        auto height() const -> Number;
        auto width() const -> Number;

    public:
        auto transform(const Coord& offset, MirrorType mirror, double angle) -> void;
        auto normalize() -> void;

    public:
        auto rectangle() const -> std::tuple<const Coord&, const Coord&>;

        auto leftdown() const -> Coord;
        auto leftup() const -> Coord;
        auto rightup() const -> Coord;
        auto rightdown() const -> Coord;

        auto maxX() const -> Number;
        auto minX() const -> Number;
        auto cenX() const -> Number;

        auto maxY() const -> Number;
        auto minY() const -> Number;
        auto cenY() const -> Number;

        auto center() const -> Coord;

    private:
        Coord _leftdown;
        Coord _rightup;
    };

}