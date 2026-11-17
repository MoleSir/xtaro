#include "./resistance.hh"
#include "../base/circuit.hh"
#include "../base/visitor.hh"

#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <utility/misc/string.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    std::string Resistance::Arguments::toString() const
    {
        return std::format("v{}", this->value);
    }

    auto Resistance::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<Resistance>(circuitName, *this);
    }

    Resistance::Resistance(std::string_view name, const Resistance::Arguments& arguments):
        Circuit{name},
        _value{arguments.value}
    {
        this->createPorts();
    }

    void Resistance::createPorts()
    {
        this->addPort("A", PortType::InOut);
        this->addPort("B", PortType::InOut);
    }

    auto Resistance::createNetlist() -> void
    {

    }

    auto Resistance::createLayout()  -> void
    {
        debug::unimplement("`createLayout` for resistance");
    }

    auto Resistance::accept(CircuitVisitor& visitor) -> void
    {
        return visitor.visitResistance(this);
    }
    
}