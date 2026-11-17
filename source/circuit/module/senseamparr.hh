#pragma once

#include "../base/module.hh"
#include <string_view>

namespace xtaro::circuit
{

    /*
        - Ports: bl0 bl1 ... br0 br1 ... dout0 dout1 ... sa_en vdd gnd
    */
    class SenseAmplifierArray : public Module
    {
    public:
        static constexpr std::string_view TYPE = "sense_amp_array";
        static constexpr std::string_view ENABLE_NAME = "sa_en";
        static auto BITLINE_NAME(int index) -> std::string_view;
        static auto BITLINE_BAR_NAME(int index) -> std::string_view;
        static auto DATA_OUTPUT_NAME(int index) -> std::string_view;

        /*
            - Construction: (int columnSize);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(int cs, int scs, bool buf = false) :
                Circuit::Arguments {SenseAmplifierArray::TYPE},
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
        SenseAmplifierArray(std::string_view name, const SenseAmplifierArray::Arguments& arguments);
        virtual ~SenseAmplifierArray() noexcept override = default;

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

        Circuit* _senseamp {nullptr};
        Circuit* _fanoutbuf {nullptr};

        std::vector<Instance*> _senseampInsts {};

        Number _intervalX {};
    };

}