#include "./lef.hh"
#include "circuit/base/port.hh"
#include "global/tech/layer.hh"
#include "utility/debug/debug.hh"
#include <circuit/base/circuit.hh>
#include <global/config/config.hh>
#include <tech/tech.hh>
#include <format>

namespace xtaro::dump
{

    Lef::Lef(circuit::Circuit* circuit, std::filesystem::path outputPath) :
        _circuit{circuit},
        _leffile{}
    {
        this->_leffile.open(outputPath);
    }

    void Lef::write()
    {
        this->writeHeader(0);
        for (const auto& port : this->_circuit->ports()) 
            this->writePin(1, port.name());

        this->writeObstructions(1);
        this->writeFooter(0);
    }

    void Lef::writeHeader(int indent)
    {
        // Header of LEF file
        this->writeLine(indent, "VERSION 5.4 ;");
        this->writeLine(indent, "NAMESCASESENSITIVE ON ;");
        this->writeLine(indent, "BUSBITCHARS \"[]\" ;");
        this->writeLine(indent, "BUSBITCHARS \"[]\" ;");
        this->writeLine(indent, "DIVIDERCHAR \"/\" ;");

        this->writeLine(indent, "UNITS");
        // MARK
        this->writeLinef(indent + 1, "DATABASE MICRONS {} ;", Lef::UNITS);
        this->writeLine(indent, "END UNITS");

        this->writeLinef(indent, "MACRO {}", this->_circuit->name());
        this->writeLine(indent + 1, "CLASS BLOCK ;");
        // MARK
        this->writeLinef(indent + 1, "SIZE {} BY {} ;", this->_circuit->width(), this->_circuit->height());
        this->writeLinef(indent + 1, "SYMMETRY X Y R90 ;");
    }

    void Lef::writePin(int indent, std::string_view name)
    {
        // MARK: VDD/GND
        if (name == tech->vdd() || name == tech->gnd())
            return;

        auto portType = this->_circuit->portType(name);
        
        this->writeLinef(indent, "PIN {}", name);
        this->writeLinef(indent + 1, "DIRECTION {} ;", Lef::portTypeToString(portType));
        this->writeLine(indent + 1, "PORT");
        for (auto pin : this->_circuit->getPins(name)) 
        {
            this->writeLinef(indent + 2, "LAYER {} ;", pin.layer());
            this->writeBlockage(indent + 2, pin.getBlockage(pin.layer()).value());
        }
        this->writeLine(indent + 1, "END");
        this->writeLinef(indent, "END {}", name);
    }

    void Lef::writeObstructions(int indent)
    {
        this->writeLine(indent, "OBS");
        for (auto layer : {Layer::Metal1, Layer::Metal2, Layer::Metal3, Layer::Metal4}) 
        {
            this->writeLinef(indent, "LAYER {} ;", layer);
            auto blockages = this->_circuit->getBlockages(layer);
            for (const auto& blockage : blockages) 
                this->writeBlockage(indent + 1, blockage);
            this->writeLine(indent, "END");
        }
    }

    void Lef::writeFooter(int indent)
    {
        this->writeLinef(indent + 1, "END {}", this->_circuit->name());
        this->writeLine(indent, "END LIBRARY");
    }

    void Lef::writeBlockage(int indent, const circuit::Blockage& blockage)
    {
        const auto& ld = blockage.leftdown;
        const auto& ru = blockage.rightup;
        this->writeLinef(indent,
             "RECT {} {} {} {}", 
             ld.x.toMicro().value(), 
             ld.y.toMicro().value(),
             ru.x.toMicro().value(),
             ru.y.toMicro().value());
    }

    void Lef::writeLine(int indent, std::string_view content)
    {
        while (indent--) this->_leffile << "    ";
        this->_leffile << content << '\n';
    }

    void Lef::writeLine(int indent, char ch)
    {
        while (indent--) this->_leffile << "    ";
        this->_leffile << ch << '\n';
    }

    void Lef::writeEnter()
    {
        this->_leffile << '\n';
    }

    auto Lef::portTypeToString(circuit::PortType type) -> std::string_view 
    {
        switch (type) {
            case circuit::PortType::Input: return "INPUT";
            case circuit::PortType::Output: return "OUTPUT";
            case circuit::PortType::InOut: return "INOUT";
        }
        debug::unreachable();
    }
}