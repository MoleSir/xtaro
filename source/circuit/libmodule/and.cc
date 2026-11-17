#include "and.hh"

#include <circuit/component/mos.hh>
#include <circuit/libmodule/nand.hh>
#include <circuit/libmodule/inv.hh>

#include <circuit/mod.hh>

#include <global/config/config.hh>
#include <global/tech/tech.hh>

#include <utility/misc/string.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{

    auto AND::Arguments::toString() const -> std::string
    {
        return std::format("{}_{}", this->inputSize, this->driveAbility);
    }

    auto AND::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<AND>(circuitName, *this);
    }

    AND::AND(std::string_view name, const AND::Arguments& arguments) :
        Module{name},
        _driveAbility{arguments.driveAbility},
        _inputSize{arguments.inputSize},
        _nand{nullptr},
        _inv{nullptr}
    {
        debug::checkf(this->_inputSize >= 2, "Input size '{}' < 2", this->_inputSize);
        this->tryLoadFromLibrary();
    }

    void AND::createPorts()
    {
        for (int i = 0; i < this->_inputSize; ++i)
            this->addPortNet(this->INPUT_NAME(i), PortType::Input);

        this->addPortNet(this->OUTPUT_NAME, PortType::Output);
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void AND::createCircuits()
    {   
        NAND::Arguments nandArgument {this->_driveAbility, this->_inputSize};
        this->_nand = this->addCircuit(nandArgument);

        this->_inv = this->addCircuit(INV::Arguments {this->_driveAbility});
    } 

    void AND::createInstances() 
    {
        std::vector<std::string_view> nets{};
        for (int i = 0; i < this->_inputSize; ++i)
            nets.emplace_back( this->INPUT_NAME(i) );
        nets.emplace_back("Z_bar");
        nets.emplace_back(tech->vdd());
        nets.emplace_back(tech->gnd());

        this->addInstance(NAND::TYPE, this->_nand, nets);
        this->addInstance(INV::TYPE, this->_inv, {"Z_bar", this->OUTPUT_NAME, tech->vdd(), tech->gnd()});
    }

    void AND::calculateLayoutConstants()
    {
        debug::unimplement("AND::calculateLayoutConstants");
    }

    void AND::placeInstances()
    {
        debug::unimplement("AND::placeInstances");
    }

    void AND::routeInstances()
    {
        debug::unimplement("AND::placeInstances");
    }

    void AND::createLayoutPins()
    {
        debug::unimplement("AND::createLayoutPins");
    }

    auto AND::INPUT_NAME(int index) -> std::string_view 
    {
        return stringf("A{}", index);
    }

}