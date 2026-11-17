#pragma once

#include "./circuitvisitor.hh"
#include "circuit/base/circuit.hh"
#include <circuit/base/visitor.hh>

namespace xtaro::circuit 
{
    class Circuit;
}

namespace xtaro::dump 
{

    class Spice 
    {
    public:
        Spice(circuit::Circuit* circuit, std::filesystem::path outputPath);
        auto write() -> void;

    private:
        circuit::Circuit* _circuit;
        std::filesystem::path _outputPath;
        SpiceCircuitVisitor _visitor;        
    };

}