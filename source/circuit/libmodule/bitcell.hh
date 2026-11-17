#pragma once

#include "./wrapmodule.hh"
#include <string_view>
#include <tech/tech.hh>

namespace xtaro::circuit
{

    /*
        - Ports: bl br wl vdd gnd;
    */
    class Bitcell : public WrapModule
    {
    public:
        static constexpr std::string_view TYPE = "wrap_bitcell";
        static constexpr std::string_view BITLINE_NAME = "bl";
        static constexpr std::string_view BITLINE_BAR_NAME = "br";
        static constexpr std::string_view WORDLINE_NAME = "wl";

        /*
            Construction: ()
        */
        struct Arguments : public Circuit::Arguments 
        {
            Arguments() noexcept : Circuit::Arguments {Bitcell::TYPE} {}
            virtual ~Arguments() noexcept = default;
            
            virtual auto toString() const -> std::string override 
            { return ""; }

            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override
            { return std::make_unique<Bitcell>(circuitName, *this); }
        };

    public:
        Bitcell(std::string_view name, const Bitcell::Arguments& arguments) : WrapModule{name} 
        { this->loadFromLibrary(); };
        
        virtual ~Bitcell() noexcept override = default;

    private:
        virtual void createPorts() override 
        {
            this->addPortNet(this->BITLINE_NAME, PortType::InOut );
            this->addPortNet(this->BITLINE_BAR_NAME, PortType::InOut );
            this->addPortNet(this->WORDLINE_NAME, PortType::Input );
            this->addPortNet(tech->vdd(), PortType::InOut );
            this->addPortNet(tech->gnd(), PortType::InOut );
        }
    };

}