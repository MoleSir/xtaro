#pragma once

#include "circuit/base/circuit.hh"
#include <circuit/base/visitor.hh>
#include <fstream>
#include <unordered_set>

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

    class SpiceCircuitVisitor : public circuit::CircuitVisitor
    {
    public:
        SpiceCircuitVisitor(const std::filesystem::path& outputPath);

    private:
        virtual auto visitModule(circuit::Module*) -> void override;
        virtual auto visitMOS(circuit::MOS*) -> void override;
        virtual auto visitResistance(circuit::Resistance*) -> void override;
        virtual auto visitCapacitance(circuit::Capacitance*) -> void override;

    private:
        auto visitInstance(circuit::Instance const& instance) -> void;

    private:
        std::ofstream _file;
        std::unordered_set<const circuit::Circuit*> _visited;
    };

}