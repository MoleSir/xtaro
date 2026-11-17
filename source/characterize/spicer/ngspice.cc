#include "./ngspice.hh"
#include <filesystem>
#include <format>

namespace xtaro::characterize 
{

    auto NGSpice::simulationCommand(
        const std::filesystem::path& simFilePath, 
        const std::filesystem::path& tempFolder) const -> std::string
    {
        return std::format(
            "ngspice -b -o {} {} > /dev/null 2>&1", 
            this->measurementFilePath(simFilePath, tempFolder).string(),
            simFilePath.string()
        );
    }

    auto NGSpice::measurementFilePath(
        const std::filesystem::path& simFilePath, 
        const std::filesystem::path& tempFolder) const -> std::filesystem::path
    {
        auto simFile = simFilePath.filename();
        return tempFolder / simFile.replace_extension(".meas");
    }

}