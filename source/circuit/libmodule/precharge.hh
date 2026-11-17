#pragma once

#include "./wrapmodule.hh"
#include <string_view>
#include <tech/tech.hh>

namespace xtaro::circuit
{

    /*
        - Ports: bl br wl vdd gnd;
    */
    class Precharge : public WrapModule
    {
    public:
        static constexpr std::string_view TYPE = "wrap_precharge";
        static constexpr std::string_view BITLINE_NAME = "bl";
        static constexpr std::string_view BITLINE_BAR_NAME = "br";
        static constexpr std::string_view ENABLE_NAME = "en_bar";

        /*
            Construction: ()
        */
        struct Arguments : public Circuit::Arguments 
        {
            Arguments() noexcept : Circuit::Arguments {Precharge::TYPE} {}
            virtual ~Arguments() noexcept = default;
            
            virtual auto toString() const -> std::string override 
            { return ""; }

            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override
            { return std::make_unique<Precharge>(circuitName, *this); }
        };

    public:
        Precharge(std::string_view name, const Precharge::Arguments& arguments) : WrapModule{name}
        { this->loadFromLibrary(); }

        virtual ~Precharge() noexcept override = default;

    private:
        virtual void createPorts() override
        {
            this->addPortNet(this->BITLINE_NAME, PortType::InOut );
            this->addPortNet(this->BITLINE_BAR_NAME, PortType::InOut );
            this->addPortNet(this->ENABLE_NAME, PortType::Input );
            this->addPortNet(tech->vdd(), PortType::InOut );
        }
    };

}