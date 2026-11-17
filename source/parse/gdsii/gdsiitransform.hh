#pragma once

#include <array>
#include <tuple>
#include <string>
#include <optional>
#include <span>

namespace xtaro::parse::transform
{

    using Coord = std::tuple<double, double>;
    using Rect = std::tuple<Coord, Coord>;
    using Matrix = std::array<std::array<double, 3>, 3>;
    using Vector = std::array<double, 3>;
    using Info = std::tuple<std::string, Matrix>;

    auto dot(const Matrix& matrix, const Vector& vector) -> Vector;
    auto dot(const Matrix& left, const Matrix& right) -> Matrix;

    auto transformCoord(const Coord& coord, const Matrix& matrix) -> Coord;
    auto transformRectangle(const Rect& rect, const Matrix& matrix) -> std::tuple<Coord, Coord>;

    auto unionRectangle(const Rect& rect1, const Rect& rect2) -> Rect;
    auto unionRectangles(std::span<const Rect> rects) -> std::optional<Rect>;

}

