#include "./klayout.hh"
#include "utility/debug/debug.hh"
#include "utility/misc/file.hh"
#include <filesystem>
#include <string>
#include <utility/misc/sys.hh>
#include <format>
#include <fstream>
#include <string_view>
#include <algorithm>

namespace xtaro::verify
{

    auto Klayout::runDrc(
        std::string_view cellName,
        const std::filesystem::path& drcFilePath,
        const std::filesystem::path& gdsFilePath,
        const std::filesystem::path& spiceFilePath,
        const std::filesystem::path& scriptFolder,
        bool extract,
        bool finalVerification
    ) const -> int
    {
        auto resultFileName = this->resultFileName(cellName, VerifyType::Drc);
        auto scriptPath = this->writeDrcScript(cellName, drcFilePath, gdsFilePath, spiceFilePath, scriptFolder, extract, finalVerification, resultFileName);
        this->runScript(cellName, scriptPath, VerifyType::Drc);

        /*
            Check the result for these lines in the summary:
            Total DRC errors found: 0
            The count is shown in this format:
            Cell replica_cell_6t has 3 error tiles.
            Cell tri_gate_array has 8 error tiles.
            etc.
        */

        auto reportFilePath = scriptFolder / resultFileName;
        auto results = util::readFileLines(reportFilePath);

        auto errors = std::count_if(results.cbegin(), results.cend(), [] (const std::string& line) -> bool {
            return std::string::npos != line.find("<visited>");
        });

        // Display summary
        debug::infof("DRC Errors {}\t{}", cellName, errors);
        debug::infof("See '{}' for more details", reportFilePath.string());

        return errors;
    }

    auto Klayout::runLvs(
        std::string_view cellName,
        const std::filesystem::path& lvsFilePath,
        const std::filesystem::path& gdsFilePath,
        const std::filesystem::path& spiceFilePath,
        const std::filesystem::path& scriptFolder,
        bool finalVerification
    ) const -> int
    {
        auto resultFileName = this->resultFileName(cellName, VerifyType::Lvs);
        auto scriptPath = this->writeLvsScript(cellName, lvsFilePath, gdsFilePath, spiceFilePath, scriptFolder, finalVerification, resultFileName);
        this->runScript(cellName, scriptPath, VerifyType::Lvs);

        auto results = util::readFileLines(scriptFolder / resultFileName);
    
        auto congrats = std::count_if(results.cbegin(), results.cend(), [] (const std::string& line) -> bool {
            return std::string::npos != line.find("CONGRATULATIONS");
        });
        auto errors = std::count_if(results.cbegin(), results.cend(), [] (const std::string& line) -> bool {
            return std::string::npos != line.find("ERROR");
        });

        if (errors > 0)
            debug::errorf("{}\tLVS mismatch (result in {})", cellName, resultFileName);
        else if (congrats > 0)
            debug::infof("{}\tLVS matches", cellName);
        else 
            debug::infof("{}\tNo LVS result", cellName);

        return errors;
    }

    auto Klayout::writeDrcScript(
        std::string_view cellName,
        const std::filesystem::path& drcFilePath,
        const std::filesystem::path& gdsFilePath,
        const std::filesystem::path& _,
        const std::filesystem::path& scriptFolder,
        bool extract,
        bool finalVerification,
        const std::filesystem::path& resultFilePath
    ) const -> std::filesystem::path
    {
        if (!util::fileExists(gdsFilePath)) 
            debug::exceptionf("No exit gdsii file '{}'", gdsFilePath.string());

        auto runFilePath = scriptFolder / "run_drc.sh";
        auto copyGdsFilePath = scriptFolder / gdsFilePath.filename();
        std::filesystem::copy_file(gdsFilePath, copyGdsFilePath, std::filesystem::copy_options::overwrite_existing);

        // klayout -b -r drc_FreePDK45.lydrc -rd input=sram_8_256_freepdk45.gds -rd topcell=sram_8_256_freepdk45 -rd output=drc_FreePDK45.lyrdb
        auto cmd = std::format(
            "klayout -b -r {} -rd input={} -rd topcell={} -rd output={}",
            drcFilePath.string(),
            copyGdsFilePath.string(),
            cellName,
            resultFilePath.string()
        );

        auto file = std::ofstream{runFilePath};
        file << "#!/bin/sh\n";
        file << cmd << '\n';
        file.close();
        util::execute(std::format("chmod u+x {}", runFilePath.string()));
    
        return runFilePath;
    }

    // Write a netgen script to perform LVS.
    auto Klayout::writeLvsScript(
        std::string_view cellName,
        const std::filesystem::path& lvsFilePath,
        const std::filesystem::path& gdsFilePath,
        const std::filesystem::path& spiceFilePath,
        const std::filesystem::path& scriptFolder,
        bool finalVerification,
        const std::filesystem::path& resultFilePath
    ) const -> std::filesystem::path
    {
        if (!util::fileExists(gdsFilePath)) 
            debug::exceptionf("No exit gdsii file '{}'", gdsFilePath.string());
        if (!util::fileExists(spiceFilePath)) 
            debug::exceptionf("No exit spice file '{}'", spiceFilePath.string());

        // klayout -b -rd input=sram_32_2048_freepdk45.gds -rd report=my_report.lyrdb -rd schematic=sram_32_2048_freepdk45.sp -rd target_netlist=sram_32_2048_freepdk45_extracted.cir -r lvs_freepdk45.lvs
        auto runFilePath = scriptFolder / "run_drc.sh";
        
        auto copyGdsFilePath = scriptFolder / gdsFilePath.filename();
        std::filesystem::copy_file(gdsFilePath, copyGdsFilePath, std::filesystem::copy_options::overwrite_existing);

        auto copySpiceFilePath = scriptFolder / spiceFilePath.filename();
        std::filesystem::copy_file(spiceFilePath, copySpiceFilePath, std::filesystem::copy_options::overwrite_existing);

        auto connectSupplies = "";
        if (finalVerification)
            connectSupplies = "-rd connect_supplies=1";
        auto cmd = std::format(
            "klayout -b -r {} -rd input={} -rd report={} -rd schematic={} -rd target_netlist={}.spice {}",
            lvsFilePath.string(),
            copyGdsFilePath.string(),
            resultFilePath.string(),
            copySpiceFilePath.filename().string(),
            cellName,
            connectSupplies
        );

        auto file = std::ofstream{runFilePath};
        file << "#!/bin/sh\n";
        file << cmd << '\n';
        file.close();
        util::execute(std::format("chmod u+x {}", runFilePath.string()));
    
        return runFilePath;
    }

}