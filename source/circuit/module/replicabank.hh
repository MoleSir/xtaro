#pragma once

#include "../base/module.hh"
#include <string>
#include <string_view>

namespace xtaro::circuit
{

    /*
        - Ports: wl_en p_en_bar we_en rbl vdd gnd
    */
    class ReplicaBank : public Module
    {
        enum
        {
            LINKED_BITCELL_SIZE = 2,
        };

    public:
        static constexpr std::string_view TYPE = "replica_bank";

        /*
            - Construction: (int bitcellSize);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(int bs) :
                Circuit::Arguments {ReplicaBank::TYPE},
                bitcellSize{bs} {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            int bitcellSize;
        };

    public:
        ReplicaBank(std::string_view name, const ReplicaBank::Arguments& arguments);
        virtual ~ReplicaBank() noexcept override = default;

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

        Circuit* _bitcell;
        Circuit* _prechage;
        Circuit* _writedriver;
    };

}