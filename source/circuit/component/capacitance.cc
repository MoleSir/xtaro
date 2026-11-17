#include "./capacitance.hh"
#include "../base/circuit.hh"
#include "../base/visitor.hh"

#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <utility/misc/string.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    std::string Capacitance::Arguments::toString() const
    {
        return std::format("v{}", this->value);
    }

    auto Capacitance::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<Capacitance>(circuitName, *this);
    }

    Capacitance::Capacitance(std::string_view name, const Capacitance::Arguments& arguments):
        Circuit{name},
        _value{arguments.value}
    {
        this->createPorts();
    }

    void Capacitance::createPorts()
    {
        this->addPort("A", PortType::InOut);
        this->addPort("B", PortType::InOut);
    }

    auto Capacitance::createNetlist() -> void
    {

    }

    auto Capacitance::createLayout()  -> void
    {
        debug::unimplement("`createLayout` for capacitance");
    }

    auto Capacitance::accept(CircuitVisitor& visitor) -> void
    {
        return visitor.visitCapacitance(this);
    }

}