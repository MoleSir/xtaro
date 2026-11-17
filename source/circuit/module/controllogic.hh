#pragma once

#include "../base/module.hh"
#include "circuit/base/instance.hh"
#include "global/number/number.hh"
#include <string_view>
#include <vector>

namespace xtaro::circuit
{

    /*
        - Ports: clk csb we rbl wl_en p_en_bar sa_en we_en vdd gnd
    */
    class ControlLogic : public Module
    {
    public:
        static constexpr std::string_view TYPE = "control_logic";
        
        static constexpr std::string_view CLOCK_NAME = "clk";
        static constexpr std::string_view CHIP_SEL_BAR_NAME = "csb";
        static constexpr std::string_view WRITE_NAME = "we";
        static constexpr std::string_view REPLICA_BL_NAME = "rbl";
        static constexpr std::string_view WORDLINE_ENABLE_NAME = "wl_en";
        static constexpr std::string_view PRECHARGE_ENABLE_NAME = "p_en_bar";
        static constexpr std::string_view SENSE_AMP_ENABLE_NAME = "sa_en";
        static constexpr std::string_view WRITE_ENABLE_NAME = "we_en";

        /*
            - Construction: ();
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments() : Circuit::Arguments {ControlLogic::TYPE} {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;
        };

    public:
        ControlLogic(std::string_view name, const ControlLogic::Arguments& arguments);
        virtual ~ControlLogic() noexcept override = default;

    private:
        enum {
            CLK_INDEX = 0,
            CSB_INDEX = 1,
            WE_INDEX = 2,
            RBL_INDEX = 3,
        };

        virtual void createPorts() override;

        virtual void createCircuits() override;
        virtual void createInstances() override;

        virtual void calculateLayoutConstants() override;
        virtual void placeInstances() override;
        virtual void routeInstances() override;
        virtual void createLayoutPins() override;

    private:
        void routeInput();
        void routeInvInput();
        void routeInvOutput();

        void routeWordlineEnable();
        void routePrechargeEnable();
        void routeSenseAmpEnable();
        void routeWriteEnable();
        void routeVDDGND();

    private:
        void routePinToBus(const Pin& pin, const Number& xOff);
        
    private:
        auto inputClockXOff() -> Number;
        auto inputChipSelBarXOff() -> Number;
        auto inputWriteXOff() -> Number;
        auto inputRBLXOff() -> Number;

        auto clockXOff() -> Number;
        auto chipSelBarXOff() -> Number;
        auto writeXOff() -> Number;
        auto rblXOff() -> Number;

        auto clockBarXOff() -> Number;
        auto chipSelXOff() -> Number;
        auto writeBarXOff() -> Number;
        auto rblBarXOff() -> Number;

    private:
        Circuit* _inv {nullptr};
        Circuit* _powerInv {nullptr};
        Circuit* _or2 {nullptr};
        Circuit* _and2 {nullptr};
        Circuit* _and3 {nullptr};
        Circuit* _nand3 {nullptr};

        // Instance
        Instance* _clkInvInst {nullptr};
        Instance* _weInvInst {nullptr};
        Instance* _csbInvInst {nullptr};
        Instance* _rblInvInst {nullptr};

        Instance* _pNand3Inst {nullptr};

        Instance* _weAnd2Inst {nullptr};

        Instance* _saAnd2Inst1 {nullptr};
        Instance* _saAnd2Inst2 {nullptr};
        Instance* _saAnd2Inst {nullptr};

        Instance* _wlAnd2Inst {nullptr};
        Instance* _wlAnd3Inst {nullptr};
        Instance* _wlOr2Inst {nullptr};

        // Constant
        Layer  _busLayer {};
        Layer  _horLayer {};
        Layer  _viaLayer {};

        Number _busWidth {};
        Number _busPitch {};

        Number _horWidth {};
        Number _horPitch {};

        Number _invXOff;
        Number _busXOff;
        Number _outputInstXOff;

        std::vector<Number> _inputBusXOffs;
        std::vector<Number> _signalBusXOffs;

        Number _cellHeight;

    };

}