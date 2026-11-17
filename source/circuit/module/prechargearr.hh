#pragma once

#include "../base/module.hh"
#include "circuit/base/instance.hh"
#include <string_view>
#include <vector>

namespace xtaro::circuit
{

    /*
        - Ports: bl0 bl1 ... br0 br1 ... p_en_bar vdd gnd
    */
    class PrechargeArray : public Module
    {
        enum 
        {
            MAX_FANOUT = 4,
        };

    public:
        static constexpr std::string_view TYPE = "precharge_array";

        static constexpr std::string_view ENABLE_NAME = "p_en_bar";
        static auto BITLINE_NAME(int index) -> std::string_view;
        static auto BITLINE_BAR_NAME(int index) -> std::string_view;

        /*
            - Construction: (int wordWidth);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(int cs, bool buf = false) :
                Circuit::Arguments {PrechargeArray::TYPE},
                columnSize{cs},
                useFanoutBuffer{buf} {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;
            
            int columnSize;
            bool useFanoutBuffer;
        };

    public:
        PrechargeArray(std::string_view name, const PrechargeArray::Arguments& arguments);
        virtual ~PrechargeArray() noexcept override = default;

    private:
        virtual void createPorts() override;

        virtual void createCircuits() override;
        virtual void createInstances() override;

        virtual void calculateLayoutConstants() override;
        virtual void placeInstances() override;
        virtual void routeInstances() override;
        virtual void createLayoutPins() override;
        
    private:
        int _columnSize;
        bool _useFanoutBuffer;

        Circuit* _precharge {nullptr};
        Circuit* _fanoutbuf {nullptr};

        std::vector<Instance*> _prechargeInsts {};
    };

}