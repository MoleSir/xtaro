#include "./verifyer.hh"
#include "utility/misc/sys.hh"
#include <format>
#include <utility/debug/debug.hh>
#include <string_view>

namespace xtaro::verify 
{

    void Verifyer::runScript(
        std::string_view cellName,
        const std::filesystem::path& scriptPath, 
        VerifyType type) const
    {
        auto stdoutFilePath = scriptPath.parent_path() / this->stdoutFileName(cellName, type);
        auto stderrFilePath = scriptPath.parent_path() / this->stderrFileName(cellName, type);
        auto cmd = std::format(
            "{} > {} 2> {}",
            scriptPath.string(),
            stdoutFilePath.string(),
            stderrFilePath.string()
        );

        int status = util::execute(cmd);
    }

    static auto formatType(VerifyType type) -> std::string_view {
        switch (type) 
        {
            case VerifyType::Drc: return "drc";
            case VerifyType::Lvs: return "lvs";
        }
        debug::unreachable();
    }

    auto Verifyer::resultFileName(std::string_view cellName, VerifyType type) const -> std::string
    {
        return std::format("{}.{}.report", cellName, formatType(type));
    }

    auto Verifyer::stdoutFileName(std::string_view cellName, VerifyType type) const -> std::string
    {
        return std::format("{}.{}.stdout", cellName, formatType(type));
    }

    auto Verifyer::stderrFileName(std::string_view cellName, VerifyType type) const -> std::string
    {
        return std::format("{}.{}.stderr", cellName, formatType(type));
    }

}