#pragma once

#include <string_view>
#include <verify/verifyer/verifyer.hh>
#include <filesystem>

namespace xtaro::verify
{

    struct Calibre : public Verifyer
    {
        auto runDrc(
            std::string_view cellName,
            const std::filesystem::path& drcFilePath,
            const std::filesystem::path& gdsFilePath,
            const std::filesystem::path& spiceFilePath,
            const std::filesystem::path& scriptFolder,
            bool extract,
            bool finalVerification
        ) const -> int override;
        
        auto runLvs(
            std::string_view cellName,
            const std::filesystem::path& lvsFilePath,
            const std::filesystem::path& gdsFilePath,
            const std::filesystem::path& spiceFilePath,
            const std::filesystem::path& scriptFolder,
            bool finalVerification
        ) const -> int override;

        auto writeDrcScript(
            std::string_view cellName,
            const std::filesystem::path& drcFilePath,
            const std::filesystem::path& gdsFilePath,
            const std::filesystem::path& spiceFilePath,
            const std::filesystem::path& scriptFolder,
            bool extract,
            bool finalVerification,
            const std::filesystem::path& resultFilePath
        ) const -> std::filesystem::path override;

        auto writeLvsScript(
            std::string_view cellName,
            const std::filesystem::path& lvsFilePath,
            const std::filesystem::path& gdsFilePath,
            const std::filesystem::path& spiceFilePath,
            const std::filesystem::path& scriptFolder,
            bool finalVerification,
            const std::filesystem::path& resultFilePath
        ) const -> std::filesystem::path override;
    
        auto summaryFileName(std::string_view cellName) const -> std::string;
    };

}