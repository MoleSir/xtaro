#pragma once

#include "../base/module.hh"

namespace xtaro::circuit
{

    /*
        - Ports: S0 S1 ... in0 in1 ... out vdd gnd 
    */
    class Selector : public Module
    {
    public:
        static constexpr std::string_view TYPE = "selector";

        /*
            - Construction: (int selectionSize);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(int ss) :
                Circuit::Arguments {Selector::TYPE},
                selectionSize{ss} {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            int selectionSize;
        };

    public:
        enum 
        {
            MAX_SELECTION_SIZE = 4,
        };

    public:
        Selector(std::string_view name, const Selector::Arguments& arguments);
        virtual ~Selector() noexcept override = default;

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
        int _inputSize;

        Circuit* _decoder;
        Circuit* _and;
        Circuit* _or;
    };

}