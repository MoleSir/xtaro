#include "dffcolumn.hh"
#include "circuit/base/port.hh"
#include <circuit/libmodule/dff.hh>

#include <circuit/libmodule/writedriver.hh>
#include <circuit/module/fanoutbuff.hh>

#include <circuit/mod.hh>

#include <ranges>
#include <utility/misc/string.hh>
#include <utility/misc/math.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    std::string DFlipFlopColumn::Arguments::toString() const
    {
        return std::format("ds{}", this->dffSize);
    }

    auto DFlipFlopColumn::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<DFlipFlopColumn>(circuitName, *this);
    }

    DFlipFlopColumn::DFlipFlopColumn(std::string_view name, const DFlipFlopColumn::Arguments& arguments) :
        Module{name},
        _dffSize{arguments.dffSize}
    {
        debug::checkf(this->_dffSize >= 1, "D-Flip-Flop size '{}' < 1", this->_dffSize);
        this->createSelf();
    }

    void DFlipFlopColumn::createPorts()
    {   
        for (auto i : std::ranges::views::iota(0, this->_dffSize))
            this->addPort(DFlipFlopColumn::DIN_NAME(i), PortType::Input);

        for (auto i : std::ranges::views::iota(0, this->_dffSize))
            this->addPort(DFlipFlopColumn::DOUT_NAME(i), PortType::Output);

        this->addPortNet(DFlipFlopColumn::CLOCK_NAME, PortType::Input);
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void DFlipFlopColumn::createCircuits()
    {
        this->_dff = this->addCircuit(DFF::Arguments{});
    } 

    void DFlipFlopColumn::createInstances() 
    {
        for (auto i : std::ranges::views::iota(0, this->_dffSize))
        {
            // D Q clk vdd gnd
            auto inst = this->addInstance(
                stringf("dff{}", i), 
                this->_dff, 
                {
                    DFlipFlopColumn::DIN_NAME(i),
                    DFlipFlopColumn::DOUT_NAME(i),
                    DFlipFlopColumn::CLOCK_NAME,
                    tech->vdd(), tech->gnd()
                }
            );
            this->_dffInsts.emplace_back(inst);
        }
    }

    void DFlipFlopColumn::calculateLayoutConstants()
    {
        this->_width = this->_dff->width();
        this->_height = this->_dffSize * this->_dff->height();
    }

    void DFlipFlopColumn::placeInstances()
    {
        this->placeInstArrayInColumn(this->_dffInsts, 0._u, 0._u);
    }

    void DFlipFlopColumn::routeInstances()
    {
    }

    void DFlipFlopColumn::createLayoutPins()
    {
        for (auto i : std::ranges::views::iota(0, this->_dffSize)) 
        {
            auto dff = this->_dffInsts.at(i);
            this->copyInstancePin(dff, DFF::DIN_NAME, DFlipFlopColumn::DIN_NAME(i));
            this->copyInstancePin(dff, DFF::DOUT_NAME, DFlipFlopColumn::DOUT_NAME(i));
            this->copyInstancePin(dff, DFF::CLOCK_NAME, DFlipFlopColumn::CLOCK_NAME);
        }

        for (auto i : std::ranges::views::iota(0, this->_dffSize)
                    | std::ranges::views::filter([] (auto a) { return a % 2 == 0; }))
        {
            auto inst = this->_dffInsts.at(i);
            this->copyInstancePin(inst, tech->vdd(), tech->vdd());
            this->copyInstancePin(inst, tech->gnd(), tech->gnd());
        }
        this->copyInstancePin(this->_dffInsts.back(), tech->gnd(), tech->gnd());
    }

    auto DFlipFlopColumn::DIN_NAME(int index) -> std::string_view 
    {
        return stringf("D{}", index);
    }

    auto DFlipFlopColumn::DOUT_NAME(int index) -> std::string_view 
    {
        return stringf("Q{}", index);
    }

}