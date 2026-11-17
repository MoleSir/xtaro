#pragma once

#include "../base/module.hh"

namespace xtaro::circuit
{

    /*
        - Ports: in out en en_bar vdd gnd
    */
    class Test : public Module
    {
    public:
        static constexpr std::string_view TYPE = "test";

        struct Arguments : public Circuit::Arguments
        {
            Arguments() noexcept : Circuit::Arguments {Test::TYPE} {}
            virtual ~Arguments() noexcept override {}

            virtual std::string toString() const override 
            { return ""; }
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;
        };

    public:
        Test(std::string_view name, const Test::Arguments& arguments);
        virtual ~Test() noexcept override = default;

    private:
        virtual void createPorts() override;

        virtual void createCircuits() override;
        virtual void createInstances() override;
    
        virtual void calculateLayoutConstants() override;
        virtual void placeInstances() override;
        virtual void routeInstances() override;
        virtual void createLayoutPins() override;
    };

}