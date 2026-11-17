#pragma once

#include "../base/module.hh"

namespace xtaro::circuit
{

    /*
        - Ports: 
            -- csb
            -- addr0 addr1 ...
            -- dout_core0[..] dout_core1[..] dout_core2[..] ...
            -- csb_core0 csb_core1 csb_core2 ...
            -- dout[..]
            -- vdd gnd
    */
    class SRAMCoreSelector : public Module
    {
    public:
        static constexpr std::string_view TYPE = "sram_core_selector";

        /*
            Construction: (int addressWidth, int wordWidth);
        */
        struct Arguments : public Circuit::Arguments 
        {
            Arguments(int aw, int ww) noexcept :
                Circuit::Arguments {SRAMCoreSelector::TYPE},
                addressWidth {aw},
                wordWidth {ww} 
            {}
                
            virtual ~Arguments() noexcept = default;
            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            int addressWidth;
            int wordWidth;
        };

    public:
        SRAMCoreSelector(std::string_view name, const SRAMCoreSelector::Arguments& arguments);
        virtual ~SRAMCoreSelector() noexcept override = default;

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

        int _outputCSBSize;

        Circuit* _decoder;
        Circuit* _inv;
        Circuit* _or;
        Circuit* _and;
        Circuit* _selectOr;
    };

}