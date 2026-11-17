#pragma once

#include "../base/module.hh"
#include "circuit/base/instance.hh"
#include <string_view>
#include <vector>

namespace xtaro::circuit
{

    /*
        -- Ports: 
            -- wl_in0 wl_in1 ...
            -- wl0 wl1 ...
            -- vdd gnd
    */
    class WordlineDriverArray : public Module
    {
    public:
        static constexpr std::string_view TYPE  = "wordline_deriver_array";
        static auto WORDLINE_INPUT_NAME(int index) -> std::string_view;
        static auto WORDLINE_OUTPUT_NAME(int index) -> std::string_view;
    
        /*
            - Construction: (int and_size);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(int fo, int ws) :
                Circuit::Arguments {WordlineDriverArray::TYPE},
                fanout{fo},
                wlSize{ws} {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            int fanout;
            int wlSize;
        };

    public:
        WordlineDriverArray(std::string_view name, const WordlineDriverArray::Arguments& arguments);
        virtual ~WordlineDriverArray() noexcept override = default;

    private:
        virtual void createPorts() override;
        
        virtual void createCircuits() override;
        virtual void createInstances() override;

        virtual void calculateLayoutConstants() override;
        virtual void placeInstances() override;
        virtual void routeInstances() override;
        virtual void createLayoutPins() override;

    private:
        int _fanout;
        int _wlSize;

        Circuit* _wordlineDriver;

        std::vector<Instance*> _wordlineDriverInsts {};
    };

}