#pragma once

#include "../base/module.hh"
#include "circuit/base/instance.hh"
#include "global/tech/layer.hh"
#include <string>
#include <string_view>
#include <vector>

namespace xtaro::circuit
{

    /*
        - Ports: rbl rbr wl vdd gnd
    */
    class ReplicaBitcellArray : public Module
    {
        static constexpr int LINKED_BITCELL_SIZE = 2;
        static_assert(LINKED_BITCELL_SIZE >= 1);

    public:
        static constexpr std::string_view TYPE = "replica_bitcell_array";
        static constexpr std::string_view REPLICA_BITLINE_NAME = "rbl";
        static constexpr std::string_view REPLICA_BITLINE_BAR_NAME = "rbr";
        static constexpr std::string_view REPLICA_WORDLINE_NAME = "rwl";

        /*
            - Construction: (int bitcellSize);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(int bs) :
                Circuit::Arguments {ReplicaBitcellArray::TYPE},
                bitcellSize{bs} {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            int bitcellSize;
        };

    public:
        ReplicaBitcellArray(std::string_view name, const ReplicaBitcellArray::Arguments& arguments);
        virtual ~ReplicaBitcellArray() noexcept override = default;

    private:
        virtual void createPorts() override;
        
        virtual void createCircuits() override;
        virtual void createInstances() override;

        virtual void calculateLayoutConstants() override;
        virtual void placeInstances() override;
        virtual void routeInstances() override;
        virtual void createLayoutPins() override;
        
    private:
        int _bitcellSize;

        Circuit* _bitcell {nullptr};

        std::vector<Instance*> _bitcellInsts {};

        Layer _bitlineLayer;
        Layer _wordlineLayer;
    };

}