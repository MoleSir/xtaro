#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

namespace xtaro::characterize 
{

    struct Spicer 
    {
        
        static auto from(std::string_view) -> std::unique_ptr<Spicer>;

        virtual auto simulationCommand(
            const std::filesystem::path& simFilePath, 
            const std::filesystem::path& tempFolder) const -> std::string = 0;

        virtual auto measurementFilePath(
            const std::filesystem::path& simFilePath, 
            const std::filesystem::path& tempFolder) const -> std::filesystem::path = 0;
    
    };

}