#include "./calibre.hh"
#include "utility/debug/debug.hh"
#include "utility/misc/file.hh"
#include <filesystem>
#include <string>
#include <utility/misc/sys.hh>
#include <format>
#include <fstream>
#include <string_view>
#include <filesystem>

namespace fs = std::filesystem;

namespace xtaro::verify 
{
    auto Calibre::runDrc(
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
        auto resultFilePath = scriptFolder / resultFileName;
        auto scriptPath = this->writeDrcScript(cellName, drcFilePath, gdsFilePath, spiceFilePath, scriptFolder, extract, finalVerification, resultFilePath);
        this->runScript(cellName, scriptPath, VerifyType::Drc);
    
        auto summaryFile = this->summaryFileName(cellName);
        auto lines = util::readFileLines(scriptFolder / summaryFile);
        
        debug::infof("DRC Result for '{}'", cellName);
        for (int i = 6; i > 0; --i) 
            debug::infof("{}", lines.at(lines.size() - i));

        debug::infof("See '{}' for more detail", resultFilePath.string());

        return 0;
    }

    auto Calibre::writeDrcScript(
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
        
        auto runsetFilePath = scriptFolder / "drc_runset";
        auto runsetFile = std::ofstream{runsetFilePath};
        #define WRAP_KEY_VALUE(key, value) "*" key ": " << (value) << '\n'
        // runsetFile << "*drcRulesFile: " << drcFilePath;
        runsetFile << WRAP_KEY_VALUE("drcRulesFile", fs::absolute(drcFilePath).string());
        runsetFile << WRAP_KEY_VALUE("drcRunDir", fs::absolute(scriptFolder).string());
        runsetFile << WRAP_KEY_VALUE("drcLayoutPaths", fs::absolute(gdsFilePath).string());
        runsetFile << WRAP_KEY_VALUE("drcLayoutPrimary", cellName);
        runsetFile << WRAP_KEY_VALUE("drcLayoutSystem", "GDSII");
        runsetFile << WRAP_KEY_VALUE("drcResultsformat", "ASCII");
        runsetFile << WRAP_KEY_VALUE("drcResultsFile", fs::absolute(resultFilePath).string());
        runsetFile << WRAP_KEY_VALUE("drcSummaryFile", this->summaryFileName(cellName));
        runsetFile << WRAP_KEY_VALUE("cmnFDIUseLayerMap", 1);
        #undef WRAP_KEY_VALUE
        runsetFile.close();
        
        auto runFilePath = scriptFolder / "run_drc.sh";
        auto runFile = std::ofstream{runFilePath};
        runFile << "#!/bin/sh\n";
        runFile << std::format("ilaunch calibre -gui -drc {} -batch\n", fs::absolute(runsetFilePath).string());
        runFile.close();
        util::execute(std::format("chmod u+x {}", runFilePath.string()));

        return runFilePath;
    }

    auto Calibre::runLvs(
        std::string_view cellName,
        const std::filesystem::path& lvsFilePath,
        const std::filesystem::path& gdsFilePath,
        const std::filesystem::path& spiceFilePath,
        const std::filesystem::path& scriptFolder,
        bool finalVerification
    ) const -> int 
    {
        debug::unimplement();
    }

    auto Calibre::writeLvsScript(
        std::string_view cellName,
        const std::filesystem::path& lvsFilePath,
        const std::filesystem::path& gdsFilePath,
        const std::filesystem::path& spiceFilePath,
        const std::filesystem::path& scriptFolder,
        bool finalVerification,
        const std::filesystem::path& resultFilePath
    ) const -> std::filesystem::path
    {
        debug::unimplement();
    }

    auto Calibre::summaryFileName(std::string_view cellName) const -> std::string
    {
        return std::format("{}.drc.summary", cellName);
    }
}