#pragma once

#include "./circuitvisitor.hh"
#include <parse/gdsii/gdsiilayout.hh>
#include <circuit/base/visitor.hh>

#include <filesystem>

namespace xtaro::circuit {
    class Circuit;
}

namespace xtaro::dump 
{

    class GDSII
    {
    public:
        GDSII(circuit::Circuit* circuit, std::filesystem::path outputPath);
        void write();

    private:
        circuit::Circuit* _circuit;
        std::filesystem::path _outputPath;
        GDSIICircuitVisitor _visitor;
    };

}