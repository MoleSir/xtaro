#pragma once

#include <global/number/number.hh>
#include <optional>
#include <string>
#include <fstream>

namespace xtaro::characterize
{

    class Measurement
    {
    public:
        Measurement(std::string name);
        virtual ~Measurement() noexcept {}

        virtual void writeCommand(std::ofstream& file) const = 0;
        bool getMeasureResult(const std::string& context);

    public:
        const std::string& name() const noexcept
        { return this->_name; }

        Number result() const noexcept
        { return this->_result.value(); }

    protected:
        std::string _name;
        std::optional<Number> _result;
    };
    
}