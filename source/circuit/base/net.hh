#pragma once

#include <string_view>

namespace xtaro::circuit
{
    
    class Port;

    class Net
    {
    public:
        Net(std::string_view name);
        auto name() const noexcept -> std::string_view
        { return this->_name; }
                
    private:
        std::string_view _name;
    };

}