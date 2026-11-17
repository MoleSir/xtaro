#include "./circuitvisitor.hh"
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

namespace xtaro::dump
{

    SpiceCircuitVisitor::SpiceCircuitVisitor(const std::filesystem::path& outputPath) :
        _file{outputPath},
        _visited{}
    {
        if (!this->_file.is_open())
            debug::exceptionfIn("Write spice", "Open file '{}' failed!", outputPath.c_str());
    }

    auto SpiceCircuitVisitor::visitModule(circuit::Module* module) -> void
    {
        this->_visited.emplace(module); 

        if (!module->hasSpiceFilePath())
        {
            // .SUBCKT and2 A B Z vdd gnd  
            // Output sub circuits in 'this'
            for (auto circuit : module->subCircuits())
            {
                if (this->_visited.contains(circuit))
                    continue;
                circuit->accept(*this);
            }

            // Write .SUBCKT line
            this->_file << "\n.SUBCKT " << module->name() << '\n';
            // Write ports, each line has 15 ports
            int currentLineNetsSize {0};
            const auto& ports = module->ports();
            for (std::size_t portIdx = 0; portIdx < ports.size(); ++portIdx)
            {
                if (currentLineNetsSize == 0) 
                    this->_file << '+';
                this->_file << ' ' << ports[portIdx].name();
                if (++currentLineNetsSize == 15 && (portIdx != ports.size() - 1) )
                {
                    this->_file << '\n';
                    currentLineNetsSize = 0;
                }
            }

            this->_file << '\n';

            // Write Instances
            for (auto& instance : module->instances())
                this->visitInstance(*instance);

            // Write connected Nets
            int netIdx {0};
            for (const auto& netsPair : module->connectedNets())
            {
                this->_file << "Rconnect" << netIdx++ << ' '
                     << netsPair.first->name() << ' '
                     << netsPair.second->name() << ' '
                     << 0.001 <<'\n';
            }

            // Write .END
            this->_file << ".ENDS " << module->name() << '\n';
        }
        else
        {
            this->_file << '\n' << util::readFile(module->spiceFilePath()) << '\n';
        }
    }

    auto SpiceCircuitVisitor::visitMOS(circuit::MOS* mos) -> void
    {
        this->_visited.emplace(mos);
    }

    auto SpiceCircuitVisitor::visitResistance(circuit::Resistance* r) -> void
    {
        this->_visited.emplace(r);
    }

    auto SpiceCircuitVisitor::visitCapacitance(circuit::Capacitance* c) -> void
    {
        this->_visited.emplace(c);
    }

    auto SpiceCircuitVisitor::visitInstance(circuit::Instance const& instance) -> void
    {
        auto visitor = SpiceInstanceVisitor{instance, this->_file};
        instance.circuit()->accept(visitor);
    }

}