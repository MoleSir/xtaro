#include "or.hh"
#include "circuit/geometry/coord.hh"

#include <circuit/libmodule/nor.hh>
#include <circuit/libmodule/inv.hh>

#include <circuit/mod.hh>

#include <global/config/config.hh>
#include <global/tech/tech.hh>

#include <utility/misc/string.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    std::string OR::Arguments::toString() const
    {
        return std::format("{}_{}", this->inputSize, this->driveAbility);
    }

    auto OR::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<OR>(circuitName, *this);
    }

    OR::OR(std::string_view name, const OR::Arguments& arguments) :
        Module{name},
        _driveAbility{arguments.driveAbility},
        _inputSize{arguments.inputSize},
        _nor{nullptr},
        _inv{nullptr}
    {
        debug::checkf(this->_inputSize >= 2, "Input size '{}' < 2", this->_inputSize);
        this->tryLoadFromLibrary();
    }

    void OR::createPorts()
    {
        for (int i = 0; i < this->_inputSize; ++i)
            this->addPortNet(this->INPUT_NAME(i), PortType::Input);

        this->addPortNet(this->OUTPUT_NAME, PortType::Output);
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void OR::createCircuits()
    {   
        NOR::Arguments norArgument {this->_driveAbility, this->_inputSize};
        this->_nor = this->addCircuit(norArgument);

        INV::Arguments invArgument {this->_driveAbility};
        this->_inv = this->addCircuit(invArgument);
    } 

    void OR::createInstances() 
    {
        std::vector<std::string_view> nets{};
        for (int i = 0; i < this->_inputSize; ++i)
            nets.emplace_back( this->INPUT_NAME(i) );
        nets.emplace_back("Z_bar");
        nets.emplace_back(tech->vdd());
        nets.emplace_back(tech->gnd());
        
        this->_norInst = this->addInstance(NOR::TYPE, this->_nor, nets);
        this->_invInst = this->addInstance(INV::TYPE, this->_inv, {"Z_bar", this->OUTPUT_NAME, tech->vdd(), tech->gnd()});
    }

    void OR::calculateLayoutConstants()
    {
        auto norH = this->_norInst->circuit()->height();
        auto invH = this->_invInst->circuit()->height();
        debug::checkf(norH.approximate(invH), "Nor's height '{}' unequal inv's height '{}'", norH, invH);
        this->_height = norH;
        this->_width = this->_norInst->circuit()->width() + this->_invInst->circuit()->width();
    }

    void OR::placeInstances()
    {
        this->_norInst->place(Coord{0._u, 0._u});
        this->_invInst->place(Coord{this->_norInst->circuit()->width(), 0._u});
    }

    void OR::routeInstances()
    {
        auto norInstOutPin = this->_norInst->getPin(NOR::OUTPUT_NAME);
        auto invInstInPin = this->_invInst->getPin(INV::INPUT_NAME);
        debug::check(norInstOutPin.layer() == invInstInPin.layer(), "Unmatch layer in nor & inv");
        this->routePathCenter(norInstOutPin.layer(), std::array{norInstOutPin.center(), invInstInPin.center()});
    }

    void OR::createLayoutPins()
    {
        for (auto i = 0; i < this->_inputSize; ++i)
            this->copyInstancePin(this->_norInst, NOR::INPUT_NAME(i), OR::INPUT_NAME(i));
        this->copyInstancePin(this->_invInst, INV::OUTPUT_NAME, OR::OUTPUT_NAME);
        this->copyInstancePin(this->_norInst, tech->vdd(), tech->vdd());
        this->copyInstancePin(this->_norInst, tech->gnd(), tech->gnd());
    }

    auto OR::INPUT_NAME(int index) -> std::string_view 
    {
        return stringf("A{}", index);
    }
    
}