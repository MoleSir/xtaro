#include "inv.hh"

#include <circuit/component/mos.hh>

#include <circuit/mod.hh>

#include <global/config/config.hh>
#include <global/tech/tech.hh>

#include <utility/misc/string.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    std::string INV::Arguments::toString() const
    {
        return std::format("{}", this->driveAbility);
    }

    auto INV::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<INV>(circuitName, *this);
    }

    INV::INV(std::string_view name, const INV::Arguments& arguments) :
        Module{name},
        _driveAbility{arguments.driveAbility},
        _nmos{nullptr},
        _pmos{nullptr}
    {
        this->tryLoadFromLibrary();
    }

    void INV::createPorts()
    {
        this->addPortNet(this->INPUT_NAME, PortType::Input);
        this->addPortNet(this->OUTPUT_NAME, PortType::Output);
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void INV::createCircuits()
    {
        auto length = tech->layerWidth(Layer::Poly);

        // Assign arguments
        MOS::Arguments nmosArguments {MOSType::NMOS, 0.0_o, length};
        MOS::Arguments pmosArguments {MOSType::PMOS, 0.0_o, length};

        switch (this->_driveAbility)
        {
        case DriveAbility::X0:
            nmosArguments.width = length;
            pmosArguments.width = length;
            break;
        case DriveAbility::X1:
            nmosArguments.width = 2.0 * length;
            pmosArguments.width = 4.0 * length;
            break;
        case DriveAbility::X2:
            nmosArguments.width = 4.0 * length;
            pmosArguments.width = 8.0 * length;
            break;
        case DriveAbility::X3:
            nmosArguments.width = 6.0 * length;
            pmosArguments.width = 12.0 * length;
            break;
        default:
            nmosArguments.width = 2.0 * length;
            pmosArguments.width = 4.0 * length;
            break;
            break;
        }

        // Create MOS
        this->_nmos = this->addCircuit(nmosArguments);
        this->_pmos = this->addCircuit(pmosArguments);
    } 

    void INV::createInstances() 
    {
        // Create instances
        this->addInstance("inv_nmos", this->_nmos, {this->OUTPUT_NAME, this->INPUT_NAME, tech->gnd(), tech->gnd()});
        this->addInstance("inv_pmos", this->_pmos, {this->OUTPUT_NAME, this->INPUT_NAME, tech->vdd(), tech->vdd()});
    }

    void INV::calculateLayoutConstants()
    {
        debug::unimplement("INV::calculateLayoutConstants");
    }

    void INV::placeInstances()
    {
        debug::unimplement("INV::placeInstances");
    }

    void INV::routeInstances()
    {
        debug::unimplement("INV::placeInstances");
    }
    
    void INV::createLayoutPins()
    {
        debug::unimplement("INV::createLayoutPins");
    }

}