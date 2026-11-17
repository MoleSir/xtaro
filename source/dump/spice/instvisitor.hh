#pragma once

#include <circuit/base/visitor.hh>
#include <fstream>

namespace xtaro::circuit {
    class Circuit;
    class Module;
    class Resistance;
    class Capacitance;
    class MOS;
    
    class Instance;
}

namespace xtaro::dump
{

    class SpiceInstanceVisitor : public circuit::CircuitVisitor
    {
    public:
        SpiceInstanceVisitor(circuit::Instance const& i, std::ofstream& f);

    public:
        virtual auto visitModule(circuit::Module* module) -> void override;
        virtual auto visitMOS(circuit::MOS* mos) -> void override;
        virtual auto visitResistance(circuit::Resistance* r) -> void override;
        virtual auto visitCapacitance(circuit::Capacitance* c) -> void override;

    private:
        auto writeConnectedPins() -> void;

    private:
        circuit::Instance const& instance;
        std::ofstream& file;
    };
}