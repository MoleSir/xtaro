#include "./factory/circuitfactory.hh"
#include "circuit/factory/argument.hh"

#include <format>
#include <utility/debug/debug.hh>
#include <string>
#include <unordered_map>

namespace xtaro::circuit 
{

    static std::unique_ptr<CircuitFactory> factory;

    auto initial() -> void 
    {
        debug::infof("Initial circuit factory");
        if (factory == nullptr)
            factory = std::make_unique<CircuitFactory>();
    }

    auto reset() -> void 
    {
        debug::infof("Reset circuit factory");
        if (factory != nullptr)
            factory->reset();
    }

    auto string(std::string str) -> std::string_view 
    {
        return factory->string(std::move(str));
    }
    
    auto circuit(const Circuit::Arguments& arguments) -> Circuit*
    {
        return factory->circuit(arguments);
    }

    auto circuit(std::string_view circuitType, const std::unordered_map<std::string, Argument>& arguments) -> Circuit*
    {
        return factory->circuit(circuitType, arguments);
    }

}