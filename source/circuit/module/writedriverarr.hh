#pragma once

#include "../base/module.hh"
#include "circuit/base/instance.hh"
#include "global/number/number.hh"
#include <string_view>
#include <vector>

namespace xtaro::circuit
{

    /*
        - Ports: din0 din1 ... bl0 bl1 ... br0 br1 ... we_en vdd gnd
    */
    class WriteDriverArray : public Module
    {
    public:
        static constexpr std::string_view TYPE = "write_driver_array";
        static constexpr std::string_view ENABLE_NAME = "we_en";
        static auto BITLINE_NAME(int index) -> std::string_view;
        static auto BITLINE_BAR_NAME(int index) -> std::string_view;
        static auto DATA_INPUT_NAME(int index) -> std::string_view;
        /*
            - Construction: (int columnSize);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(int cs, int scs, bool buf = false) :
                Circuit::Arguments {WriteDriverArray::TYPE},
                columnSize{cs},
                spareColumnSize{scs},
                useFanoutBuffer{buf} {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            int columnSize;
            int spareColumnSize;
            bool useFanoutBuffer;
        };

    public:
        WriteDriverArray(std::string_view name, const WriteDriverArray::Arguments& arguments);
        virtual ~WriteDriverArray() noexcept override = default;

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
        int _spareColumnSize;
        bool _useFanoutBuffer;

        Circuit* _writedriver {nullptr};
        Circuit* _fanoutbuf {nullptr};

        std::vector<Instance*> _writedriverInsts {};

        Number _intervalX {};
    };

}