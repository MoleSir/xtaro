#include "dffrow.hh"
#include "circuit/base/port.hh"
#include <circuit/libmodule/dff.hh>
#include "global/tech/tech.hh"

#include <circuit/libmodule/writedriver.hh>
#include <circuit/module/fanoutbuff.hh>

#include <circuit/mod.hh>

#include <ranges>
#include <utility/misc/string.hh>
#include <utility/misc/math.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    std::string DFlipFlopRow::Arguments::toString() const
    {
        return std::format("ds{}", this->dffSize);
    }

    auto DFlipFlopRow::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<DFlipFlopRow>(circuitName, *this);
    }

    DFlipFlopRow::DFlipFlopRow(std::string_view name, const DFlipFlopRow::Arguments& arguments) :
        Module{name},
        _dffSize{arguments.dffSize}
    {
        debug::checkf(this->_dffSize >= 1, "D-Flip-Flop size '{}' < 1", this->_dffSize);
        this->createSelf();
    }

    void DFlipFlopRow::createPorts()
    {   
        for (auto i : std::ranges::views::iota(0, this->_dffSize))
            this->addPort(DFlipFlopRow::DIN_NAME(i), PortType::Input);

        for (auto i : std::ranges::views::iota(0, this->_dffSize))
            this->addPort(DFlipFlopRow::DOUT_NAME(i), PortType::Output);

        this->addPortNet(DFlipFlopRow::CLOCK_NAME, PortType::Input);
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void DFlipFlopRow::createCircuits()
    {
        this->_dff = this->addCircuit(DFF::Arguments{});
    } 

    void DFlipFlopRow::createInstances() 
    {
        for (auto i : std::ranges::views::iota(0, this->_dffSize))
        {
            // D Q clk vdd gnd
            auto inst = this->addInstance(
                stringf("dff{}", i), 
                this->_dff, 
                {
                    DFlipFlopRow::DIN_NAME(i),
                    DFlipFlopRow::DOUT_NAME(i),
                    DFlipFlopRow::CLOCK_NAME,
                    tech->vdd(), tech->gnd()
                }
            );
            this->_dffInsts.emplace_back(inst);
        }
    }

    void DFlipFlopRow::calculateLayoutConstants()
    {
        this->_width = this->_dffSize * this->_dff->width();
        this->_height = this->_dff->height();

        this->_vertLayer = Layer::Metal4;
        this->_horiLayer = Layer::Metal3;
        this->_viaLayer = Layer::Via3;
        
        this->_vertWidth = tech->layerWidth(this->_vertLayer);
        this->_vertPitch = tech->layerPitch(this->_vertLayer);

        this->_horiWidth = tech->layerWidth(this->_horiLayer);
        this->_horiPitch = tech->layerPitch(this->_horiLayer); 
    }

    void DFlipFlopRow::placeInstances()
    {
        this->placeInstArrayInRow(this->_dffInsts, 0._u, 0._u);
    }

    void DFlipFlopRow::routeInstances()
    {
    }

    void DFlipFlopRow::createLayoutPins()
    {
        for (auto i : std::ranges::views::iota(0, this->_dffSize)) 
        {
            auto dff = this->_dffInsts.at(i);
            this->copyInstancePin(dff, DFF::DIN_NAME, DFlipFlopRow::DIN_NAME(i));
            this->copyInstancePin(dff, DFF::DOUT_NAME, DFlipFlopRow::DOUT_NAME(i));
            this->copyInstancePin(dff, DFF::CLOCK_NAME, DFlipFlopRow::CLOCK_NAME);
        }
        
        for (auto inst : this->_dffInsts) 
        {
            this->copyInstancePin(inst, tech->vdd(), tech->vdd());
            this->copyInstancePin(inst, tech->gnd(), tech->gnd());
        }
    }

    auto DFlipFlopRow::DIN_NAME(int index) -> std::string_view 
    {
        return stringf("D{}", index);
    }

    auto DFlipFlopRow::DOUT_NAME(int index) -> std::string_view 
    {
        return stringf("Q{}", index);
    }

}