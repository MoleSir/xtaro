#pragma once

#include "../base/module.hh"
#include <global/tech/driveability.hh>
#include <string_view>

namespace xtaro::circuit
{

    /*
        - Ports: A0 A1 ... An Z vdd gnd
    */
    class NAND : public Module
    {
    public:
        static constexpr std::string_view TYPE = "wrap_nand";
        static constexpr std::string_view OUTPUT_NAME = "Z";
        static auto INPUT_NAME(int index) -> std::string_view;

        /*
            - Construction: (DriveAbility dc, int inputSize);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(DriveAbility dc, int is) : 
                Circuit::Arguments {NAND::TYPE},
                driveAbility{dc}, inputSize{is} {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            DriveAbility driveAbility;
            int inputSize;
        };

    public:
        NAND(std::string_view name, const NAND::Arguments& arguments);
        virtual ~NAND() noexcept override = default;

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

        Circuit* _nmos;
        Circuit* _pmos;
    };

}