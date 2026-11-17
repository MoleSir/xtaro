#include "./circuit.hh"
#include "./port.hh"
#include "circuit/geometry/blockage.hh"
#include "global/tech/layer.hh"
#include <circuit/geometry/coord.hh>
#include <filesystem>
#include <global/number/number.hh>
#include <global/tech/tech.hh>
#include <global/config/config.hh>

#include <utility/misc/file.hh>
#include <utility/misc/string.hh>
#include <utility/debug/debug.hh>

#include <string_view>
#include <vector>
#include <algorithm>

namespace xtaro::circuit
{

    Circuit::Arguments::Arguments(std::string_view circuitType) noexcept : 
        _circuitType {circuitType}
    {
    }

    auto Circuit::Arguments::circuitType() const noexcept -> std::string_view
    {
        return this->_circuitType;
    }

    Circuit::Circuit(std::string_view name) :
        _name{name},
        _ports{},
        _paths{},
        _rectangles{},
        _labels{},
        _pins{},
        
        _spiceFilePath{},
        _gdsiiFilePath{}
    {
    }

    Circuit::~Circuit() noexcept
    {
    }

    auto Circuit::createSelf() -> void 
    {
        this->createPorts();
        this->createNetlist();
        if (config->buildBackend) 
            this->createLayout();
    }

    auto Circuit::spiceFilePath() const -> const std::filesystem::path&
    { 
        if (!this->_spiceFilePath.has_value())
            debug::exceptionf("No spice file in circuit '{}'", this->name());
        return *this->_spiceFilePath; 
    }

    auto Circuit::hasSpiceFilePath() const -> bool 
    { 
        return this->_spiceFilePath.has_value(); 
    }

    auto Circuit::setSpiceFilePath(std::filesystem::path filepath) -> void 
    { 
        this->_spiceFilePath.emplace(std::move(filepath)); 
    }

    auto Circuit::gdsiiFilePath() const -> const std::filesystem::path&
    { 
        if (!this->_gdsiiFilePath.has_value())
            debug::exceptionf("No gdsii file in circuit '{}'", this->name());
        return *this->_gdsiiFilePath; 
    }

    auto Circuit::hasGDSIIFilePath() const -> bool 
    { 
        return this->_gdsiiFilePath.has_value(); 
    }

    auto Circuit::setGDSIIFilePath(std::filesystem::path filepath) -> void 
    { 
        this->_gdsiiFilePath.emplace(std::move(filepath)); 
    }

    void Circuit::addPort(std::string_view name, PortType type)
    {
        this->_ports.emplace_back(name, type);
    }

    void Circuit::addPorts(const std::vector<std::string_view>& portsName, PortType portType)
    {
        for (const std::string_view& portName : portsName)
            this->addPort(portName, portType);
    }

    auto Circuit::portsName() const -> std::vector<std::string_view>
    {
        std::vector<std::string_view> names{};     
        std::transform(
            this->_ports.cbegin(), this->_ports.cend(), 
            std::back_inserter(names), 
            [] (const Port& port) -> std::string_view { return port.name(); }
        );
        return names;
    }

    auto Circuit::outputPortsName() const -> std::vector<std::string_view>
    {
        std::vector<std::string_view> names{};
        for (const auto& port : this->_ports)
        {
            if (port.type() == PortType::Output)
                names.emplace_back(port.name());
        }
        return names;
    }

    auto Circuit::copyPort() const -> std::vector<Port>
    {
        auto ports = std::vector<Port>{};
        std::transform(
            this->_ports.cbegin(), this->_ports.cend(), std::back_inserter(ports),
            [] (const Port& port) -> Port {
                return Port{port.name(), port.type()};
            }
        );
        return ports;
    } 

    auto Circuit::portType(std::string_view name) const -> PortType 
    {
        for (const auto& port : this->_ports) 
        {
            if (port.name() == name) {
                return port.type();
            }
        }
        debug::exceptionf("No exit '{}' port in circuit '{}'", name, this->_name);
    }

    auto Circuit::getPin(std::string_view name) -> const Pin& 
    {
        auto res = this->_pins.find(name);
        if (res == this->_pins.end() || res->second.empty())
            debug::exceptionf("No exit pin '{}' in circuit '{}'", name, this->_name);
        return res->second.front();
    }

    auto Circuit::getPins(std::string_view name) -> std::span<const Pin>
    {
        auto res = this->_pins.find(name);
        if (res == this->_pins.end() || res->second.empty())
            debug::exceptionf("No exit pin '{}' in circuit '{}'", name, this->_name);
        return res->second;
    }

    auto Circuit::doAddPin(Layer layer, std::string_view name, const Coord& _leftdown, const Number& _width, const Number& _height) -> Pin&
    {
        auto leftdown = _leftdown.snapToGrid();
        auto width = Coord::snapOffsetToGrid(_width);
        auto height = Coord::snapOffsetToGrid(_height);
        debug::debugf("Add a pin '{}' in '{}', with width '{}', height '{}' in layer '{}'", name, leftdown, width, height, layer);
        auto& pinList = this->_pins.emplace(name, std::vector<Pin>{}).first->second;
        return pinList.emplace_back(name, leftdown, width, height, layer);
    }

    auto Circuit::doAddRectangle(Layer layer, const Coord& _leftdown, const Number& _width, const Number& _height) -> Rectangle&
    {
        auto leftdown = _leftdown.snapToGrid();
        auto width = Coord::snapOffsetToGrid(_width);
        auto height = Coord::snapOffsetToGrid(_height);
        debug::debugf("Add a rectangle in '{}', with width '{}', height '{}' in layer '{}'", leftdown, width, height, layer);  
        return this->_rectangles.emplace_back(leftdown, width, height, layer);
    }

    auto Circuit::addRectangle(Layer layer, const Coord& leftdown, std::optional<Number> widthOpt, std::optional<Number> heightOpt) -> Rectangle&
    {        
        auto width = this->defaultWidth(layer, widthOpt);
        auto height = this->defaultWidth(layer, heightOpt);

        return this->doAddRectangle(layer, leftdown, width, height);
    }

    auto Circuit::addRectangleCenter(Layer layer, const Coord& center, std::optional<Number> widthOpt, std::optional<Number> heightOpt) -> Rectangle&
    {
        auto width = this->defaultWidth(layer, widthOpt);
        auto height = this->defaultWidth(layer, heightOpt);
        auto leftdown = Coord { center.x - (0.5 * width), center.y - (0.5 * height) };

        return this->doAddRectangle(layer, leftdown, width, height);
    }

    auto Circuit::addRectangleSegment(Layer layer, const Coord& begin, const Coord& end, std::optional<Number> widthOpt) -> Rectangle&
    {
        debug::checkf(
        begin.x.approximate(end.x) || begin.y.approximate(end.y), 
            "Nonrectilinear between coord '{}' and '{}'",
            begin, end
        );

        auto width = this->defaultWidth(layer, widthOpt);
        
        auto boxWdith  = begin.x.max(end.x) - begin.x.min(end.x);
        auto boxHeight = begin.y.max(end.y) - begin.y.min(end.y);
        auto leftdown = Coord { begin.x.min(end.x), begin.y.min(end.y) };
    
        if (boxHeight.value() == 0)
            leftdown.y -= 0.5 * width;
        if (boxWdith.value() == 0) 
            leftdown.x -= 0.5 * width;

        return this->doAddRectangle(layer, leftdown, boxWdith.max(width), boxHeight.max(width));
    }

    auto Circuit::addRectangleSegmentCenter(Layer layer, const Coord& begin, const Coord& end, std::optional<Number> widthOpt) -> Rectangle& 
    {
        debug::checkf(
            begin.x == end.x || begin.y == end.y ||
            begin.x.approximate(end.x) || begin.y.approximate(end.y), 
            "Nonrectilinear between coord '{}' and '{}'",
            begin, end
        );

        auto width = this->defaultWidth(layer, widthOpt);
        
        auto boxWdith  = begin.x.max(end.x) - begin.x.min(end.x);
        auto boxHeight = begin.y.max(end.y) - begin.y.min(end.y);

        auto leftdown = Coord { 
            begin.x.min(end.x) - 0.5 * width, 
            begin.y.min(end.y) - 0.5 * width 
        };
    
        if (boxHeight.value() == 0)
            // begin.y == end.y
            return this->doAddRectangle(layer, leftdown, boxWdith + width, width);
        
        // begin.x == end.x
        return this->doAddRectangle(layer, leftdown, width, boxHeight + width);
    }

    auto Circuit::addPin(Layer layer, std::string_view name, const Coord& leftdown, std::optional<Number> widthOpt, std::optional<Number> heightOpt) -> Pin&
    {
        auto width = this->defaultWidth(layer, widthOpt);
        auto height = this->defaultWidth(layer, heightOpt);
        return this->doAddPin(layer, name, leftdown, width, height);
    }

    auto Circuit::addPinCenter(Layer layer, std::string_view name, const Coord& center, std::optional<Number> widthOpt, std::optional<Number> heightOpt) -> Pin&
    {
        auto width = this->defaultWidth(layer, widthOpt);
        auto height = this->defaultWidth(layer, heightOpt);
        auto leftdown = center - Coord {0.5 * width, 0.5 * height};

        return this->doAddPin(layer, name, leftdown, width, height);
    }

    auto Circuit::addPinSegment(Layer layer, std::string_view name, const Coord& begin, const Coord& end, std::optional<Number> widthOpt) -> Pin&
    {
        debug::checkf(
        begin.x == end.x || begin.y == end.y, 
            "Nonrectilinear between coord '{}' and '{}'",
            begin, end
        );

        auto width = this->defaultWidth(layer, widthOpt);
        
        auto boxWdith  = begin.x.max(end.x) - begin.x.min(end.x);
        auto boxHeight = begin.y.max(end.y) - begin.y.min(end.y);
        auto leftdown = Coord { begin.x.min(end.x), begin.y.min(end.y) };
    
        if (boxHeight.value() == 0)
            leftdown.y -= 0.5 * width;
        if (boxWdith.value() == 0) 
            leftdown.x -= 0.5 * width;

        return this->doAddPin(layer, name, leftdown, boxWdith.max(width), boxHeight.max(width));
    }

    auto Circuit::addPinSegmentCenter(Layer layer, std::string_view name, const Coord& begin, const Coord& end, std::optional<Number> widthOpt) -> Rectangle&
    {
        debug::checkf(
        begin.x == end.x || begin.y == end.y, 
            "Nonrectilinear between coord '{}' and '{}'",
            begin, end
        );

        auto width = this->defaultWidth(layer, widthOpt);
        
        auto boxWdith  = begin.x.max(end.x) - begin.x.min(end.x);
        auto boxHeight = begin.y.max(end.y) - begin.y.min(end.y);

        auto leftdown = Coord { 
            begin.x.min(end.x) - 0.5 * width, 
            begin.y.min(end.y) - 0.5 * width 
        };
    
        if (boxHeight.value() == 0)
            // begin.y == end.y
            return this->doAddPin(layer, name, leftdown, boxWdith, width);
            
        // begin.x == end.x
        return this->doAddPin(layer, name, leftdown, width, boxHeight);
    }

    auto Circuit::addLabel(Layer layer, std::string_view name, const Coord& _offset) -> Label&
    {   
        auto offset = _offset.snapToGrid();
        debug::debugf("Add a label '{}' in '{}', in layer '{}'", name, offset, layer);
        return this->_labels.emplace_back(name, offset, layer);
    }
    
    auto Circuit::addLabelPin(Layer layer, std::string_view name, const Coord& leftdown, std::optional<Number> widthOpt, std::optional<Number> heightOpt) -> void
    {
        auto width = this->defaultWidth(layer, widthOpt);
        auto height = this->defaultWidth(layer, heightOpt);

        this->doAddPin(layer, name, leftdown, width, height);
        this->addLabel(layer, name, leftdown + Coord{ 0.5 * width, 0.5 * height});
    }

    auto Circuit::addLabelPinCenter(Layer layer, std::string_view name, const Coord& center, std::optional<Number> widthOpt, std::optional<Number> heightOpt) -> void 
    {
        auto width = this->defaultWidth(layer, widthOpt);
        auto height = this->defaultWidth(layer, heightOpt);
        auto leftdown = center - Coord {0.5 * width, 0.5 * height};

        this->doAddPin(layer, name, leftdown, width, height);
        this->addLabel(layer, name, leftdown + Coord{ 0.5 * width, 0.5 * height});
    }

    auto Circuit::routePathCenter(Layer layer, std::span<const Coord> coords, std::optional<Number> widthOpt) -> void
    {
        debug::checkf(coords.size() > 1, "A Path with coord less then 2");

        auto width = this->defaultWidth(layer, widthOpt);
     
        // create_rectilinear_route
        auto snapCoords = Coord::snapToGrid(coords);

        auto newCoords = std::vector<Coord>{};        
        for (auto index = 0; index < snapCoords.size() - 1; ++index) 
        {
            auto& curCoord = snapCoords[index];
            auto& nextCoord = snapCoords[index + 1];
            if (curCoord != nextCoord)
                newCoords.emplace_back(curCoord);

            if (curCoord.x != nextCoord.x && curCoord.y != nextCoord.y)
                newCoords.emplace_back( curCoord.x, nextCoord.y );
        }
        newCoords.emplace_back(snapCoords.back());

        debug::checkf(newCoords.size() > 1, "A Path after expand with coord less then 2");

        // connect_corner
        for (auto index = 0; index < newCoords.size() - 2; ++index)
        {
            auto& c1 = newCoords[index];
            auto& corner = newCoords[index + 1];
            auto& c2 = newCoords[index + 2];

            if (c1.x == c2.x || c1.y == c2.y) 
                continue;
            
            // auto corner = Coord { offset.x - (0.5 * width), offset.y - (0.5 * width)};
            // this->addRectangle(layer, corner, width, width);
            this->addRectangleCenter(layer, corner, width, width);
        }

        // create_rectangles
        for (auto index = 0; index < newCoords.size() - 1; ++index) 
        {
            auto& curCoord = newCoords[index];
            auto& nextCoord = newCoords[index + 1];
        
            if (curCoord.x != nextCoord.x) 
            {
                auto lineLength = (nextCoord.x - curCoord.x).abs() + width;
                // auto offset = Coord { curCoord.x, curCoord.y - 0.5 * width};
                auto center = Coord { (curCoord.x + nextCoord.x) / 2., curCoord.y };
                // if (lineLength.value() < 0)
                //     offset.x += lineLength;
                this->addRectangleCenter(layer, center, lineLength, width);
            }
            else if (curCoord.y != nextCoord.y)
            {
                auto lineLength = (nextCoord.y - curCoord.y).abs() + width;
                // auto offset = Coord { curCoord.x - 0.5 * width, curCoord.y };
                auto center = Coord {curCoord.x, (curCoord.y + nextCoord.y) / 2.};
                // if (lineLength.value() < 0)
                //     offset.y += lineLength;
                this->addRectangleCenter(layer, center, width, lineLength);
            }
        }
    }

    auto Circuit::addViaCenter(Layer viaLayer, const Coord& _center, std::tuple<int, int> dimensions) -> Via&
    {
        auto center = _center.snapToGrid();
        debug::debugf("Add a via in '{}', with size '({}, {})'", 
            center, viaLayer, std::get<0>(dimensions), std::get<1>(dimensions)
        );
        auto& via = this->_vias.emplace_back(viaLayer, dimensions);
        via.place(center);
        return via;
    }

    auto Circuit::addPRBoundary() -> void 
    {
        this->addRectangle(Layer::PRBoundary, Coord{0._u, 0._u}, this->_width, this->_height);
    }

    auto Circuit::getBlockages(Layer layer) const -> std::vector<Blockage> 
    {
        auto blockages = std::vector<Blockage>{};

        auto tryPushBack = [&blockages] (const std::optional<Blockage>& blockage) {
            if (blockage.has_value())
                blockages.emplace_back(*blockage);
        };

        for (const auto& path : this->_paths) 
            tryPushBack(path.getBlockage(layer));
        for (const auto& rect : this->_rectangles) 
            tryPushBack(rect.getBlockage(layer));
        for (const auto& label : this->_labels) 
            tryPushBack(label.getBlockage(layer));
        for (const auto& via : this->_vias) 
            tryPushBack(via.getBlockage(layer));

        return blockages;
    }

    auto Circuit::routePinsCenter(
        const Coord& beginCoord, Layer beginLayer, 
        const Coord& endCoord, Layer endLayer, 
        Layer pathLayer, const std::optional<Number>& pathWidth) -> void
    {
        this->addViaThroughtLayersCenter(beginLayer, pathLayer, beginCoord);
        this->addViaThroughtLayersCenter(endLayer, pathLayer, endCoord);

        if (beginCoord == endCoord) 
        {
            return;
        } 
        else if (beginCoord.x == endCoord.x || beginCoord.y == endCoord.y) 
        {
            this->addRectangleSegmentCenter(pathLayer, beginCoord, endCoord, pathWidth);
        }
        else
        {
            auto corner = Coord {beginCoord.x, endCoord.y};
            this->addRectangleSegmentCenter(pathLayer, beginCoord, corner, pathWidth);
            this->addRectangleSegmentCenter(pathLayer, corner, endCoord, pathWidth);
        }
    }

    auto Circuit::routePinsCenter(
        const Pin& begin, const Pin& end, 
        Layer pathLayer, const std::optional<Number>& pathWidth) -> void 
    {
        this->routePinsCenter(
            begin.center(), begin.layer(),
            end.center(), end.layer(),
            pathLayer, pathWidth
        );
    }

    auto Circuit::routePinsByPathCenter(
        Layer beginLayer, Layer endLayer,
        Layer pathLayer, std::span<const Coord> path, std::optional<Number> widthOpt) -> void
    {
        this->routePathCenter(pathLayer, path, widthOpt);
        this->addViaThroughtLayersCenter(beginLayer, pathLayer, path.front());
        this->addViaThroughtLayersCenter(endLayer, pathLayer, path.back());
    }

    auto Circuit::routeColBus(Layer layer, const Number& width, const Number& height, const Number& yOff, std::span<const Number> xOffs) -> void
    {
        for (const auto& xOff : xOffs) 
        {
            this->addRectangle(
                layer,
                Coord {xOff, yOff},
                width,
                height
            );
        }
    }

    auto Circuit::routeRowBus(Layer layer, const Number& width, const Number& height, const Number& xOff, std::span<const Number> yOffs) -> void 
    {
        for (const auto& yOff : yOffs) 
        {
            this->addRectangle(
                layer,
                Coord {xOff, yOff},
                width,
                height
            );
        }
    }

    auto Circuit::routeRowBusPin(Layer layer, const Number& width, const Number& height, const Number& xOff, std::span<const Number> yOffs, std::span<const std::string_view> names) -> void 
    {
        debug::check(yOffs.size() == names.size(), "Pin yoff' size should equal to name's size");
        for (auto i = 0; i < yOffs.size(); ++i) {
            auto yOff = yOffs[i];
            auto name = names[i];
            this->addLabelPin(layer, name, Coord {xOff, yOff}, width, height);
        }
    }

    auto Circuit::addViaThroughtLayersCenter(Layer layer1, Layer layer2, const Coord& center, std::optional<Number> widthOpt) -> void 
    {
        auto layers = layerStacks(layer1, layer2);
        for (auto layer : layers)
        {
            auto width = this->defaultWidth(layer, widthOpt);
            if (layerIsVia(layer))
                this->addViaCenter(layer, center);
            else
                this->addRectangleCenter(layer, center, width, width);
        }
    }

    auto Circuit::defaultWidth(Layer layer, const std::optional<Number>& width) -> Number
    {
        auto minWidth = tech->layerWidth(layer);
        if (width.has_value()) 
        {
            if (*width < minWidth && !width->approximate(minWidth)) 
                debug::warningf("Use width '{}' less then min width '{}' in layer '{}'", *width, minWidth, layer);
            return *width;
        }
        return minWidth;
    }

}