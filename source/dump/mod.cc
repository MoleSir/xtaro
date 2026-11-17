#include "./mod.hh"


#include "./lef/lef.hh"
#include "./verilog/verilog.hh"
#include "./liberty/liberty.hh"
#include "./spice/spice.hh"
#include "./gdsii/gdsii.hh"

#include <characterize/mod.hh>
#include <circuit/module/sram.hh>
#include <circuit/base/circuit.hh>

#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <global/number/number.hh>
#include <utility/debug/debug.hh>


#include <utility>
#include <vector>

namespace xtaro::dump
{

    void writeSpice(circuit::Circuit* circuit, std::optional<std::filesystem::path> outputPathOpt)
    try
    {
        debug::infof("Write spice file for '{}'", circuit->name());

        auto outputPath = 
            outputPathOpt.has_value() ? 
            std::move(*outputPathOpt) : 
            config->spiceFolderPath / std::format("{}.sp", circuit->name());

        auto spice = Spice{circuit, std::move(outputPath)};
        spice.write();
    }
    catch (const std::exception& err)
    {
        debug::exceptionIn("Write spice", err.what());
    }

    void writeGDSII(circuit::Circuit* circuit, std::optional<std::filesystem::path> outputPathOpt)
    try
    {
        debug::infof("Write gdsii file for '{}'", circuit->name());

        auto outputPath = 
            outputPathOpt.has_value() ? 
            std::move(*outputPathOpt) : 
            config->gdsFolderPath / std::format("{}.gds", circuit->name());

        auto gdsii = GDSII{circuit, std::move(outputPath)};
        gdsii.write();
    }
    catch (const std::exception& err)
    {
        debug::exceptionIn("Write gdsii", err.what());
    }

    void writeLef(circuit::Circuit* circuit, std::optional<std::filesystem::path> outputPathOpt)
    try
    {
        debug::infof("Write lef file for '{}'", circuit->name());

        auto outputPath = 
            outputPathOpt.has_value() ? 
            std::move(*outputPathOpt) : 
            config->lefFolderPath / std::format("{}.lef", circuit->name());

        auto lef = Lef {circuit, std::move(outputPath)};
        lef.write();
    }
    catch (const std::exception& err)
    {
        debug::exceptionIn("Write lef", err.what());
    }

    void writeVerilog(circuit::SRAM* sram, std::optional<std::filesystem::path> outputFolderOpt)
    try
    {
        debug::infof("Write verilog file for '{}'", sram->name());

        auto outputFolder = 
            outputFolderOpt.has_value() ?
            std::move(*outputFolderOpt) :
            config->verilogFolderPath;

        auto verilog = dump::Verilog{sram, std::move(outputFolder)};
        verilog.write();
    }
    catch (const std::exception& err)
    {
        debug::exceptionIn("Write verilog", err.what());
    }

    void writeLibertys(circuit::SRAM* sram, const std::vector<PVT>& pvts)
    try
    {
        debug::infof("Write liberty file for '{}'", sram->name());
        for (const auto& pvt : pvts)
            writeLiberty(sram, pvt);
    }
    catch (const std::exception& err)
    {
        debug::exceptionIn("Write libertys", err.what());
    }

    void writeLiberty(circuit::SRAM* sram, PVT pvt, std::optional<std::filesystem::path> outputPathOpt)
    try
    {
        // Get load and slew 
        debug::infof("Write liberty file in {}", pvt.toString());
        auto load = tech->normalCapacitance();
        auto slew = tech->dffRiseTime();
        std::vector<Number> loads { 0.25 * load, load, 4.0 * load };
        std::vector<Number> slews { 0.25 * slew, slew, 8.0 * slew };

        // Character delay and setuphold
        auto delayResult = characterize::analyzeDelays(sram, pvt, slews, loads);
        auto setupholdResult = characterize::analyzeSetupholds(pvt, slews, slews);

        auto outputPath = 
            outputPathOpt ? 
            *outputPathOpt : 
            config->libFolderPath / std::format("{}_{}.lib", sram->name(), pvt.toString());

        // Write Liberty file
        dump::Liberty{
            sram, std::move(pvt),
            std::move(slews), std::move(loads),
            std::move(delayResult), std::move(setupholdResult),
            std::move(outputPath)
        }.write();
    }
    catch (const std::exception& err)
    {
        debug::exceptionIn("Write liberty", err.what());
    }

}