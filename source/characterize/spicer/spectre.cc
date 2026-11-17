#include "./spectre.hh"
#include <filesystem>
#include <format>

namespace xtaro::characterize 
{

    auto Spectre::simulationCommand(
        const std::filesystem::path& simFilePath, 
        const std::filesystem::path& tempFolder) const -> std::string
    {
        return std::format(
            "spectre {} -outdir {} > /dev/null 2>&1",
            simFilePath.string(),
            tempFolder.string()
        );
    }

    auto Spectre::measurementFilePath(
        const std::filesystem::path& simFilePath, 
        const std::filesystem::path& tempFolder) const -> std::filesystem::path
    {
        auto simFile = simFilePath.filename();
        return tempFolder / simFile.replace_extension(".measure");
    }

}