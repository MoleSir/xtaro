#pragma once

#include "../base/module.hh"
#include "circuit/base/circuit.hh"
#include "circuit/base/instance.hh"
#include "circuit/geometry/coord.hh"
#include "global/number/number.hh"
#include <array>
#include <string_view>

namespace xtaro::circuit
{

    /*
        - Ports:
            -- clk csb we
            -- addr0 addr1 ...
            -- din0 din1 ...
            -- dout0 dout1 ...
            -- vdd gnd

    */
    class SRAM1Bank : public Module
    {
    public:
        static constexpr std::string_view TYPE = "sram_1_bank";
        static constexpr std::string_view CLOCK_NAME = "clk";
        static constexpr std::string_view CHIP_SEL_BAR_NAME = "csb";
        static constexpr std::string_view WRITE_ENABLE_NAME = "we";
        static auto ADDRESS_NAME(int index) -> std::string_view;
        static auto DATA_INPUT_NAME(int index) -> std::string_view;
        static auto DATA_OUTPUT_NAME(int index) -> std::string_view;

    private:
        static auto REG_ADDRESS_NAME(int index) -> std::string_view;
        static auto REG_DATA_INPUT_NAME(int index) -> std::string_view;
        static constexpr std::string_view REG_CHIP_SEL_BAR_NAME = "csb_reg";
        static constexpr std::string_view REG_WRITE_ENABLE_NAME = "we_reg";

    public:
        /*
            - Construction: (int addressWidth, int wordWidth, bool mbist);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(int aw, int caw, int ww) :
                Circuit::Arguments {SRAM1Bank::TYPE},
                addressWidth{aw},
                colAddressWdith{caw},
                wordWidth{ww}
            {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            int addressWidth;
            int colAddressWdith;
            int wordWidth;
        };

    public:
        SRAM1Bank(std::string_view name, const SRAM1Bank::Arguments& arguments);
        virtual ~SRAM1Bank() noexcept override = default;

    private:
        virtual void createPorts() override;

        virtual void createCircuits() override;
        virtual void createInstances() override;

        virtual void calculateLayoutConstants() override;
        virtual void placeInstances() override;
        virtual void routeInstances() override;
        virtual void createLayoutPins() override;
        virtual void layoutMiscellaneous() override;

    private:
        // Route instance ...
        void routeColDecoder();
        void routePrechargeArray();
        void routeRowDecoder();
        void routeControlLogic();
        void routeRBL();
        void routeDinDFlipFlops();
        void routeAddrDFlipFlops();
        void routeControlDFlipFlops();
        void routeVDDGND();
        void routeClock();
        void routeIO();
        
    private:
        void routeSourceRingCenter(const Coord& leftdown, const Coord& rightup, const Number& width);

    private:
        int _addressWidth;
        int _colAddressWidth;
        int _wordWidth;

        int _rowAddressWidth;

        int _columnSelSize;
        int _bitcellRowSize;
        int _bitcellColSize;

        Circuit*  _replicatedBitcellArray {nullptr};
        Circuit*  _prechargeArray {nullptr};
        Circuit*  _rowDecoder {nullptr};
        Circuit*  _colDecoder {nullptr};
        Circuit*  _datapath {nullptr};
        Circuit*  _controlLogic {nullptr};
        Circuit*  _writeDriver {nullptr};
        Circuit*  _buffer {nullptr};
        Circuit*  _dinDffs {nullptr};
        Circuit*  _addressDffs {nullptr};
        Circuit*  _controlSignalDffs {nullptr};
        
        Instance* _replicatedBitcellArrayInst {nullptr};
        Instance* _prechargeArrayInst {nullptr};
        Instance* _rowDecoderInst {nullptr};
        Instance* _colDecoderInst {nullptr};
        Instance* _datapathInst {nullptr};
        Instance* _controlLogicInst {nullptr};
        Instance* _writeDriverInst {nullptr};
        Instance* _bufferInst {nullptr};
        Instance* _dinDffsInst {nullptr};
        Instance* _addressDffsInst {nullptr};
        Instance* _controlSignalDffsInst {nullptr};

        Coord  _replicatedBitcellArrayPos {};
        Coord  _prechargeArrayPos {};
        Coord  _rowDecoderPos {};
        Coord  _colDecoderPos {};
        Coord  _datapathPos {};
        Coord  _controlLogicPos {};
        Coord  _writeDriverPos {};
        Coord  _bufferPos {};
        Coord  _dinDffsPos {};
        Coord  _addressDffsPos {};
        Coord  _controlSignalDffsPos {};

        Number _horiInstGap {};
        Number _vertInstGap {};

        Layer  _horiLayer;
        Number _horiPitch;
        Number _horiWidth;

        Layer  _vertLayer;
        Number _vertWidth;
        Number _vertPitch;

        Layer  _viaLayer;

        Layer  _higherHoriLayer;
        Number _higherHoriPitch;
        Number _higherHoriWidth;

        Layer  _higherVertLayer;
        Number _higherVertPitch;
        Number _higherVertWidth;

        Layer  _higherViaLayer;

        enum {
            P_EN_INDEX = 0,
            WL_EN_INDEX = 1,
            SA_EN_INDEX = 2,
            WE_EN_INEDX = 3,
            RBL_INDEX = 4,
        };

        std::array<Number, 5> _controlBusXOffs;
    };

}