#pragma once

#include "./port.hh"
#include "../geometry/label.hh"
#include "../geometry/rectangle.hh"
#include "../geometry/pin.hh"
#include "../geometry/path.hh"
#include "../geometry/via.hh"
#include "../geometry/coord.hh"
#include "circuit/base/instance.hh"
#include "circuit/geometry/blockage.hh"
#include "global/tech/layer.hh"
#include <global/number/number.hh>

#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>
#include <string_view>
#include <memory>
#include <filesystem>

namespace xtaro::circuit
{

    struct CircuitVisitor;

    /*
        A base abstract class for all circuit. 
    */

    class Circuit
    {
    public:
        class Arguments
        {
        public:
            Arguments(std::string_view circuitType) noexcept;
            virtual ~Arguments() noexcept = default;

        public:
            virtual auto toString() const -> std::string = 0;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> = 0;

        public:
            auto circuitType() const noexcept -> std::string_view;

        private:
            std::string_view _circuitType;
        };

    public:
        Circuit(std::string_view name);
        virtual ~Circuit() noexcept;
        
        // No copy No move
        Circuit(const Circuit&) = delete;
        Circuit(Circuit&&) noexcept = delete;
        Circuit& operator = (const Circuit&) = delete;
        Circuit& operator = (Circuit&&) noexcept = delete;

    public:
        virtual auto accept(CircuitVisitor& visitor) -> void = 0;

    public:
        /*
            Add an port to this circuit.
            - portName: the port's name;
            - portType: the port's type.
        */
        void addPort(std::string_view portName, PortType portType);

        /*
            Add ports to this circuit.
            - portName: the ports' name;
            - portType: the ports' type.
        */
        void addPorts(const std::vector<std::string_view>& portsName, PortType portType);

        /*
            Return all ports's name
        */
        auto portsName() const -> std::vector<std::string_view>;

        /*
            Return all output port's name
        */
        auto outputPortsName() const -> std::vector<std::string_view>;

        /*
            Copy all ports in this circuit, return as a std::vector<Port*>.
            This is a deep copy, the method will take the datas in this ports,
            and create some new ports object.
            This method mainly used in creating a Instance object.
        */
        auto copyPort() const -> std::vector<Port>;

        auto portType(std::string_view name) const -> PortType;

    public:
        /*
            Add geometry...

            The menthod ends with `Center` means the coords in param is center of gemotry
            otherwise is leftdown
        */
        auto addRectangle(Layer layer, const Coord& leftdown, std::optional<Number> width = std::nullopt, std::optional<Number> height = std::nullopt) -> Rectangle&;
        auto addRectangleCenter(Layer layer, const Coord& center, std::optional<Number> width = std::nullopt, std::optional<Number> height = std::nullopt) -> Rectangle&;
        auto addRectangleSegment(Layer layer, const Coord& begin, const Coord& end, std::optional<Number> width = std::nullopt) -> Rectangle&;
        auto addRectangleSegmentCenter(Layer layer, const Coord& begin, const Coord& end, std::optional<Number> width = std::nullopt) -> Rectangle&;

        auto addPin(Layer layer, std::string_view name, const Coord& leftdown, std::optional<Number> width = std::nullopt, std::optional<Number> height = std::nullopt) -> Pin&;
        auto addPinCenter(Layer layer, std::string_view name, const Coord& center, std::optional<Number> width = std::nullopt, std::optional<Number> height = std::nullopt) -> Pin&;
        auto addPinSegment(Layer layer, std::string_view name, const Coord& begin, const Coord& end, std::optional<Number> width = std::nullopt) -> Pin&;
        auto addPinSegmentCenter(Layer layer, std::string_view name, const Coord& begin, const Coord& end, std::optional<Number> width = std::nullopt) -> Rectangle&;
        
        auto addLabel(Layer layer, std::string_view name, const Coord& offset) -> Label&;

        auto addLabelPin(Layer layer, std::string_view name, const Coord& leftdown, std::optional<Number> width = std::nullopt, std::optional<Number> height = std::nullopt) -> void;
        auto addLabelPinCenter(Layer layer, std::string_view name, const Coord& center, std::optional<Number> width = std::nullopt, std::optional<Number> height = std::nullopt) -> void;

        auto addViaCenter(Layer via, const Coord& center, std::tuple<int, int> dimensions = {1, 1}) -> Via&;

        auto addPRBoundary() -> void;

        auto getBlockages(Layer layer) const -> std::vector<Blockage>;

    /*
        Some util function for routing
    */
    public:
        /*
            Add a path in given coords, if coords it not line, add a new coord
        */
        auto routePathCenter(
            Layer layer, std::span<const Coord> path, std::optional<Number> width = std::nullopt) -> void;
        
        /*
            Connect two pins in given layer and add vias in begin & end coord(need in a line)
        */
        auto routePinsCenter(
            const Coord& beginCoord, Layer beginLayer, 
            const Coord& endCoord, Layer endLayer, 
            Layer pathLayer, const std::optional<Number>& pathWidth = std::nullopt) -> void;

        auto routePinsCenter(
            const Pin& begin, const Pin& end, 
            Layer pathLayer, const std::optional<Number>& pathWidth = std::nullopt) -> void;

        /*
            Route two pins with in spec path
        */
        auto routePinsByPathCenter(
            Layer beginLayer, Layer endLayer,
            Layer pathLayer, std::span<const Coord> path, std::optional<Number> width = std::nullopt) -> void;

        /*
            Route bus line for col or row
        */
        auto routeColBus(Layer layer, const Number& width, const Number& height, const Number& yOff, std::span<const Number> xOffs) -> void;
        auto routeRowBus(Layer layer, const Number& width, const Number& height, const Number& xOff, std::span<const Number> yOffs) -> void;

        auto routeRowBusPin(Layer layer, const Number& width, const Number& height, const Number& xOff, std::span<const Number> yOffs, std::span<const std::string_view> names) -> void;

        /*
            Add via for specfic layers
        */
        auto addViaThroughtLayersCenter(Layer layer1, Layer layer2, const Coord& center, std::optional<Number> width = std::nullopt) -> void;

    private:
        auto doAddPin(Layer layer, std::string_view name, const Coord& leftdown, const Number& width, const Number& height) -> Pin&;
        auto doAddRectangle(Layer layer, const Coord& leftdown, const Number& width, const Number& height) -> Rectangle&;

    public:
        auto connectRowPins(Layer layer, std::span<const Pin> pins);
        auto connectColPins(Layer layer, std::span<const Pin> pins);

    public:
        auto getPin(std::string_view name) -> const Pin&;
        auto getPins(std::string_view name) -> std::span<const Pin>;

    protected:
        auto createSelf() -> void;

        virtual auto createPorts() -> void = 0;
        virtual auto createNetlist() -> void = 0;
        virtual auto createLayout()  -> void = 0;

    public:
        auto name() const noexcept -> std::string_view { return this->_name; }
        auto width() const noexcept -> const Number& { return this->_width.value(); }
        auto height() const noexcept -> const Number& { return this->_height.value(); }
        auto setWidth(const Number& w) { this->_width.emplace(w); }
        auto setHeight(const Number& h) { this->_height.emplace(h); }

        auto ports() const noexcept -> const std::vector<Port>& { return this->_ports; }
        auto paths() const -> std::span<const Path> { return this->_paths; }
        auto rectangles() const -> std::span<const Rectangle> { return this->_rectangles; }
        auto labels() const -> std::span<const Label> { return this->_labels; }
        auto pins() const -> const std::unordered_map<std::string_view, std::vector<Pin>>& { return this->_pins; }
        auto vias() const -> std::span<const Via> { return this->_vias; }

    protected:
        auto defaultWidth(Layer layer, const std::optional<Number>& width) -> Number;

    public:
        auto spiceFilePath() const -> const std::filesystem::path&;
        auto hasSpiceFilePath() const -> bool;

        auto setSpiceFilePath(std::filesystem::path filepath) -> void;
        auto gdsiiFilePath() const -> const std::filesystem::path&;

        auto hasGDSIIFilePath() const -> bool;
        auto setGDSIIFilePath(std::filesystem::path filepath) -> void;

    protected:
        std::string_view _name;

        std::optional<Number> _width;
        std::optional<Number> _height;

        std::unordered_map<std::string_view, std::vector<Pin>> _pins;

        std::vector<Port> _ports;
        std::vector<Path> _paths;
        std::vector<Rectangle> _rectangles;
        std::vector<Label> _labels;
        std::vector<Via> _vias;

        std::optional<std::filesystem::path> _spiceFilePath;
        std::optional<std::filesystem::path> _gdsiiFilePath;
    };

}