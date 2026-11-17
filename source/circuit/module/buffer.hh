#pragma once

#include "../base/module.hh"
#include "circuit/base/instance.hh"
#include <global/tech/driveability.hh>
#include <string_view>

namespace xtaro::circuit
{

    /*
        - Ports: in out vdd gnd;
    */
    class Buffer : public Module
    {
    public:
        static constexpr std::string_view TYPE = "buffer";
        static constexpr std::string_view INPUT_NAME = "in";
        static constexpr std::string_view OUTPUT_NAME = "out";

        /*
            Arguments (DriveAbility dc)
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(DriveAbility dc) : 
                Circuit::Arguments {Buffer::TYPE},
                driveAbility{dc} {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;
            
            DriveAbility driveAbility;
        };

    public:
        Buffer(std::string_view name, const Buffer::Arguments& arguments);
        virtual ~Buffer() noexcept override = default;

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
        
        Circuit* _inv;

        Instance* _invInst1;
        Instance* _invInst2;
    };

}