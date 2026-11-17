// #include "./contact.hh"
// #include "./coord.hh"
// #include <utility/debug/debug.hh>
// #include <global/tech/layer.hh>
// #include <global/tech/tech.hh>

// namespace xtaro::circuit 
// {

//     Via::Via(Layer via, const std::tuple<int, int>& dimensions) :
//         _via{via},
//         _layerAbove{layerAbove(via)},
//         _layerBelow{layerAbove(via)},

//         _dimensions{dimensions},
        
//         _directions{tech->layerDirection(_layerAbove), tech->layerDirection(_layerBelow)},
//         _width{},
//         _height{},

//         _offset{},
//         _rectangles{}
//     {
//         // // setup_layers
//         // // layer1 => via => layer2

//         // // setup_layout_constants
//         // auto contactWidth = tech->layerWidth(this->_stack.layerVia);
//         // auto contactSpacing = tech->layerSpacing(this->_stack.layerVia);
//         // auto contactPitch = contactWidth + contactSpacing;

//         // auto contactArrayWidth  = contactWidth + (std::get<0>(this->_dimensions) - 1) * contactPitch;
//         // auto contactArrayHeight = contactWidth + (std::get<1>(this->_dimensions) - 1) * contactPitch;

//         // auto layer1Width = tech->layerWidth(this->_stack.layer1);
//         // auto layer1Enclosure = tech->drc.enclose(this->_stack.layer1, this->_stack.layerVia);
//         // auto layer1Extent = tech->drc.extent(this->_stack.layer1, this->_stack.layerVia);
//         // auto layer1Minarea = tech->drc.area(this->_stack.layer1);

//         // auto layer2Width = tech->layerWidth(this->_stack.layer2);
//         // auto layer2Enclosure = tech->drc.enclose(this->_stack.layer2, this->_stack.layerVia);
//         // auto layer2Extent = tech->drc.extent(this->_stack.layer2, this->_stack.layerVia);
//         // auto layer2Minarea = tech->drc.area(this->_stack.layer2);

//         // // MARK
//         // auto layer1HoriEnclosure = 0._o;
//         // auto layer1VertEnclosure = 0._o;
//         // if (std::get<0>(this->_directions) == RoutingDirection::Hori) 
//         // {
//         //     layer1HoriEnclosure = layer1Enclosure.max((layer1Width - contactArrayWidth) / 2.0);
//         //     layer1VertEnclosure = layer1Extent.max((layer1Width - contactArrayHeight) / 2.0);
//         // }
//         // else 
//         // {
//         //     layer1HoriEnclosure = layer1Extent.max((layer1Width - contactArrayWidth) / 2.0);
//         //     layer1VertEnclosure = layer1Enclosure.max((layer1Width - contactArrayHeight) / 2.0);
//         // }
        
//         // auto layer2HoriEnclosure = 0._o;
//         // auto layer2VertEnclosure = 0._o;
//         // if (std::get<1>(this->_directions) == RoutingDirection::Vert) 
//         // {
//         //     layer2HoriEnclosure = layer2Enclosure.max((layer2Width - contactArrayWidth) / 2.0);
//         //     layer2VertEnclosure = layer2Extent.max((layer2Width - contactArrayHeight) / 2.0);
//         // }
//         // else 
//         // {
//         //     layer2HoriEnclosure = layer2Extent.max((layer2Width - contactArrayWidth) / 2.0);
//         //     layer2VertEnclosure = layer2Enclosure.max((layer2Width - contactArrayHeight) / 2.0);
//         // } 

//         // // create_contact_array
//         // auto viaCoord = Coord {
//         //     layer1HoriEnclosure.max(layer2HoriEnclosure),
//         //     layer1VertEnclosure.max(layer2VertEnclosure)
//         // };
//         // for (int row = 0; row < std::get<0>(this->_dimensions); ++row) 
//         // {
//         //     for (int col = 0; col < std::get<1>(this->_dimensions); ++col) 
//         //     {
//         //         this->addRectangle(
//         //             this->_stack.layerVia,
//         //             viaCoord + Coord { row * contactPitch, col * contactPitch },
//         //             contactWidth, 
//         //             contactWidth
//         //         );
//         //     }
//         // }

//         // // create_first_layer_enclosure
//         // auto layer1Coord = Coord {
//         //     (0._o).max(layer2HoriEnclosure - layer1HoriEnclosure),
//         //     (0._o).max(layer2VertEnclosure - layer1VertEnclosure)
//         // };

//         // auto layer1RectWidth  = layer1Width.max(contactArrayWidth + 2. * layer1HoriEnclosure);
//         // auto layer1RectHeight = layer1Width.max(contactArrayHeight + 2. * layer1VertEnclosure);

//         // // TODO area

//         // this->addRectangle(
//         //     this->_stack.layer1, 
//         //     layer1Coord,
//         //     layer1RectWidth,
//         //     layer1RectHeight
//         // );

//         // // create_second_layer_enclosure
//         // auto layer2Coord = Coord {
//         //     (0._o).max(layer1HoriEnclosure - layer2HoriEnclosure),
//         //     (0._o).max(layer1VertEnclosure - layer2VertEnclosure)
//         // };

//         // auto layer2RectWidth  = layer2Width.max(contactArrayWidth + 2. * layer2HoriEnclosure);
//         // auto layer2RectHeight = layer2Width.max(contactArrayHeight + 2. * layer2VertEnclosure);

//         // // TODO area

//         // this->addRectangle(
//         //     this->_stack.layer2, 
//         //     layer2Coord,
//         //     layer2RectWidth,
//         //     layer2RectHeight
//         // );

//         // this->_width = (layer1Coord.x + layer1RectWidth).max(layer1Coord.x + layer2RectWidth);
//         // this->_height = (layer1Coord.y + layer1RectHeight).max(layer1Coord.y + layer2RectHeight);
//     }

//     auto Via::addRectangle(Layer layer, const Coord& offset, const Number& width, const Number& height) -> void
//     {
//         this->_rectangles.emplace_back(
//             offset + this->_offset, width, height, layer
//         );
//     }

//     auto Via::place(const Coord& offset) -> void
//     {
//         this->_offset = offset;
//     }

// }