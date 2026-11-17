#pragma once

#include "base/circuit.hh"
#include "factory/argument.hh"

#include <format>
#include <string>
#include <unordered_map>

namespace xtaro::circuit
{

    /*
        Initial the circuit facroty
    */
    auto initial() -> void;

    /*
        Clear all string object and circuit object
        !!! This will make all pointer invalid!
    */
    auto reset() -> void;

    /*
        Alloc string from factory
        MARK: Maybe use `Circuit::Str` instead of `std::string_view`
    */
    auto string(std::string str) -> std::string_view;
    
    /*
        Alloc circuit object
    */
    auto circuit(const Circuit::Arguments& arguments) -> Circuit*;

    /*
        Alloc circuit object
    */
    auto circuit(std::string_view circuitType, const std::unordered_map<std::string, Argument>& arguments) -> Circuit*;

    template<typename... Args>
    auto stringf(std::format_string<Args...> fmt, Args&&... args) -> std::string_view {
        auto str = std::format(std::move(fmt), std::forward<Args>(args)...);
        return string(str);
    }

}
