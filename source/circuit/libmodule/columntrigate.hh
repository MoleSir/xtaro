#pragma once

#include "./wrapmodule.hh"
#include <string_view>
#include <tech/tech.hh>

namespace xtaro::circuit
{

    /*
        - Ports: bl br bl_out br_out sel vdd gnd
    */
    class ColumnTristateGate : public WrapModule
    {
    public:
        static constexpr std::string_view TYPE = "wrap_column_tri_gate";
        static constexpr std::string_view BITLINE_NAME = "bl";
        static constexpr std::string_view BITLINE_BAR_NAME = "br";
        static constexpr std::string_view OUTPUT_BITLINE_NAME = "bl_out";
        static constexpr std::string_view OUTPUT_BITLINE_BAR_NAME = "br_out";
        static constexpr std::string_view SELECTION_NAME = "sel";

        /*
            Construction: ()
        */
        struct Arguments : public Circuit::Arguments 
        {
            Arguments() noexcept : Circuit::Arguments {ColumnTristateGate::TYPE} {}
            virtual ~Arguments() noexcept = default;
            
            virtual auto toString() const -> std::string override 
            { return ""; }

            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override
            { return std::make_unique<ColumnTristateGate>(circuitName, *this); }
        };

    public:
        ColumnTristateGate(std::string_view name, const ColumnTristateGate::Arguments& arguments) : WrapModule{name}
        { this->loadFromLibrary(); }

        virtual ~ColumnTristateGate() noexcept override = default;

    private:
        virtual void createPorts() override 
        {
            this->addPortNet(this->BITLINE_NAME, PortType::InOut );
            this->addPortNet(this->BITLINE_BAR_NAME, PortType::InOut );
            this->addPortNet(this->OUTPUT_BITLINE_NAME, PortType::InOut );
            this->addPortNet(this->OUTPUT_BITLINE_BAR_NAME, PortType::InOut );
            this->addPortNet(this->SELECTION_NAME, PortType::Input );
            this->addPortNet(tech->vdd(), PortType::InOut );
            this->addPortNet(tech->gnd(), PortType::InOut );
        }
    };

}