#pragma once

#include "./wrapmodule.hh"
#include <string_view>

namespace xtaro::circuit
{

    /*
        - Ports: bl br dout en vdd gnd
    */
    class SenseAmplifier : public WrapModule
    {
    public:
        static constexpr std::string_view TYPE = "wrap_sense_amp";
        static constexpr std::string_view BITLINE_NAME = "bl";
        static constexpr std::string_view BITLINE_BAR_NAME = "br";
        static constexpr std::string_view DATA_OUTPUT_NAME = "dout";
        static constexpr std::string_view ENABLE_NAME = "en";

        /*
            Construction: ()
        */
        struct Arguments : public Circuit::Arguments 
        {
            Arguments() noexcept : Circuit::Arguments {SenseAmplifier::TYPE} {}
            virtual ~Arguments() noexcept = default;
            
            virtual auto toString() const -> std::string override 
            { return ""; }

            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override
            { return std::make_unique<SenseAmplifier>(circuitName, *this); }
        };

    public:
        SenseAmplifier(std::string_view name, const SenseAmplifier::Arguments& arguments) : WrapModule{name}
        { this->loadFromLibrary(); }

        virtual ~SenseAmplifier() noexcept override = default;

    private:
        virtual void createPorts() override 
        {
            // bl br dout en vdd gnd
            this->addPortNet(this->BITLINE_NAME, PortType::InOut);
            this->addPortNet(this->BITLINE_BAR_NAME, PortType::InOut);
            this->addPortNet(this->DATA_OUTPUT_NAME, PortType::Output);
            this->addPortNet(this->ENABLE_NAME, PortType::Input);
            this->addPortNet(tech->vdd(), PortType::InOut);
            this->addPortNet(tech->gnd(), PortType::InOut);
        }
    };

}