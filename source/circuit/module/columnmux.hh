#pragma once

#include "../base/module.hh"
#include "circuit/base/instance.hh"
#include "global/number/number.hh"
#include "global/tech/layer.hh"
#include <string_view>
#include <vector>

namespace xtaro::circuit
{

    /*
        - Ports: 
            -- sel0 sel1 ... sel2^n-1
            -- bl0 bl1 ... bl2^n-1 
            -- br0 br1 ... br2^n-1 
            -- bl br 
            -- vdd gnd 
    */
    class ColumnMux : public Module
    {
    public:
        static constexpr std::string_view TYPE = "column_mux";
        static auto SELECTION_NAME(int index) -> std::string_view;
        static auto BITLINE_NAME(int index) -> std::string_view;
        static auto BITLINE_BAR_NAME(int index) -> std::string_view;
        static constexpr std::string_view OUTPUT_BITLINE_NAME = "bl";
        static constexpr std::string_view OUTPUT_BITLINE_BAR_NAME = "br";

        /*
            - Construction: (int selectionSize);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(int ss) :
                Circuit::Arguments {ColumnMux::TYPE},
                selectionSize{ss} {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            int selectionSize;
        };

    public:
        ColumnMux(std::string_view name, const ColumnMux::Arguments& arguments);
        virtual ~ColumnMux() noexcept override = default;

    private:
        virtual void createPorts() override;

        virtual void createCircuits() override;
        virtual void createInstances() override;

        virtual void calculateLayoutConstants() override;
        virtual void placeInstances() override;
        virtual void routeInstances() override;
        virtual void createLayoutPins() override;
        
    private:
        int _selectionSize;

        Circuit* _columnTrigate;

        std::vector<Instance*> _columnTrigateInsts;

        std::vector<Number> _selectionBusYOff;
        
        Layer _busLayer;
        Number _busWidth;
        Number _busPitch;

        Layer _vertLayer;
        Number _vertWidth;
        Number _vertPitch;

        Layer _viaLayer;

        Number _firstBLPinX;
        Number _firstBRPinX;

        Number _outputBLPinLength;
        Number _outputBRPinLength;
        Number _outputBLPinY;
        Number _outputBRPinY;

        Number _instBiasY;
    };

}