#include "./circuitvisitor.hh"
#include "circuit/geometry/coord.hh"
#include "global/tech/tech.hh"
#include "parse/gdsii/gdsiilayout.hh"
#include "parse/gdsii/gdsiiprimitives.hh"

#include <circuit/base/circuit.hh>
#include <circuit/base/module.hh>
#include <circuit/base/instance.hh>
#include <circuit/component/mos.hh>
#include <circuit/component/resistance.hh>
#include <circuit/component/capacitance.hh>

#include <circuit/geometry/geometry.hh>
#include <circuit/geometry/label.hh>
#include <circuit/geometry/rectangle.hh>
#include <circuit/geometry/path.hh>
#include <circuit/geometry/pin.hh>
#include <circuit/geometry/via.hh>

#include <global/config/config.hh>
#include <memory>
#include <optional>
#include <utility/debug/debug.hh>

namespace xtaro::dump 
{

    auto GDSIICircuitVisitor::layout(circuit::Circuit* cir) -> std::unique_ptr<parse::GDSIILayout>&
    {
        return this->_layouts.at(cir);
    }

    auto GDSIICircuitVisitor::visitModule(circuit::Module* mod) -> void
    {
        if (this->_layouts.contains(mod)) {
            return;
        }

        auto layout = std::unique_ptr<parse::GDSIILayout>();
        if (mod->hasGDSIIFilePath()) 
        {
            layout = parse::GDSIILayout::loadFromFile(mod->gdsiiFilePath());
        }
        else
        {
            layout = std::make_unique<parse::GDSIILayout>(std::string{mod->name()});
            layout->metersPerDBunit(tech->gdsMetersPerDBunit().originValue());

            for (auto& inst : mod->instances())
                this->writeInstance(layout, *inst);

            this->writeCircuit(layout, mod);
        }

        this->_layouts.emplace(mod, std::move(layout));
    }

    auto GDSIICircuitVisitor::visitNotModule(circuit::Circuit* cir) -> void
    {
        if (this->_layouts.contains(cir)) {
            return;
        }
        auto layout = std::make_unique<parse::GDSIILayout>(std::string{cir->name()});
        this->writeCircuit(layout, cir);
        this->_layouts.emplace(cir, std::move(layout));
    }

    auto GDSIICircuitVisitor::visitMOS(circuit::MOS* mos) -> void
    {
        this->visitNotModule(mos);
    }

    auto GDSIICircuitVisitor::visitResistance(circuit::Resistance* res) -> void
    {
        this->visitNotModule(res);
    }

    auto GDSIICircuitVisitor::visitCapacitance(circuit::Capacitance* cap) -> void
    {
        this->visitNotModule(cap);
    }

    auto GDSIICircuitVisitor::writeCircuit(std::unique_ptr<parse::GDSIILayout>& layout, circuit::Circuit* cir) -> void
    {
        for (auto& label : cir->labels())
            this->writeGeometry(layout, label);

        for (auto& rect : cir->rectangles())
            this->writeGeometry(layout, rect);

        for (auto& path : cir->paths())
            this->writeGeometry(layout, path);

        for (auto& via : cir->vias()) 
            this->writeGeometry(layout, via);

        for (auto& [name, pins] : cir->pins()) 
        {
            for (auto& pin : pins)
                this->writeGeometry(layout, pin);
        }
    }

    auto GDSIICircuitVisitor::writeGeometry(std::unique_ptr<parse::GDSIILayout>& layout, const circuit::Label& label) -> void
    {
        layout->addText(label.layerMask(), label.offset().toGDSIIMicroCoord(), std::string{label.text()});
    }

    auto GDSIICircuitVisitor::writeGeometry(std::unique_ptr<parse::GDSIILayout>& layout, const circuit::Pin& pin) -> void
    {
        this->writeGeometry(layout, static_cast<const circuit::Rectangle&>(pin));
        layout->addText(
            pin.layerMask(),
            pin.leftdown().toGDSIIMicroCoord(),
            std::string{pin.name()}
        );
    }

    auto GDSIICircuitVisitor::writeGeometry(std::unique_ptr<parse::GDSIILayout>& layout, const circuit::Rectangle& rect) -> void
    {
        layout->addRectangle(
            rect.layerMask(),
            rect.leftdown().toGDSIIMicroCoord(),
            rect.width().microValue(),
            rect.height().microValue()
        );
    }

    auto GDSIICircuitVisitor::writeGeometry(std::unique_ptr<parse::GDSIILayout>& layout, const circuit::Path& path) -> void
    {
        auto microcoords = std::vector<parse::GDSIIMicroCoord>{};
        for (auto& coord : path.coords()) 
            microcoords.emplace_back(coord.toGDSIIMicroCoord());

        layout->addPath(path.layerMask(), microcoords, path.width().microValue());
    }

    auto GDSIICircuitVisitor::writeGeometry(std::unique_ptr<parse::GDSIILayout>& layout, const circuit::Via& via) -> void
    {
        for (auto& rect : via.rectangles()) 
            this->writeGeometry(layout, rect);
    }

    auto mirrorTypeToGDSIIMirror(MirrorType mirror) -> std::optional<parse::GDSIIMirror> 
    {
        switch (mirror) {
            case MirrorType::None:     return std::nullopt;
            case MirrorType::MirrorX:  return {parse::GDSIIMirror::MirrorX};
            case MirrorType::MirrorY:  return {parse::GDSIIMirror::MirrorY};
            case MirrorType::MirrorXY: return {parse::GDSIIMirror::MirrorXY};
        }
        return std::nullopt;
    }

    auto GDSIICircuitVisitor::writeInstance(std::unique_ptr<parse::GDSIILayout>& layout, const circuit::Instance& instance) -> void
    {
        auto circuit = instance.circuit();
        circuit->accept(*this);
        auto& subLayout = this->_layouts.at(circuit);

        layout->addInstance(
            subLayout,
            instance.offset().toGDSIIMicroCoord(),
            mirrorTypeToGDSIIMirror(instance.mirror()),
            instance.angle()
        );
    }

}