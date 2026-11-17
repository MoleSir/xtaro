#pragma once

#include "./spicer.hh"

namespace xtaro::characterize 
{

    struct Spectre : public Spicer {

        auto simulationCommand(
            const std::filesystem::path& simFilePath, 
            const std::filesystem::path& tempFolder) const -> std::string override;

        auto measurementFilePath(
            const std::filesystem::path& simFilePath, 
            const std::filesystem::path& tempFolder) const -> std::filesystem::path override;

    };

}