#pragma once

#include <filesystem>
#include <string_view>

namespace xtaro::verify
{

    enum class VerifyType
    {
        Drc, Lvs
    };

    struct Verifyer 
    {
        virtual auto runDrc(
            std::string_view cellName,
            const std::filesystem::path& drcFilePath,
            const std::filesystem::path& gdsFilePath,
            const std::filesystem::path& spiceFilePath,
            const std::filesystem::path& scriptFolder,
            bool extract,
            bool finalVerification
        ) const -> int = 0;

        virtual auto runLvs(
            std::string_view cellName,
            const std::filesystem::path& lvsFilePath,
            const std::filesystem::path& gdsFilePath,
            const std::filesystem::path& spiceFilePath,
            const std::filesystem::path& scriptFolder,
            bool finalVerification
        ) const -> int = 0;

        virtual auto writeDrcScript(
            std::string_view cellName,
            const std::filesystem::path& drcFilePath,
            const std::filesystem::path& gdsFilePath,
            const std::filesystem::path& spiceFilePath,
            const std::filesystem::path& scriptFolder,
            bool extract,
            bool finalVerification,
            const std::filesystem::path& resultFilePath
        ) const -> std::filesystem::path = 0;

        virtual auto writeLvsScript(
            std::string_view cellName,
            const std::filesystem::path& lvsFilePath,
            const std::filesystem::path& gdsFilePath,
            const std::filesystem::path& spiceFilePath,
            const std::filesystem::path& scriptFolder,
            bool finalVerification,
            const std::filesystem::path& resultFilePath
        ) const -> std::filesystem::path = 0;

        void runScript(
            std::string_view cellName,
            const std::filesystem::path& scriptPath, 
            VerifyType type
        ) const;

        auto resultFileName(std::string_view cellName, VerifyType type) const -> std::string;
        auto stdoutFileName(std::string_view cellName, VerifyType type) const -> std::string;
        auto stderrFileName(std::string_view cellName, VerifyType type) const -> std::string;
    };

}