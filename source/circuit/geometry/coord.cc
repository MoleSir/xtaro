#include "./coord.hh"
#include <parse/gdsii/gdsiicoord.hh>
#include <utility/misc/math.hh>
#include <global/tech/tech.hh>
#include <global/number/number.hh>
#include <cstdint>
#include <format>
#include <cmath>
#include <vector>

namespace xtaro
{

    Coord::Coord() : 
        x{},
        y{}
    {
    }

    Coord::Coord(const Number& x, const Number& y) :
        x{x},
        y{y}
    {
    }

    auto Coord::snapToGrid() const -> Coord
    {
        return Coord {
            Coord::snapOffsetToGrid(this->x),
            Coord::snapOffsetToGrid(this->y)
        };
    }

    auto Coord::snapOffsetToGrid(const Number& v) -> Number
    {
        auto grid = tech->gridWidth();
        auto vv = v.unitValue(grid.unit());
        // gradvalue should be integer!
        auto gradvalue = grid.value();
        return Number{
            static_cast<int>(vv / gradvalue) * gradvalue,
            grid.unit()
        };

        auto girdSize = std::ceil(v.divide(grid));
        return girdSize * grid;
    }

    auto Coord::snapToGrid(std::span<const Coord> coords) -> std::vector<Coord>
    {
        auto res = std::vector<Coord>{};
        for (auto& coord : coords) 
            res.emplace_back(coord.snapToGrid());
        return res;
    }

    auto Coord::rotate() const -> Coord
    {
        return Coord{this->y, this->x};
    }

    auto Coord::scale(double factor) -> Coord
    {
        return Coord {
            factor * this->x,
            factor * this->y
        };
    }

    auto Coord::scale(double factorX, double factorY) -> Coord
    {
        return Coord {
            factorX * this->x,
            factorY * this->y
        };   
    }

    auto Coord::rotateScale(double factor) -> Coord
    {
        return Coord {
            factor * this->y,
            factor * this->x
        };
    }

    auto Coord::rotateScale(double factorX, double factorY) -> Coord
    {
        return Coord {
            factorX * this->y,
            factorY * this->x
        };
    }

    auto Coord::transform(std::span<const Coord> coords, const Coord& offset, MirrorType mirror, double rotate) -> std::vector<Coord>
    {
        auto newCoords = std::vector<Coord>{};
        for (const auto& coord : coords) 
            newCoords.emplace_back(coord.transform(offset, mirror, rotate));
        return newCoords;
    }

    auto Coord::transform(const Coord& offset, MirrorType mirror, double angle) const -> Coord
    {
        double mirrorValue = 1.;
        switch (mirror) 
        {
            case MirrorType::MirrorY: 
                mirrorValue = -1;
                break;
            case MirrorType::MirrorX:
                mirrorValue = -1;
                angle += 180.0;
                break;
            case MirrorType::MirrorXY:
                mirrorValue = 1;
                angle += 180.0;
                break;
            case MirrorType::None:
                break;
        }

        auto radians = util::radians(angle);

        auto newX = std::cos(radians) * this->x - std::sin(radians) * mirrorValue * this->y;
        auto newY = std::sin(radians) * this->x + std::cos(radians) * mirrorValue * this->y;
        return Coord {offset.x + newX, offset.y + newY};
    }

    auto Coord::floor() const -> Coord
    {
        return Coord {
            this->x.floor(),
            this->y.floor()
        };
    }

    auto Coord::ceil() const -> Coord
    {
        return Coord {
            this->x.ceil(),
            this->y.ceil()
        };
    }

    auto Coord::round() const -> Coord
    {
        return Coord {
            this->x.round(),
            this->y.round()
        };
    }

    auto Coord::max(const Coord& other) const -> Coord
    {
        return Coord {
            this->x.max(other.x),
            this->y.max(other.y)
        };
    }

    auto Coord::min(const Coord& other) const -> Coord
    {
        return Coord {
            this->x.min(other.x),
            this->y.min(other.y)
        };
    }

    auto Coord::toString() const -> std::string
    {
        return std::format("{}", *this);
    }

    auto Coord::toGDSIIMicroCoord() const -> parse::GDSIIMicroCoord
    {
        return parse::GDSIIMicroCoord{
            this->x.microValue(), this->y.microValue()
        };
    }

    auto operator + (const Coord& c1, const Coord& c2) -> Coord
    {
        return Coord {c1.x + c2.x, c1.y + c2.y};
    }

    auto operator - (const Coord& c1, const Coord& c2) -> Coord
    {
        return Coord {c1.x - c2.x, c1.y - c2.y};
    }

    auto operator == (const Coord& c1, const Coord& c2) -> bool
    {
        return c1.x == c2.x && c1.y == c2.y;
    }

    auto operator != (const Coord& c1, const Coord& c2) -> bool
    {
        return !(c1 == c2);
    }

    auto operator += (Coord& c1, const Coord& c2) -> void
    {
        c1.x += c2.x;
        c1.y += c2.y;
    }

    auto operator -= (Coord& c1, const Coord& c2) -> void
    {
        c1.x -= c2.x;
        c1.y -= c2.y;
    }

}

auto std::hash<xtaro::Coord>::operator() (const xtaro::Coord& c) const noexcept -> std::uint64_t
{
    auto hash = std::hash<xtaro::Number>{};
    return hash(c.x) ^ hash(c.y);
}