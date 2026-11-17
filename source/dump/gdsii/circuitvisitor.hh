#pragma once

#include <parse/gdsii/gdsiilayout.hh>
#include <circuit/base/visitor.hh>

#include <memory>
#include <unordered_map>

namespace xtaro::circuit {
    class Circuit;
    class Module;
    class Resistance;
    class Capacitance;
    class MOS;
    class Instance;

    class Label;
    class Path;
    class Rectangle;
    class Pin;
    class Via;
}

namespace xtaro::dump 
{

    struct GDSIICircuitVisitor : public circuit::CircuitVisitor
    {
    public:
        GDSIICircuitVisitor() = default;

    private:
        virtual auto visitModule(circuit::Module*) -> void override;
        virtual auto visitMOS(circuit::MOS*) -> void override;
        virtual auto visitResistance(circuit::Resistance*) -> void override;
        virtual auto visitCapacitance(circuit::Capacitance*) -> void override;
        
        auto visitNotModule(circuit::Circuit*) -> void;

    private:
        auto writeCircuit(std::unique_ptr<parse::GDSIILayout>& layout, circuit::Circuit*) -> void;
        auto writeGeometry(std::unique_ptr<parse::GDSIILayout>& layout, const circuit::Label& label) -> void;
        auto writeGeometry(std::unique_ptr<parse::GDSIILayout>& layout, const circuit::Pin& label) -> void;
        auto writeGeometry(std::unique_ptr<parse::GDSIILayout>& layout, const circuit::Rectangle& label) -> void;
        auto writeGeometry(std::unique_ptr<parse::GDSIILayout>& layout, const circuit::Path& label) -> void;
        auto writeGeometry(std::unique_ptr<parse::GDSIILayout>& layout, const circuit::Via& via) -> void;
        auto writeInstance(std::unique_ptr<parse::GDSIILayout>& layout, const circuit::Instance& instance) -> void;

    public:
        auto layout(circuit::Circuit*) -> std::unique_ptr<parse::GDSIILayout>&;

    private:
        std::unordered_map<circuit::Circuit*, std::unique_ptr<parse::GDSIILayout>> _layouts{};
    };

}