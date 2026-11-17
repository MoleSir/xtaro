#pragma once

#include "circuit/geometry/blockage.hh"
#include "circuit/geometry/pin.hh"
#include <circuit/geometry/coord.hh>
#include <parse/gdsii/gdsiilayout.hh>

#include <string_view>
#include <vector>

namespace xtaro::circuit
{
    
    class Circuit;
    class Port;
    class Net;

    class Instance
    {
    public:
        Instance(std::string_view name, Circuit* circuit);
        ~Instance() noexcept;

    public:
        //! \brief Connect 'net' to the 'port' whose name is 'portName'
        void connectNet(std::string_view portName, Net* net);
        void connectNets(const std::vector<Net*>& nets);

        void checkPortsFullyConnected() const;

    public:
        auto getBlockages(Layer layer) const -> std::vector<Blockage>;

    public:
        auto place(const Coord& offset, MirrorType mirror = MirrorType::None, double angle = 0.) -> void;

        auto getPin(std::string_view name) -> Pin;
        auto getPins(std::string_view name) -> std::vector<Pin>;

    public:
        auto name() const noexcept -> std::string_view { return this->_name; }
        auto circuit() const noexcept -> Circuit* { return this->_circuit; }
        auto ports() const noexcept -> const std::vector<Port>& { return this->_ports; }
        auto offset() const -> const Coord& { return this->_offset; }
        auto mirror() const -> MirrorType { return this->_mirror; }
        auto angle() const -> double { return this->_angle; }

    private:
        std::string_view _name;
        Circuit* _circuit;
        std::vector<Port> _ports;
        std::size_t _connectCount;

        Coord _offset;
        MirrorType _mirror;
        double _angle;
    };

}