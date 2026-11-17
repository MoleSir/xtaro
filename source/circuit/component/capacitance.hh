#pragma once

#include "../base/circuit.hh"
#include <global/number/number.hh>

namespace xtaro::circuit
{

    /*
        - Ports: A B
    */
    class Capacitance : public Circuit
    {
    public:
        static constexpr std::string_view TYPE = "capacitance";

        /*
            - Construction: (Number value);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(Number v) :
                Circuit::Arguments {Capacitance::TYPE},
                value{v} {}
                
            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            Number value;
        };

    public:
        Capacitance(std::string_view name, const Capacitance::Arguments& arguments);
        virtual ~Capacitance() noexcept override = default;

    public:
        virtual auto accept(CircuitVisitor& visitor) -> void override;

    private:
        virtual auto createPorts() -> void override;
        virtual auto createNetlist() -> void override;
        virtual auto createLayout()  -> void override;

    public:
        auto value() const -> Number { return this->_value; }
    
    private:
        Number _value;
    };

}