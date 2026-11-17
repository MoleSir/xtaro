#include "./instvisitor.hh"
#include "circuit/base/visitor.hh"

#include <circuit/base/circuit.hh>

#include <circuit/base/module.hh>
#include <circuit/base/instance.hh>
#include <circuit/component/mos.hh>
#include <circuit/component/resistance.hh>
#include <circuit/component/capacitance.hh>

#include <utility/debug/debug.hh>
#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <utility/misc/file.hh>

#include <fstream>
#include <string_view>

namespace xtaro::dump
{
    
    SpiceInstanceVisitor::SpiceInstanceVisitor(circuit::Instance const& i, std::ofstream& f) :
        instance{i}, file{f} {}

    auto SpiceInstanceVisitor::writeConnectedPins() -> void {
        instance.checkPortsFullyConnected();

        this->file << '\n';
        int currentLineNetsSize {0};
        auto& ports = instance.ports();
        for (std::size_t portIdx = 0; portIdx < ports.size(); ++portIdx)
        {
            if (currentLineNetsSize == 0) this->file << '+';
            this->file << ' ' << ports[portIdx].net()->name();
            if (++currentLineNetsSize == 15 && (portIdx != ports.size() - 1) )
            {
                this->file << '\n';
                currentLineNetsSize = 0;
            }
        }
        this->file << '\n' << '+';
    }

    auto SpiceInstanceVisitor::visitModule(circuit::Module* module) -> void  
    {
        this->file << 'X' << this->instance.name();
        this->writeConnectedPins();
        this->file << ' ' << module->name() << '\n';
    }

    auto SpiceInstanceVisitor::visitMOS(circuit::MOS* mos) -> void 
    {
        this->file << 'M' << this->instance.name();

        this->writeConnectedPins();
        
        if (mos->mosType() == circuit::MOSType::NMOS) 
            this->file << tech->nmosName();
        else
            this->file << tech->pmosName();
        
        this->file << std::format(" w={:.2f} l={:.2f}\n", mos->mosWidth(), mos->mosLength());
    }

    auto SpiceInstanceVisitor::visitResistance(circuit::Resistance* r) -> void 
    {
        this->file << 'R' << this->instance.name();
        this->writeConnectedPins();
        this->file << std::format(" {}\n", r->value());
    }

    auto SpiceInstanceVisitor::visitCapacitance(circuit::Capacitance* c) -> void 
    {
        this->file << 'C' << this->instance.name();
        this->writeConnectedPins();
        this->file << std::format(" {}\n", c->value());
    }

}