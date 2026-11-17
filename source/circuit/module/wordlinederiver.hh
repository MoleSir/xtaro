#pragma once

#include "../base/module.hh"
#include "circuit/base/circuit.hh"
#include "circuit/base/instance.hh"
#include <string_view>

namespace xtaro::circuit
{

    /*
        - Ports: wl_in wl vdd gnd
    */
    class WordlineDriver : public Module
    {
    public:
        static constexpr std::string_view TYPE = "wordline_driver";
        static constexpr std::string_view WORDLINE_INPUT_NAME = "wl_in";
        static constexpr std::string_view WORDLINE_OUTPUT_NAME = "wl";

        struct Arguments : public Circuit::Arguments
        {
            Arguments(int fo) noexcept : 
                Circuit::Arguments {WordlineDriver::TYPE},
                fanout{fo}
            {}
            virtual ~Arguments() noexcept override {}

            virtual std::string toString() const override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            int fanout;
        };

    public:
        WordlineDriver(std::string_view name, const WordlineDriver::Arguments& arguments);
        virtual ~WordlineDriver() noexcept override = default;

    private:
        virtual void createPorts() override;

        virtual void createCircuits() override;
        virtual void createInstances() override;
    
        virtual void calculateLayoutConstants() override;
        virtual void placeInstances() override;
        virtual void routeInstances() override;
        virtual void createLayoutPins() override;

    private:
        int _fanout;

        Circuit* _buffer;

        Instance* _bufferInst;
    };

}