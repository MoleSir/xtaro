#include "./columnmuxarr.hh"
#include "circuit/base/module.hh"
#include "circuit/module/columnmux.hh"
#include <tech/tech.hh>

#include <cassert>
#include <circuit/module/fanoutbuff.hh>

#include <circuit/mod.hh>

#include <ranges>
#include <utility/misc/string.hh>
#include <utility/misc/math.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    std::string ColumnMuxArray::Arguments::toString() const
    {
        return std::format("ss{}_ms{}{}", 
            this->selSize, 
            this->muxSize,
            this->useFanoutBuffer ? "_buf" : "");
    }

    auto ColumnMuxArray::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<ColumnMuxArray>(circuitName, *this);
    }

    ColumnMuxArray::ColumnMuxArray(std::string_view name, const ColumnMuxArray::Arguments& arguments) :
        Module{name},
        _selSize{arguments.selSize},
        _muxSize{arguments.muxSize},
        _useFanoutBuffer{arguments.useFanoutBuffer}
    {
        debug::checkf(this->_selSize > 1, "Selection size '{}' <= 1", this->_selSize);
        this->createSelf();
    }

    void ColumnMuxArray::createPorts()
    {
        for (int i = 0; i < this->_selSize; ++i)
            this->addPortNet(this->SELECTION_NAME(i), PortType::Input);

        for (int muxIdx = 0; muxIdx < this->_muxSize; ++muxIdx)
        {
            for (int inputIdx = 0; inputIdx < this->_selSize; ++inputIdx)
                this->addPortNet( this->BITLINE_NAME(muxIdx, inputIdx), PortType::InOut);
            for (int inputIdx = 0; inputIdx < this->_selSize; ++inputIdx)
                this->addPortNet( this->BITLINE_BAR_NAME(muxIdx, inputIdx), PortType::InOut);
        }
        
        for (int muxIdx = 0; muxIdx < this->_muxSize; ++muxIdx)
        {
            this->addPortNet(this->OUTPUT_BITLINE_NAME(muxIdx), PortType::InOut);
            this->addPortNet(this->OUTPUT_BITLINE_BAR_NAME(muxIdx), PortType::InOut);
        }

        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void ColumnMuxArray::createCircuits()
    {
        this->_mux = this->addCircuit(ColumnMux::Arguments {this->_selSize});
        if (this->_useFanoutBuffer) 
            this->_fanoutbuf = this->addCircuit(FanoutBuffer::Arguments {this->_muxSize});
    } 

    void ColumnMuxArray::createInstances() 
    {
        if (this->_useFanoutBuffer)
        {
            assert(this->_fanoutbuf != nullptr);

            // Fanout buffer
            std::vector<std::vector<std::string_view>> fanoutNets {};

            for (int selIdx = 0; selIdx < this->_selSize; ++selIdx)
            {
                std::vector<std::string_view> nets {};
                nets.emplace_back(this->SELECTION_NAME(selIdx));

                for (int muxIdx = 0; muxIdx < this->_muxSize; ++muxIdx)
                    nets.emplace_back(stringf("sel{}_buf{}", selIdx, muxIdx));

                nets.emplace_back(tech->vdd());
                nets.emplace_back(tech->gnd());
                
                this->addInstance(stringf("fanout_buff_{}", selIdx), this->_fanoutbuf, nets);

                fanoutNets.emplace_back(std::move(nets));
            }

            for (int muxIdx = 0; muxIdx < this->_muxSize; ++muxIdx)
            { 
                assert(this->_fanoutbuf == nullptr);

                // sel0 sel1 ... sel2^n-1 bl0 bl1 ... bl2^n-1 br0 br1 ... br2^n-1 bl br vdd gnd
                std::vector<std::string_view> muxPorts {};
                for (int selIdx = 0; selIdx < this->_selSize; ++selIdx)
                    muxPorts.emplace_back(stringf("sel{}_buf{}", selIdx, muxIdx));
                
                for (int inputIdx = 0; inputIdx < this->_selSize; ++inputIdx)
                    muxPorts.emplace_back(this->BITLINE_NAME(muxIdx, inputIdx));
                
                for (int inputIdx = 0; inputIdx < this->_selSize; ++inputIdx)
                    muxPorts.emplace_back(this->BITLINE_BAR_NAME(muxIdx, inputIdx));

                muxPorts.emplace_back(this->OUTPUT_BITLINE_NAME(muxIdx));
                muxPorts.emplace_back(this->OUTPUT_BITLINE_BAR_NAME(muxIdx));

                muxPorts.emplace_back(tech->vdd());
                muxPorts.emplace_back(tech->gnd());

                auto inst = this->addInstance(stringf("mux{}", muxIdx), this->_mux, muxPorts);
                this->_muxInsts.emplace_back(inst);
            }
        }
        else 
        {
            for (int muxIdx = 0; muxIdx < this->_muxSize; ++muxIdx)
            { 
                // sel0 sel1 ... sel2^n-1 bl0 bl1 ... bl2^n-1 br0 br1 ... br2^n-1 bl br vdd gnd
                std::vector<std::string_view> muxPorts {};
                for (int selIdx = 0; selIdx < this->_selSize; ++selIdx)
                    muxPorts.emplace_back(this->SELECTION_NAME(selIdx));
                
                for (int inputIdx = 0; inputIdx < this->_selSize; ++inputIdx)
                    muxPorts.emplace_back(this->BITLINE_NAME(muxIdx, inputIdx));
                
                for (int inputIdx = 0; inputIdx < this->_selSize; ++inputIdx)
                    muxPorts.emplace_back(this->BITLINE_BAR_NAME(muxIdx, inputIdx));

                muxPorts.emplace_back(this->OUTPUT_BITLINE_NAME(muxIdx));
                muxPorts.emplace_back(this->OUTPUT_BITLINE_BAR_NAME(muxIdx));

                muxPorts.emplace_back(tech->vdd());
                muxPorts.emplace_back(tech->gnd());

                auto inst = this->addInstance(stringf("mux{}", muxIdx), this->_mux, muxPorts);
                this->_muxInsts.emplace_back(inst);
            }
        }
    }

    void ColumnMuxArray::calculateLayoutConstants()
    {
        if (this->_useFanoutBuffer) {
            assert(this->_fanoutbuf != nullptr);
            debug::warning("Fanout Buffer in ANDArray does not support to build layout!");
        }

        this->_width = this->_muxInsts.size() * this->_mux->width();
        this->_height = this->_mux->height();
    }

    void ColumnMuxArray::placeInstances()
    {
        Module::placeInstArrayInRow(this->_muxInsts, 0._u, 0._u);
    }

    void ColumnMuxArray::routeInstances()
    {

    }

    void ColumnMuxArray::createLayoutPins()
    {
        // this->copyInstancePins(this->_muxInsts.front(), ColumnMux);
        for (auto i : std::ranges::views::iota(0, this->_selSize)) 
            this->copyInstancePin(this->_muxInsts.front(), ColumnMux::SELECTION_NAME(i), ColumnMuxArray::SELECTION_NAME(i));

        for (auto muxIndex : std::ranges::views::iota(0, this->_muxSize))
        {
            auto inst = this->_muxInsts.at(muxIndex);

            for (auto inputIndex : std::ranges::views::iota(0, this->_selSize))
            {
                this->copyInstancePin(inst, ColumnMux::BITLINE_NAME(inputIndex), ColumnMuxArray::BITLINE_NAME(muxIndex, inputIndex));
                this->copyInstancePin(inst, ColumnMux::BITLINE_BAR_NAME(inputIndex), ColumnMuxArray::BITLINE_BAR_NAME(muxIndex, inputIndex));
            }

            this->copyInstancePin(inst, ColumnMux::OUTPUT_BITLINE_NAME, ColumnMuxArray::OUTPUT_BITLINE_NAME(muxIndex));
            this->copyInstancePin(inst, ColumnMux::OUTPUT_BITLINE_BAR_NAME, ColumnMuxArray::OUTPUT_BITLINE_BAR_NAME(muxIndex));
            this->copyInstancePins(inst, tech->vdd(), tech->vdd());
            this->copyInstancePins(inst, tech->gnd(), tech->gnd());
        }
    }   

    auto ColumnMuxArray::SELECTION_NAME(int index) -> std::string_view {
        return stringf("sel{}", index);
    }

    auto ColumnMuxArray::BITLINE_NAME(int muxIndex, int inputIdx) -> std::string_view {
        return stringf("bl{}_{}", muxIndex, inputIdx);
    }

    auto ColumnMuxArray::BITLINE_BAR_NAME(int muxIndex, int inputIdx) -> std::string_view {
        return stringf("br{}_{}", muxIndex, inputIdx);
    }

    auto ColumnMuxArray::OUTPUT_BITLINE_NAME(int index) -> std::string_view {
        return stringf("bl{}", index);
    }

    auto ColumnMuxArray::OUTPUT_BITLINE_BAR_NAME(int index) -> std::string_view {
        return stringf("br{}", index);
    }

}