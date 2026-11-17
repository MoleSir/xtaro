#include "gdsiitransform.hh"
#include <cmath>

namespace xtaro::parse::transform
{

    auto dot(const Matrix& matrix, const Vector& vector) -> Vector
    {
        return Vector {
            matrix[0][0] * vector[0] + matrix[0][1] * vector[1] + matrix[0][2] * vector[2],
            matrix[1][0] * vector[0] + matrix[1][1] * vector[1] + matrix[1][2] * vector[2],  
            matrix[2][0] * vector[0] + matrix[2][1] * vector[1] + matrix[2][2] * vector[2],  
        };
    }

    auto dot(const Matrix& left, const Matrix& right) -> Matrix
    {
        auto result {Matrix{}};
        for (std::size_t r {0}; r < 3; ++r) 
        {
            for (std::size_t c {0}; c < 3; ++c)
            {
                result[r][c] = left[r][0] * right[0][c] + left[r][1] * right[1][c] + left[r][2] * right[2][c]; 
            }
        }
        return result;
    }

    auto transformCoord(const Coord& coord, const Matrix& transformMatrix) -> Coord
    {
        const auto& [x, y] {coord};
        auto vector { Vector{x, y, 1.0} };
        auto result { dot(transformMatrix, vector) };
        return Coord{ result[0], result[1] };
    }

    auto transformRectangle(const Rect& rect, const Matrix& transformMatrix) -> std::tuple<Coord, Coord>
    {
        auto [leftBottom, rightTop] = rect;
        auto [x1, y1] { transformCoord(leftBottom, transformMatrix) };
        auto [x2, y2] { transformCoord(rightTop, transformMatrix) };

        auto [minX, maxX] { x1 > x2 ? std::make_tuple(x2, x1) : std::make_tuple(x1, x2) };
        auto [minY, maxY] { y1 > y2 ? std::make_tuple(y2, y1) : std::make_tuple(y1, y2) };

        return { {minX, minY}, {maxX, maxY} };
    }

    auto unionRectangle(const Rect& Rect1, const Rect& Rect2) -> Rect
    {
        auto [leftBottom1, rightTop1] = Rect1;
        auto [minX1, minY1] = leftBottom1;
        auto [maxX1, maxY1] = rightTop1;

        auto [leftBottom2, rightTop2] = Rect2;
        auto [minX2, minY2] = leftBottom2;
        auto [maxX2, maxY2] = rightTop2; 
        
        return {
            Coord {
                minX1 < minX2 ? minX1 : minX2,
                minY1 < minY2 ? minY1 : minY2,  
            },
            Coord {
                maxX1 < maxX2 ? maxX2 : maxX1,
                maxY1 < maxY2 ? maxY2 : maxY1,
            }
        };
    }

    auto unionRectangles(std::span<const Rect> rects) -> std::optional<Rect>
    {
        if (rects.empty())
            return std::nullopt;

        auto iter = rects.begin();
        auto maxRect { *iter };

        for (iter++; iter != rects.end(); ++iter)
            maxRect = unionRectangle(maxRect, *iter);

        return maxRect;
    }

    // auto isCoordInRect(const GDSIIDBCoord& coord, std::span<const GDSIIDBCoord> xy) -> bool
    // {
    //     auto [x, y] = coord;
    //     auto [minX, minY] {xy[0]};
    //     auto [maxX, maxY] {xy[2]}; 

    //     return 
    //         minX < x &&
    //         maxX > x &&
    //         minY < y &&
    //         maxY > y;     
    // }

    // auto isCoordInRect(const Coord& coord, const Rect& Rect) -> bool
    // {
    //     auto [x, y] {coord};
    //     auto [leftBottom, rightTop] {Rect};
    //     auto [minX, minY] {leftBottom};
    //     auto [maxX, maxY] {rightTop}; 

    //     return 
    //         minX < x &&
    //         maxX > x &&
    //         minY < y &&
    //         maxY > y;
    // }

    // uint64_t doubleToReal(double value) 
    // {
    //     if (value == 0) return 0;
    //     uint8_t u8_1 = 0;
    //     if (value < 0) 
    //     {
    //         u8_1 = 0x80;
    //         value = -value;
    //     }
    //     const double fexp = 0.25 * std::log2(value);
    //     double exponent = std::ceil(fexp);
    //     if (exponent == fexp) exponent++;
    //     const uint64_t mantissa = (uint64_t)(value * std::pow(16, 14 - exponent));
    //     u8_1 += (uint8_t)(64 + exponent);
    //     const uint64_t result = ((uint64_t)u8_1 << 56) | (mantissa & 0x00FFFFFFFFFFFFFF);
    //     return result;
    // }

    // double realToDouble(uint64_t real) 
    // {
    //     const int64_t exponent = ((real & 0x7F00000000000000) >> 54) - 256;
    //     const double mantissa = ((double)(real & 0x00FFFFFFFFFFFFFF)) / 72057594037927936.0;
    //     const double result = mantissa * std::exp2((double)exponent);
    //     return (real & 0x8000000000000000) ? -result : result;
    // }

}
