#pragma once

#include "../base/circuit.hh"
#include "./argument.hh"

#include <optional>
#include <string>
#include <format>
#include <unordered_map>
#include <unordered_set>
#include <string_view>
#include <memory>

namespace xtaro::circuit
{

    class CircuitFactory
    {
    public:
        CircuitFactory() = default;
        ~CircuitFactory() noexcept;

    public:
        void reset() noexcept;

        auto circuit(const Circuit::Arguments& arguments) -> Circuit*;
        auto circuit(std::string_view circuitType, const std::unordered_map<std::string, Argument>& arguments) -> Circuit*;

        auto string(std::string str) -> std::string_view;
        template<typename... Args>
        auto stringf(std::format_string<Args...> fmt, Args&&... args) -> std::string_view {
            auto str = std::format(std::move(fmt), std::forward<Args>(args)...);
            return this->string(std::move(str));
        }

    private:        
        auto getCircuitName(const Circuit::Arguments& arguments) -> std::string_view;
    
        auto findCircuit(std::string_view circuitName) const -> std::optional<Circuit*>;
        
        auto createNewCircuit(
            std::string_view circuitName,
            const Circuit::Arguments& arguments
        ) const -> std::unique_ptr<Circuit>;

        auto createCircuitArguments(
            std::string_view circuitType,
            const std::unordered_map<std::string, Argument>& arguments
        ) -> std::unique_ptr<Circuit::Arguments>;

        void collectCircuit(
            std::string_view circuitName,
            std::unique_ptr<Circuit> circuit
        );

    private:
        std::unordered_set<std::string> _stringPool {};
        std::unordered_map<std::string_view, std::unique_ptr<Circuit>> _circuitPool {};
    };
    
}