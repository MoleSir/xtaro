#include "instance.hh"
#include "circuit/geometry/blockage.hh"
#include "circuit/geometry/coord.hh"
#include "net.hh"
#include "circuit.hh"
#include "port.hh"

#include <utility/misc/string.hh>
#include <utility/debug/debug.hh>

#include <string>
#include <string>
#include <vector>

namespace xtaro::circuit
{

    Instance::Instance(std::string_view name, Circuit* circuit) :
        _name{name},
        _circuit{std::move(circuit)},
        _ports{_circuit->copyPort()},
        _connectCount{0},
    
        _offset{{}, {}},
        _mirror{MirrorType::None},
        _angle{0.0}
    { 
    }

    Instance::~Instance() noexcept
    {
    }

    void Instance::connectNet(std::string_view portName, Net* net)
    {
        for (auto& port : this->_ports)
        {
            if (port.name() == portName)
            {
                port.setNet(net);
                this->_connectCount += 1;
                return;
            }
        }
    }

    void Instance::connectNets(const std::vector<Net*>& nets)
    {
        for (std::size_t i = 0; i < this->_ports.size(); ++i)
        {
            this->_ports[i].setNet(nets[i]);
            this->_connectCount += 1;
        }
    }

    void Instance::checkPortsFullyConnected() const
    {
        if (this->_connectCount < this->_ports.size())
        {
            std::string errorMsg {
                std::format("The instance '{}', belongs circuit '{}'. Not all port has been connected. Connected: '{}'.", 
                             this->_name, 
                             this->_circuit->name(), 
                             this->_connectCount)
            };

            debug::exceptionIn("Generate Instance spice", errorMsg);
        }
    }
    
    auto Instance::place(const Coord& _offset, MirrorType mirror, double angle) -> void
    {
        auto offset = _offset.snapToGrid();
        debug::debugf("Place instance '{}' in {}", this->_name, offset);
        this->_offset = offset;
        this->_mirror = mirror;
        this->_angle  = angle;
    }

    auto Instance::getBlockages(Layer layer) const -> std::vector<Blockage> 
    {
        auto transformBlockags = std::vector<Blockage>{};
        
        for (const auto& blockage : this->_circuit->getBlockages(layer)) 
            transformBlockags.emplace_back(blockage.transform(this->_offset, this->_mirror, this->_angle));

        return transformBlockags;
    }

    auto Instance::getPin(std::string_view name) -> Pin
    {
        auto pin = this->_circuit->getPin(name);
        pin.transform(this->_offset, this->_mirror, this->_angle);
        return pin;
    }

    auto Instance::getPins(std::string_view name) -> std::vector<Pin>
    {
        auto pins = std::vector<Pin>{};
        for (auto p : this->_circuit->getPins(name)) {
            p.transform(this->_offset, this->_mirror, this->_angle);
            pins.emplace_back(std::move(p));
        }
        return pins;
    }

}