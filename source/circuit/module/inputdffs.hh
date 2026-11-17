#pragma once

#include "../base/module.hh"

namespace xtaro::circuit
{

    /*
        - Ports: 
            -- clk 
            -- csb we
            -- addr0 addr1 ...
            -- din0 din1 ...
            -- csb_r we_r
            -- addr0_r addr1_r ...
            -- din0_r din1_r ...
            -- vdd gnd
    */
    class InputDFFs : public Module
    {
    public:
        static constexpr std::string_view TYPE = "input_dffs";

        /*
            - Construction: (int addressWidth, int wordWidth);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(int aw, int ww) :
                Circuit::Arguments {InputDFFs::TYPE},
                addressWidth{aw}, wordWidth{ww} {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            int addressWidth;
            int wordWidth;
        };

    public:
        InputDFFs(std::string_view name, const InputDFFs::Arguments& arguments);
        virtual ~InputDFFs() noexcept override = default;

    private:
        virtual void createPorts() override;

        virtual void createCircuits() override;
        virtual void createInstances() override;

        virtual void calculateLayoutConstants() override;
        virtual void placeInstances() override;
        virtual void routeInstances() override;
        virtual void createLayoutPins() override;
        
    private:
        int _addressWidth;
        int _wordWidth;

        Circuit* _dff;
    };

}