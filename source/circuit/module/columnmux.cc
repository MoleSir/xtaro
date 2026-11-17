#include "./columnmux.hh"
#include "circuit/libmodule/columntrigate.hh"
#include "circuit/base/circuit.hh"
#include "circuit/base/module.hh"
#include "circuit/geometry/coord.hh"
#include "global/tech/layer.hh"
#include "global/tech/tech.hh"

#include <circuit/module/decoder.hh>
#include <circuit/libmodule/inv.hh>
#include <circuit/module/trigate.hh>

#include <circuit/mod.hh>

#include <ranges>
#include <string_view>
#include <utility/misc/string.hh>
#include <utility/misc/math.hh>
#include <utility/debug/debug.hh>
#include <vector>

namespace xtaro::circuit
{
    std::string ColumnMux::Arguments::toString() const
    {
        return std::format("ss{}", this->selectionSize);
    }

    auto ColumnMux::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<ColumnMux>(circuitName, *this);
    }

    ColumnMux::ColumnMux(std::string_view name, const ColumnMux::Arguments& arguments) :
        Module{name},
        _selectionSize{arguments.selectionSize},

        _columnTrigate{nullptr}
    {
        debug::checkf(this->_selectionSize >= 1, "Selection size '{}' < 1", this->_selectionSize);
        this->createSelf();
    }

    void ColumnMux::createPorts()
    {
        for (int i = 0; i < this->_selectionSize; ++i)
            this->addPortNet(this->SELECTION_NAME(i), PortType::Input);

        for (int i = 0; i < this->_selectionSize; ++i)
            this->addPortNet(this->BITLINE_NAME(i), PortType::InOut);
        for (int i = 0; i < this->_selectionSize; ++i)
            this->addPortNet(this->BITLINE_BAR_NAME(i), PortType::InOut);

        this->addPortNet(this->OUTPUT_BITLINE_NAME, PortType::InOut);
        this->addPortNet(this->OUTPUT_BITLINE_BAR_NAME, PortType::InOut);
    
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void ColumnMux::createCircuits()
    {
        this->_columnTrigate = this->addCircuit(ColumnTristateGate::Arguments{});
    }

    void ColumnMux::createInstances() 
    {
        for (int i = 0; i < this->_selectionSize; ++i)
        {
            // bl br bl_out br_out sel gnd
            auto inst = this->addInstance(
                stringf("column_mux_{}", i),
                this->_columnTrigate,
                {
                    this->BITLINE_NAME(i), this->BITLINE_BAR_NAME(i),
                    this->OUTPUT_BITLINE_NAME, this->OUTPUT_BITLINE_BAR_NAME,
                    this->SELECTION_NAME(i),
                    tech->vdd(), tech->gnd()
                }
            );
            this->_columnTrigateInsts.emplace_back(inst);
        }
    }


    /*
    
        +-----+-----+-----+-----+
        |     |     |     |     |
        |     |     |     |     | column tri gates
        |     |     |     |     |
        |     |     |     |     |
        +-----+-----+-----+-----+

        ------------------------- sel0
        ------------------------- sel1
        ------------------------- sel2
        ------------------------- sel3
         --------------------     bl
            --------------------  br

    */

    void ColumnMux::calculateLayoutConstants()
    {
        this->_busLayer = Layer::Metal1;
        this->_busPitch = tech->layerPitch(this->_busLayer);
        this->_busWidth = tech->layerWidth(this->_busLayer);

        this->_vertLayer = Layer::Metal2;
        this->_vertPitch = tech->layerPitch(this->_vertLayer);
        this->_vertWidth = tech->layerWidth(this->_vertLayer);

        this->_viaLayer = Layer::Via1;

        this->_outputBRPinY = 0._u;
        this->_outputBLPinY = this->_outputBRPinY + this->_busPitch;

        auto y = this->_outputBLPinY;
        for (int sel = 0; sel < this->_selectionSize; ++sel) {
            y += this->_busPitch;
            this->_selectionBusYOff.emplace_back(y);
        }

        this->_instBiasY = y + this->_busPitch;

        this->_width = this->_selectionSize * this->_columnTrigate->width();
        this->_height = this->_instBiasY + this->_columnTrigate->height();
    }

    void ColumnMux::placeInstances()
    {
        Module::placeInstArrayInRow(this->_columnTrigateInsts, this->_instBiasY, 0._u);
        
        auto firstBLPin = this->_columnTrigateInsts.front()->getPin(ColumnTristateGate::OUTPUT_BITLINE_NAME);
        auto firstBRPin = this->_columnTrigateInsts.front()->getPin(ColumnTristateGate::OUTPUT_BITLINE_BAR_NAME);

        auto lastBLPin = this->_columnTrigateInsts.back()->getPin(ColumnTristateGate::OUTPUT_BITLINE_NAME);
        auto lastBRPin = this->_columnTrigateInsts.back()->getPin(ColumnTristateGate::OUTPUT_BITLINE_BAR_NAME);
    
        this->_firstBLPinX = firstBLPin.minX();
        this->_firstBRPinX = firstBRPin.minX();

        this->_outputBLPinLength = lastBLPin.maxX() - this->_firstBLPinX;
        this->_outputBRPinLength = lastBRPin.maxX() - this->_firstBRPinX;
    }

    void ColumnMux::routeInstances()
    {
        // Add bus
        auto selNames = std::vector<std::string_view>{};
        for (auto i : std::ranges::views::iota(0, this->_selectionSize)) {
            selNames.emplace_back(ColumnMux::SELECTION_NAME(i));
        }

        this->routeRowBusPin(
            this->_busLayer, 
            this->width(), 
            this->_busWidth, 
            0._u, 
            this->_selectionBusYOff, 
            selNames);

        this->addRectangle(
            this->_busLayer,
            Coord {this->_firstBLPinX, this->_outputBLPinY},
            this->_outputBLPinLength,
            this->_busWidth);

        this->addRectangle(
            this->_busLayer,
            Coord {this->_firstBRPinX, this->_outputBRPinY},
            this->_outputBRPinLength,
            this->_busWidth);

        // Route trigate
        for (int i = 0; i < this->_columnTrigateInsts.size(); ++i) {
            auto inst = this->_columnTrigateInsts[i];
            auto outputBLPin = inst->getPin(ColumnTristateGate::OUTPUT_BITLINE_NAME);
            auto outputBRPin = inst->getPin(ColumnTristateGate::OUTPUT_BITLINE_BAR_NAME);
            auto selectionPin = inst->getPin(ColumnTristateGate::SELECTION_NAME);

            // Route selection
            auto begin = Coord{selectionPin.cenX(), this->_selectionBusYOff[i] + this->_vertWidth / 2.};
            auto end = selectionPin.center();
            this->addRectangleSegmentCenter(this->_vertLayer, begin, end);
            this->addViaCenter(this->_viaLayer, begin);
            if (selectionPin.layer() != this->_vertLayer) 
                this->addViaCenter(this->_viaLayer, end);
            
            // All tri_gate's bl should route to bl_out
            auto blbegin = Coord{outputBLPin.cenX(), this->_outputBLPinY + this->_vertWidth / 2.};
            auto blend = outputBLPin.center();
            this->addRectangleSegmentCenter(this->_vertLayer, blbegin, blend);
            this->addViaCenter(this->_viaLayer, blbegin);
            if (outputBLPin.layer() != this->_vertLayer) 
                this->addViaCenter(this->_viaLayer, blend);

            // All tri_gate's br should route to bl_out
            auto brbegin = Coord{outputBRPin.cenX(), this->_outputBRPinY + this->_vertWidth / 2.};
            auto brend = outputBRPin.center();
            this->addRectangleSegmentCenter(this->_vertLayer, brbegin, brend);
            this->addViaCenter(this->_viaLayer, brbegin);
            if (outputBRPin.layer() != this->_vertLayer) 
                this->addViaCenter(this->_viaLayer, brend);

            if (i == 0) 
            {
                this->addLabelPinCenter(this->_vertLayer, this->OUTPUT_BITLINE_NAME, blbegin);
                this->addLabelPinCenter(this->_vertLayer, this->OUTPUT_BITLINE_BAR_NAME, brbegin);
            }
        } 
    }

    void ColumnMux::createLayoutPins()
    {
        for (int i = 0; i < this->_columnTrigateInsts.size(); ++i) {
            auto inst = this->_columnTrigateInsts[i];

            // this->copyInstancePins(inst, ColumnTristateGate::SELECTION_NAME, ColumnMux::SELECTION_NAME(i));
            this->copyInstancePins(inst, ColumnTristateGate::BITLINE_NAME, ColumnMux::BITLINE_NAME(i));
            this->copyInstancePins(inst, ColumnTristateGate::BITLINE_BAR_NAME, ColumnMux::BITLINE_BAR_NAME(i));
            this->copyInstancePins(inst, tech->vdd(), tech->vdd());
            this->copyInstancePins(inst, tech->gnd(), tech->gnd());
        }
    }
    
    auto ColumnMux::SELECTION_NAME(int index) -> std::string_view 
    {
        return stringf("sel{}", index);
    }
    
    auto ColumnMux::BITLINE_NAME(int index) -> std::string_view 
    {
        return stringf("bl{}", index);
    }
    
    auto ColumnMux::BITLINE_BAR_NAME(int index) -> std::string_view 
    {
        return stringf("br{}", index);
    }

}