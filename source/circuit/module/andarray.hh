#pragma once

#include "../base/module.hh"
#include "circuit/base/instance.hh"
#include "global/tech/layer.hh"
#include <string_view>
#include <vector>

namespace xtaro::circuit
{

    /*
        -- Ports: 
            -- A0 A1 ...
            -- en 
            -- Z0 Z1 ...
            -- vdd gnd
    */
    class ANDArray : public Module
    {
    public:
        static constexpr std::string_view TYPE = "and_array";
        static constexpr std::string_view ENABLE_NAME = "en";
        static auto INPUT_NAME(int index) -> std::string_view;
        static auto OUTPUT_NAME(int index) -> std::string_view;

        /*
            - Construction: (int and_size);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(int as, bool buf = false) :
                Circuit::Arguments {ANDArray::TYPE},
                andSize{as},
                useFanoutBuffer{buf} {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            int andSize;
            bool useFanoutBuffer;
        };

    public:
        ANDArray(std::string_view name, const ANDArray::Arguments& arguments);
        virtual ~ANDArray() noexcept override = default;

    private:
        virtual void createPorts() override;
        
        virtual void createCircuits() override;
        virtual void createInstances() override;

        virtual void calculateLayoutConstants() override;
        virtual void placeInstances() override;
        virtual void routeInstances() override;
        virtual void createLayoutPins() override;

    private:
        int _andSize;
        bool _useFanoutBuffer;

        Circuit* _and {nullptr};
        Circuit* _fanoutbuf {nullptr};

        std::vector<Instance*> _andInsts {};

        Layer _enableLayer;
    };

}