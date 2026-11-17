#pragma once

#include <string_view>

namespace xtaro::circuit
{
    
    class Net;
    class Circuit;
    class Instance;

    enum class PortType
    {
        Input, Output, InOut,
    };

    class Port
    {
    public:
        Port(std::string_view name, PortType type);

    public:
        auto name() const noexcept -> std::string_view
        { return this->_name; }

        auto type() const noexcept -> PortType
        { return this->_type; }

        auto net() const noexcept -> Net*
        { return this->_net; }
        
    public:
        void setNet(Net* net) noexcept
        { this->_net = net; } 

    private:
        std::string_view _name;
        PortType _type;
        Net* _net;
    };


}