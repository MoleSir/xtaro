#include "circuit/base/circuit.hh"
#include <circuit/module/sram.hh>
#include <global/env/pvt.hh>
#include <global/env/slewload.hh>
#include <global/env/slews.hh>
#include <optional>

namespace xtaro::dump
{

    void writeSpice(circuit::Circuit* circuit, std::optional<std::filesystem::path> outputPath = std::nullopt);

    void writeGDSII(circuit::Circuit* circuit, std::optional<std::filesystem::path> outputPath = std::nullopt);

    void writeLef(circuit::Circuit* circuit, std::optional<std::filesystem::path> outputPath = std::nullopt);

    void writeVerilog(circuit::SRAM* sram, std::optional<std::filesystem::path> outputFolder = std::nullopt);
    
    void writeLiberty(circuit::SRAM* sram, PVT pvt, std::optional<std::filesystem::path> outputFolder = std::nullopt);

    void writeLibertys(circuit::SRAM* sram, const std::vector<PVT>& pvts);
    
}