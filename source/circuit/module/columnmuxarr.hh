#pragma once

#include "../base/module.hh"
#include "circuit/base/instance.hh"
#include <string_view>
#include <vector>

namespace xtaro::circuit
{

    /*
        - Ports: 
            -- sel0 sel1 ...
            
            -- bl0_0 bl0_1 ... 
            -- br0_0 br0_1 ...
            -- bl1_0 bl1_1 ...
            -- br1_0 br1_1 ...
            -- ...
            -- blm-1_0 blm-1_1 ...
            -- brm-1_0 brm-1_1 ...

            -- bl0 br0 bl1 br1 ...

            -- vdd gnd
    */
    class ColumnMuxArray : public Module
    {
    public:
        static constexpr std::string_view TYPE = "column_mux_array";
        static auto SELECTION_NAME(int index) -> std::string_view;
        static auto BITLINE_NAME(int muxIndex, int inputIdx) -> std::string_view;
        static auto BITLINE_BAR_NAME(int muxIndex, int inputIdx) -> std::string_view;
        static auto OUTPUT_BITLINE_NAME(int index) -> std::string_view;
        static auto OUTPUT_BITLINE_BAR_NAME(int index) -> std::string_view;
    
        /*
            - Construction: (int selectionSize, int muxSize);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(int ss, int ms, bool buf = false) :
                Circuit::Arguments {ColumnMuxArray::TYPE},
                selSize{ss}, muxSize{ms}, useFanoutBuffer{buf} {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            int selSize;
            int muxSize;
            bool useFanoutBuffer;
        };

    public:
        ColumnMuxArray(std::string_view name, const ColumnMuxArray::Arguments& arguments);
        virtual ~ColumnMuxArray() noexcept override = default;

    private:
        virtual void createPorts() override;

        virtual void createCircuits() override;
        virtual void createInstances() override;

        virtual void calculateLayoutConstants() override;
        virtual void placeInstances() override;
        virtual void routeInstances() override;
        virtual void createLayoutPins() override;
        
    private:
        int _selSize;
        int _muxSize;
        bool _useFanoutBuffer;

        Circuit* _mux {nullptr};
        Circuit* _fanoutbuf {nullptr};

        std::vector<Instance*> _muxInsts {};
    };

}