#pragma once

#include "./wrapmodule.hh"
#include <string_view>
#include <tech/tech.hh>

namespace xtaro::circuit
{

    /*
        - Ports: D Q clk vdd gnd
    */
    class DFF : public WrapModule
    {
    public:
        static constexpr std::string_view TYPE = "wrap_dff";
        static constexpr std::string_view DIN_NAME = "D";
        static constexpr std::string_view DOUT_NAME = "Q";
        static constexpr std::string_view CLOCK_NAME = "clk";

        /*
            Construction: ()
        */
        struct Arguments : public Circuit::Arguments 
        {
            Arguments() noexcept : Circuit::Arguments {DFF::TYPE} {}
            virtual ~Arguments() noexcept = default;
            
            virtual auto toString() const -> std::string override 
            { return ""; }

            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override
            { return std::make_unique<DFF>(circuitName, *this); }
        };

    public:
        DFF(std::string_view name, const DFF::Arguments& arguments) : WrapModule{name}
        { this->loadFromLibrary(); }

        virtual ~DFF() noexcept override = default;

    private:
        virtual void createPorts() override 
        {
            this->addPortNet(DIN_NAME, PortType::Input );
            this->addPortNet(DOUT_NAME, PortType::Output );
            this->addPortNet(CLOCK_NAME, PortType::Input );
            this->addPortNet(tech->vdd(), PortType::Input );
            this->addPortNet(tech->gnd(), PortType::Input );
        }
    };

}