#pragma once

#include "../base/module.hh"
#include <global/tech/driveability.hh>

namespace xtaro::circuit
{

    /*
        - Ports: A Z vdd gnd
    */
    class INV : public Module
    {
    public:
        static constexpr std::string_view TYPE = "wrap_inv";
        static constexpr std::string_view INPUT_NAME = "A";
        static constexpr std::string_view OUTPUT_NAME = "Z";

        /*
            - Construction: (DriveAbility dc);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(DriveAbility dc) : 
                Circuit::Arguments {INV::TYPE},
                driveAbility{dc} {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            DriveAbility driveAbility;
        };

    public:
        INV(std::string_view name, const INV::Arguments& arguments);
        virtual ~INV() noexcept override = default;

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

        Circuit* _nmos;
        Circuit* _pmos;
    };

}