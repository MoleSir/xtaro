#include "./rectangle.hh"
#include "circuit/geometry/blockage.hh"
#include "circuit/geometry/coord.hh"
#include "circuit/geometry/geometry.hh"
#include "utility/debug/debug.hh"
#include <optional>

namespace xtaro::circuit 
{
 
    Rectangle::Rectangle(const Coord& offset, const Number& width, const Number& height, Layer layer) :
        Geometry{layer},
        _leftdown{offset},
        _rightup{Coord{offset.x + width, offset.y + height}}
    {
    }

    Rectangle::Rectangle(const Coord& leftdown, const Coord& rightup, Layer layer) :
        Geometry{layer},
        _leftdown{leftdown},
        _rightup{rightup}
    {
        // Check rect
        debug::checkf(leftdown.x <= rightup.x, "Rectangle leftdown x '{}' > rightup x'{}'", leftdown.x, rightup.x);
        debug::checkf(leftdown.y <= rightup.y, "Rectangle leftdown y '{}' > rightup y'{}'", leftdown.y, rightup.y);
    }

    auto Rectangle::getBlockage(Layer layer) const -> std::optional<Blockage> 
    {
        if (this->layer() == layer)
            return Blockage{.leftdown = this->_leftdown, .rightup = this->_rightup};
        return std::nullopt;
    }


    auto Rectangle::inflated(const Number& spacing) const -> Rectangle
    {
        auto spacingCoord = Coord{spacing, spacing};

        auto nrect = *this;
        nrect._rightup -= spacingCoord;
        nrect._leftdown += spacingCoord;

        return nrect;
    }

    auto Rectangle::intersection(const Rectangle& other) const -> std::optional<Rectangle>
    {
        if (!this->overlaps(other))
            return std::nullopt;

        auto minX = this->_leftdown.x.max(other._leftdown.x);
        auto maxX = this->_rightup.x.min(other._rightup.x);

        auto minY = this->_leftdown.y.max(other._leftdown.y);
        auto maxY = this->_rightup.y.min(other._rightup.y);

        if (maxX == minX || maxY == minY) 
            return std::nullopt;
            
        auto nrect = *this;
        nrect._leftdown = Coord{minX, minY};
        nrect._rightup  = Coord{maxX, maxY};
        return nrect;
    }

    auto Rectangle::xoverlaps(const Rectangle& other) const -> bool
    {
        auto [ld, ru] = this->rectangle();
        auto [old, oru] = other.rectangle(); 

        if ((ld.x >= old.x && ld.x <= oru.x) || 
            (ru.x >= old.x && ru.x <= oru.x) ||
            (old.x >= ld.x && old.x <= ru.x) ||
            (oru.x >= ld.x && oru.x <= ru.x)
        )
        {
            return true;
        }
        return false;
    }

    auto Rectangle::yoverlaps(const Rectangle& other) const -> bool
    {
        auto [ld, ru] = this->rectangle();
        auto [old, oru] = other.rectangle(); 

        if ((ld.y >= old.y && ld.y <= oru.y) || 
            (ru.y >= old.y && ru.y <= oru.y) ||
            (old.y >= ld.y && old.y <= ru.y) ||
            (oru.y >= ld.y && oru.y <= ru.y)
        )
        {
            return true;
        }
        return false;
    }

    auto Rectangle::overlaps(const Rectangle& other) const -> bool
    {
        return this->sameLayer(other) &&
                this->xoverlaps(other) && 
                this->yoverlaps(other);
    }

    auto Rectangle::xcontains(const Rectangle& other) const -> bool
    {
        auto [ld, ru] = this->rectangle();
        auto [old, oru] = other.rectangle(); 

        return (old.x >= ld.x && oru.x <= ru.x);
    }

    auto Rectangle::ycontains(const Rectangle& other) const -> bool
    {
        auto [ld, ru] = this->rectangle();
        auto [old, oru] = other.rectangle(); 

        return (old.y >= ld.y && oru.y <= ru.y);
    }

    auto Rectangle::contains(const Rectangle& other) const -> bool
    {
        if (this == &other) 
            return true;

        return this->sameLayer(other) && 
                this->xcontains(other) && 
                this->ycontains(other);
    }

    auto Rectangle::contains(std::span<const Rectangle> pins) const -> bool
    {
        for (auto& pin : pins) 
            if (this->contains(pin)) 
                return true;
        return false;
    }

    auto Rectangle::area() const -> Number
    {
        return this->width() * this->height();
    }

    auto Rectangle::height() const -> Number
    {
        auto [ld, ru] = this->rectangle();
        return (ru.y - ld.y).abs();
    }

    auto Rectangle::width() const -> Number
    {
        auto [ld, ru] = this->rectangle();
        return (ru.x - ld.x).abs();
    }

    auto Rectangle::transform(const Coord& offset, MirrorType mirror, double angle) -> void
    {
        this->_leftdown = this->_leftdown.transform(offset, mirror, angle);
        this->_rightup  = this->_rightup.transform(offset, mirror, angle);
        this->normalize();
    }

    auto Rectangle::normalize() -> void
    {
        const auto& [ld, ru] = this->rectangle();

        auto leftdown = Coord{ ld.x.min(ru.x), ld.y.min(ru.y) };
        auto rightup  = Coord{ ld.x.max(ru.x), ld.y.max(ru.y) };

        this->_leftdown = leftdown;
        this->_rightup = rightup;
    }

    auto Rectangle::rectangle() const -> std::tuple<const Coord&, const Coord&>
    {
        return {this->_leftdown, this->_rightup};
    }
    
    auto Rectangle::leftdown() const -> Coord
    {
        return this->_leftdown;
    }

    auto Rectangle::leftup() const -> Coord
    {
        return Coord {
            this->_leftdown.x,
            this->_rightup.y
        };
    }

    auto Rectangle::rightup() const -> Coord
    {
        return this->_rightup;
    }

    auto Rectangle::rightdown() const -> Coord
    {
        return Coord {
            this->_rightup.x,
            this->_leftdown.y
        };
    }

    auto Rectangle::maxX() const -> Number
    {
        return this->_rightup.x;
    }

    auto Rectangle::minX() const -> Number
    {
        return this->_leftdown.x;
    }

    auto Rectangle::cenX() const -> Number
    {
        return (this->_rightup.x + this->_leftdown.x) / 2.0;
    }

    auto Rectangle::maxY() const -> Number
    {
        return this->_rightup.y;
    }

    auto Rectangle::minY() const -> Number
    {
        return this->_leftdown.y;
    }

    auto Rectangle::cenY() const -> Number
    {
        return (this->_rightup.y + this->_leftdown.y) / 2.0;
    }

    auto Rectangle::center() const -> Coord
    {
        return Coord {
            this->cenX(),
            this->cenY()
        };
    }

}