#include "trigate.hh"
#include "../base/module.hh"
#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    TristateGate::TristateGate(std::string_view name, const TristateGate::Arguments& arguments) :
        Module{name}
    {
        this->loadFromLibrary();
    }

    auto TristateGate::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<TristateGate>(circuitName, *this);
    }

    void TristateGate::createPorts()
    {
        // in out en en_bar vdd gnd
        this->addPortNet("in", PortType::Input );
        this->addPortNet("out", PortType::Output );
        this->addPortNet("en", PortType::Input );
        this->addPortNet("en_bar", PortType::Input );
        this->addPortNet(tech->vdd(), PortType::InOut );
        this->addPortNet(tech->gnd(), PortType::InOut );
    }

    void TristateGate::createCircuits()
    {
    } 

    void TristateGate::createInstances() 
    {
    }

    void TristateGate::calculateLayoutConstants()
    {
    }

    void TristateGate::placeInstances()
    {
    }

    void TristateGate::routeInstances()
    {
    }
    
    void TristateGate::createLayoutPins()
    {
        debug::unimplement("TristateGate::createLayoutPins");
    }

}