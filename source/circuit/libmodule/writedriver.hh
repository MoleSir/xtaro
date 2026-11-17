#pragma once

#include "./wrapmodule.hh"
#include <string_view>
#include <tech/tech.hh>

namespace xtaro::circuit
{

    /*
        - Ports: bl br wl vdd gnd;
    */
    class WriteDriver : public WrapModule
    {
    public:
        static constexpr std::string_view TYPE = "wrap_write_driver";
        static constexpr std::string_view BITLINE_NAME = "bl";
        static constexpr std::string_view BITLINE_BAR_NAME = "br";
        static constexpr std::string_view DATA_INPUT_NAME = "din";
        static constexpr std::string_view ENABLE_NAME = "en";

        /*
            Construction: ()
        */
        struct Arguments : public Circuit::Arguments 
        {
            Arguments() noexcept : Circuit::Arguments {WriteDriver::TYPE} {}
            virtual ~Arguments() noexcept = default;
            
            virtual auto toString() const -> std::string override 
            { return ""; }

            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override
            { return std::make_unique<WriteDriver>(circuitName, *this); }
        };

    public:
        WriteDriver(std::string_view name, const WriteDriver::Arguments& arguments) : WrapModule{name}
        { this->loadFromLibrary(); }

        virtual ~WriteDriver() noexcept override = default;

    private:
        virtual void createPorts() override 
        {
            this->addPortNet(this->DATA_INPUT_NAME, PortType::Input );
            this->addPortNet(this->BITLINE_NAME, PortType::Output );
            this->addPortNet(this->BITLINE_BAR_NAME, PortType::Output );
            this->addPortNet(this->ENABLE_NAME, PortType::Input );
            this->addPortNet(tech->vdd(), PortType::InOut );
            this->addPortNet(tech->gnd(), PortType::InOut );
        }
    };

}