#pragma once

#include "../base/module.hh"

namespace xtaro::circuit
{

    /*
        - Ports:
          -- mbist
          -- csb we
          -- addr0 addr1 ...
          -- din0 din1 ...
          -- bist_csb bist_we
          -- bist_addr0 bist_addr1 ...
          -- bist_din0 bist_din1 ...
          -- sram_csb sram_we
          -- sram_addr0 sram_addr1 ...
          -- sram_din0 sram_din1 ...
          -- vdd gnd
    */
    class MBISTSelector : public Module
    {
    public:
        static constexpr std::string_view TYPE = "mbist_selector";

        /*
            - Construction: (int addressWidth, int wordWidth);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(int aw, int ww) :
                Circuit::Arguments {MBISTSelector::TYPE},
                addressWidth{aw}, 
                wordWidth{ww} {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            int addressWidth;
            int wordWidth;
        };

    public:
        enum 
        {
            MAX_SELECTION_SIZE = 4,
        };

    public:
        MBISTSelector(std::string_view name, const MBISTSelector::Arguments& arguments);
        virtual ~MBISTSelector() noexcept override = default;

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

        int _fanoutBufSize;

        Circuit* _selector;
        Circuit* _fanoutbuf;
    };

}