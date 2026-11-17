#include "./config.hh"

#include <parse/json/json.hh>
#include <utility/debug/debug.hh>

#include <utility/misc/string.hh>
#include <utility/misc/file.hh>

#include <memory>
#include <filesystem>

namespace xtaro
{

    std::unique_ptr<Config const> config;

    static void buildOutputFilesName(std::unique_ptr<Config>& info);
    static void buildOutputPath(std::unique_ptr<Config>& info);

    auto Config::initial(const parse::Json& json) -> void 
    try
    {
        auto c = std::make_unique<Config>();
        parse::Deserialize<parse::Json, Config>::from(json, *c);
        buildOutputFilesName(c);
        buildOutputPath(c);

        config = std::move(c);
    }
    catch (const std::exception& e) 
    {
        debug::exceptionIn("Load Config", e.what());
    }

    static void buildOutputPath(std::unique_ptr<Config>& info)
    {
        if (!util::directoryExists(info->outputPath))
        {
            if (!std::filesystem::create_directories(info->outputPath))
                debug::exceptionIn(
                    "Load global info", 
                    std::format("Create output path '{}'", info->outputPath.c_str())
                );
        }

        std::filesystem::create_directories(info->spiceFolderPath);
        std::filesystem::create_directories(info->verilogFolderPath);
        std::filesystem::create_directories(info->gdsFolderPath);

        std::filesystem::create_directories(info->simFolderPath);
        std::filesystem::create_directories(info->libFolderPath);
        std::filesystem::create_directories(info->lefFolderPath);

        std::filesystem::create_directories(info->tempFolderPath);
    }

    static void buildOutputFilesName(std::unique_ptr<Config>& info)
    {
        info->spiceFolderPath = info->outputPath / "spice";
        info->verilogFolderPath = info->outputPath / "verilog";
        info->gdsFolderPath = info->outputPath / "gdsii";

        info->simFolderPath = info->outputPath / "simulation";
        info->libFolderPath = info->outputPath / "liberty";
        info->lefFolderPath = info->outputPath / "lef";

        info->tempFolderPath = info->outputPath / "temp";
    }

}