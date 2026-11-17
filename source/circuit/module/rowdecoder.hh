#pragma once

#include "../base/module.hh"
#include "circuit/base/circuit.hh"
#include "circuit/base/instance.hh"
#include "global/number/number.hh"
#include <string_view>

namespace xtaro::circuit
{

    /*
        - Ports: wl_en A0 A1 ... An wl0 wl1 ... wl2^n-1 vdd gnd
    */
    class RowDecoder : public Module
    {
    public:
        static constexpr std::string_view TYPE = "row_decoder";
        static constexpr std::string_view WORDLINE_ENABLE_NAME = "wl_en";
        static auto ADDRESS_NAME(int index) -> std::string_view;
        static auto WORDLINE_NAME(int index) -> std::string_view;

        /*
            - Construction: (int addressSize);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(int as) :
                Circuit::Arguments {RowDecoder::TYPE},
                addressSize{as} {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            int addressSize;
        };

    public:
        RowDecoder(std::string_view name, const RowDecoder::Arguments& arguments);
        virtual ~RowDecoder() noexcept override = default;

    private:
        virtual void createPorts() override;
        
        virtual void createCircuits() override;
        virtual void createInstances() override;

        virtual void calculateLayoutConstants() override;
        virtual void placeInstances() override;
        virtual void routeInstances() override;
        virtual void createLayoutPins() override;
        
    private:
        int _addressSize;
        int _wlSize;

        Circuit* _andArray {nullptr};
        Circuit* _decoder {nullptr};
        Circuit* _wordlineDriverArray {nullptr};

        Instance* _andArrayInst {nullptr};
        Instance* _decoderInst {nullptr};
        Instance* _wordlineDriverArrayInst {nullptr};

        Layer _routeLayer;
        Number _instGap;
    };

}