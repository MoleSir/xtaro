#pragma once

#include "../base/module.hh"

namespace xtaro::circuit
{

    /*

        - Ports
            -- clk csb we

            -- rsel0 rsel1 ...
            -- rcol0 rcol1 ...
            
            -- din0 din1 ...
            -- dout0 dout1 ...
            
            -- vdd gnd

    */
    class SRAMCore : public Module
    {
    public:
        enum 
        {
            MAX_CORE_ROW   = 64,
            MAX_CORE_COL   = 128,

            MAX_CORE_BIT_SIZE  = MAX_CORE_ROW * MAX_CORE_COL,
        };

        static constexpr std::string_view TYPE = "sram_core";

        /*
            - Construction: (int rowSelSize, int colSelSize, int wordWidth);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(int rss, int css, int ww) :
                Circuit::Arguments {SRAMCore::TYPE},
                rowSelSize{rss}, 
                colSelSize{css},
                wordWidth{ww}
            {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            int rowSelSize;
            int colSelSize;
            int wordWidth;
        };

    public:
        SRAMCore(std::string_view name, const SRAMCore::Arguments& arguments);
        virtual ~SRAMCore() noexcept override = default;

    private:
        virtual void createPorts() override;

        virtual void createCircuits() override;
        virtual void createInstances() override;

        virtual void calculateLayoutConstants() override;
        virtual void placeInstances() override;
        virtual void routeInstances() override;
        virtual void createLayoutPins() override;
        
    private:
        auto hasColAddress() const -> bool { return this->_colSelSize > 1; }

    private:
        int _rowSelSize;
        int _colSelSize;
        int _wordWidth;
        
        int _arrayColSize;
        int _bitcellSize;

        Circuit* _perpheralBitcellArray;
        Circuit* _datapath;
        Circuit* _controllogic;
    };

}