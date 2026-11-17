#pragma once

#include "../base/module.hh"
#include "circuit/base/circuit.hh"
#include "circuit/base/instance.hh"
#include <string>
#include <string_view>

namespace xtaro::circuit
{

    /*
        - Ports: bl0 ... bln br0 ... brn ... wl0 ... wlm rbl rbr rwl vdd gnd
    */
    class ReplicatedBitcellArray : public Module
    {
        static constexpr int LINKED_BITCELL_SIZE = 2;
        static_assert(LINKED_BITCELL_SIZE >= 1);

    public:
        static constexpr std::string_view TYPE = "replicated_bitcell_array";
        static constexpr std::string_view REPLICA_WORDLINE_NAME = "rwl";
        static constexpr std::string_view REPLICA_BITLINE_NAME = "rbl";
        static constexpr std::string_view REPLICA_BITLINE_BAR_NAME = "rbr";
        static auto BITLINE_NAME(int index) -> std::string_view;
        static auto BITLINE_BAR_NAME(int index) -> std::string_view;
        static auto WORDLINE_NAME(int index) -> std::string_view;
        
        /*
            - Construction: (int rowSize, int columnSize);
        */
        struct Arguments : public Circuit::Arguments 
        {
            Arguments(int rs, int cs) noexcept : 
                Circuit::Arguments {ReplicatedBitcellArray::TYPE},
                rowSize{rs}, columnSize{cs} {}

            virtual ~Arguments() noexcept = default;

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            int rowSize;
            int columnSize;
        };

    public:
        ReplicatedBitcellArray(std::string_view name, const ReplicatedBitcellArray::Arguments& arguments);
        virtual ~ReplicatedBitcellArray() noexcept override = default;

    private:
        virtual void createPorts() override;
        
        virtual void createCircuits() override;
        virtual void createInstances() override;

        virtual void calculateLayoutConstants() override;
        virtual void placeInstances() override;
        virtual void routeInstances() override;
        virtual void createLayoutPins() override;
        
    private:
        int _rowSize;
        int _columnSize;

        Circuit* _bitcellArray {nullptr};
        Circuit* _replicaArray {nullptr};

        Instance* _bitcellArrayInst {nullptr};
        Instance* _replicaArrayInst {nullptr};
    };

}