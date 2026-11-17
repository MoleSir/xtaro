// #pragma once

// #include "./rectangle.hh"
// #include "circuit/geometry/coord.hh"

// #include <global/number/number.hh>
// #include <global/tech/layer.hh>
// #include <span>
// #include <tuple>
// #include <vector>

// namespace xtaro::circuit 
// {

//     class Via 
//     {
//     public:
//         Via(Layer via, const std::tuple<int, int>& dimensions);

//     public:
//         auto addRectangle(Layer layer, const Coord& offset, const Number& width, const Number& height) -> void;
//         auto place(const Coord& offset) -> void;

//     public:
//         auto rectangles() const -> std::span<const Rectangle> 
//         { return this->_rectangles; }

//         auto width() const -> const Number& 
//         { return this->_width; }

//         auto height() const -> const Number& 
//         { return this->_height; }

//     private:
//         Layer _via;
//         Layer _layerBelow;
//         Layer _layerAbove;
        
//         std::tuple<int, int> _dimensions;

//         std::tuple<RoutingDirection, RoutingDirection> _directions;
//         Number _width;
//         Number _height;

//         Coord _offset;
//         std::vector<Rectangle> _rectangles;
//     };

// }