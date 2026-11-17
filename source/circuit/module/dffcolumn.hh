#pragma once

#include "../base/module.hh"
#include "circuit/base/circuit.hh"
#include "circuit/base/instance.hh"
#include <string_view>
#include <vector>

namespace xtaro::circuit
{

    /*
        - Ports: 
            D0 D1 ... 
            Q0 Q1 ... 
            clk 
            vdd gnd
    */
    class DFlipFlopColumn : public Module
    {
    public:
        static constexpr std::string_view TYPE = "dff_column";
        static constexpr std::string_view CLOCK_NAME = "clk";
        static auto DIN_NAME(int index) -> std::string_view;
        static auto DOUT_NAME(int index) -> std::string_view;

        /*
            - Construction: (int dffSize);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(int ds) :
                Circuit::Arguments {DFlipFlopColumn::TYPE},
                dffSize{ds} {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            int dffSize;
        };

    public:
        DFlipFlopColumn(std::string_view name, const DFlipFlopColumn::Arguments& arguments);
        virtual ~DFlipFlopColumn() noexcept override = default;

    private:
        virtual void createPorts() override;

        virtual void createCircuits() override;
        virtual void createInstances() override;

        virtual void calculateLayoutConstants() override;
        virtual void placeInstances() override;
        virtual void routeInstances() override;
        virtual void createLayoutPins() override;
        
    private:
        int _dffSize;

        Circuit*  _dff {nullptr};
        std::vector<Instance*> _dffInsts {};
    };

}