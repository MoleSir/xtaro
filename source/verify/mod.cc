#include "./mod.hh"
#include "./kalyout/klayout.hh"
#include "./verifyer/verifyer.hh"
#include "global/config/config.hh"
#include "global/tech/tech.hh"
#include "verify/calibre/calibre.hh"
#include <circuit/base/circuit.hh>

#include <exception>
#include <filesystem>
#include <utility/debug/debug.hh>

#include <memory>
#include <string_view>

namespace xtaro::verify 
{

    static auto getVerifyer(std::string_view name) -> std::unique_ptr<Verifyer> 
    {
        if (name == "klayout")
            return std::make_unique<Klayout>();
        else if (name == "calibre")
            return std::make_unique<Calibre>();
        else 
            debug::exceptionf("Unsupport verifyer '{}'", name);
    }

    auto runDrc(
        std::string_view exe, circuit::Circuit* circuit, 
        bool extract, bool finalVerification, std::optional<std::filesystem::path> scriptFolder) -> int
    try 
    {
        auto verifyer = getVerifyer(exe);
        return verifyer->runDrc(
            circuit->name(),
            tech->drcFilePath(),
            circuit->gdsiiFilePath(),
            circuit->spiceFilePath(),
            scriptFolder.has_value() ? *scriptFolder : config->tempFolderPath,
            extract,
            finalVerification
        );
    }
    catch (const std::exception& err) 
    {
        debug::exceptionIn("Run DRC", err.what());
    }

    auto runLvs(
        std::string_view exe, circuit::Circuit* circuit, 
        bool finalVerification, std::optional<std::filesystem::path> scriptFolder) -> int
    try 
    {
        auto verifyer = getVerifyer(exe);
        return verifyer->runLvs(
            circuit->name(),
            tech->lvsFilePath(),
            circuit->gdsiiFilePath(),
            circuit->spiceFilePath(),
            scriptFolder.has_value() ? *scriptFolder : config->tempFolderPath,
            finalVerification
        );
    }
    catch (const std::exception& err) 
    {
        debug::exceptionIn("Run LVS", err.what());
    }

    auto writeDrcScript(
        std::string_view exe, circuit::Circuit* circuit, 
        bool extract, bool finalVerification, std::optional<std::filesystem::path> scriptFolder) -> std::filesystem::path
    try
    {
        auto verifyer = getVerifyer(exe);
        return verifyer->writeDrcScript(
            circuit->name(),
            tech->lvsFilePath(),
            circuit->gdsiiFilePath(),
            circuit->spiceFilePath(),
            scriptFolder.has_value() ? *scriptFolder : config->tempFolderPath,
            extract,
            finalVerification,
            verifyer->resultFileName(circuit->name(), VerifyType::Drc)
        );   
    }
    catch (const std::exception& err) 
    {
        debug::exceptionIn("Write DRC Script", err.what());
    }

    auto writeLvsScript(
        std::string_view exe, circuit::Circuit* circuit, 
        bool finalVerification, std::optional<std::filesystem::path> scriptFolder) -> std::filesystem::path
    try
    {
        auto verifyer = getVerifyer(exe);
        return verifyer->writeLvsScript(
            circuit->name(),
            tech->lvsFilePath(),
            circuit->gdsiiFilePath(),
            circuit->spiceFilePath(),
            scriptFolder.has_value() ? *scriptFolder : config->tempFolderPath,
            finalVerification,
            verifyer->resultFileName(circuit->name(), VerifyType::Lvs)
        );
    }
    catch (const std::exception& err) 
    {
        debug::exceptionIn("Write LVS Script", err.what());
    }

}