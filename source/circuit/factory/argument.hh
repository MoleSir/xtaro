#pragma once

#include <string_view>
#include <utility/debug/debug.hh>

#include <exception>
#include <parse/serde/mod.hh>

#include <any>

namespace xtaro::circuit {

    class Argument
    {
    public:
        Argument() :
            _value{}
        {
        }

        template <typename Value>
        Argument(Value&& value) : 
            _value{value}
        {
        }

        static auto buildByString(std::string_view str) -> Argument;
        
        template <typename Value>
        auto get() const -> Value 
        {
            try
            {
                return std::any_cast<Value>(this->_value);
            }
            catch (const std::exception& err) 
            {
                debug::exceptionf("Unexcept argument type of circuit argument, '{}'", err.what());
            }
        }

    public:
        auto value() -> std::any& { return this->_value; }

    private:
        std::any _value;
    };

}

template <typename Deserializer>
struct xtaro::parse::Deserialize<Deserializer, xtaro::circuit::Argument>
{
    static void from(const Deserializer& dsr, xtaro::circuit::Argument& value)
    {
        // Try get value that Deserializer can get
        xtaro::parse::Deserialize<Deserializer, std::any>::from(dsr, value.value());
        try
        {
            // Use `buildByString` to get a Argument
            auto str = std::any_cast<std::string_view>(value.value());
            value = circuit::Argument::buildByString(str);
        }
        catch (const std::exception&) {}
    }
};
