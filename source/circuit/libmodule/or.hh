#pragma once

#include "../base/module.hh"
#include "circuit/base/instance.hh"
#include <global/tech/driveability.hh>

namespace xtaro::circuit
{

    /*
        - Ports: A0 A1 ... An Z vdd gnd
    */
    class OR : public Module
    {
    public:
        static constexpr std::string_view TYPE = "wrap_or";
        static constexpr std::string_view OUTPUT_NAME = "Z";
        static auto INPUT_NAME(int index) -> std::string_view;

        /*
            - Construction: (DriveAbility dc, int inputSize);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(DriveAbility dc, int is) :     
                Circuit::Arguments {OR::TYPE},
                driveAbility{dc}, inputSize{is} {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            DriveAbility driveAbility;
            int inputSize;
        };

    public:
        OR(std::string_view name, const OR::Arguments& arguments);
        virtual ~OR() noexcept override = default;

    private:
        virtual void createPorts() override;

        virtual void createCircuits() override;
        virtual void createInstances() override;

        virtual void calculateLayoutConstants() override;
        virtual void placeInstances() override;
        virtual void routeInstances() override;
        virtual void createLayoutPins() override;
        
    private:
        DriveAbility _driveAbility;
        int _inputSize;

        Circuit* _nor;
        Circuit* _inv;

        Instance* _norInst;
        Instance* _invInst;
    };

}