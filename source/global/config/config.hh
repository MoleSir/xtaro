#pragma once

#include <parse/serde/de.hh>

#include <memory>
#include <filesystem>

namespace xtaro::parse { class Json; }
namespace xtaro::parse { class Xml; }
namespace xtaro::parse { class Yuml; }

namespace xtaro
{
    
    struct Config
    {
        static auto initial(const parse::Json&) -> void;
        static auto initial(const parse::Xml&) -> void;
        static auto initial(const parse::Yuml&) -> void;

        // Tech & Output path 
        std::filesystem::path techPath {"./tech/freepdk45"};
        std::filesystem::path outputPath {"./output"};

        // Output options
        bool buildBackend {false};

        // Character infos
        bool useSetupHoldModel {true};

        // Spice
        std::string spiceSimulator {"ngspice"};

        // verifer
        std::string verifyer {"klayout"};

        // Folder path
        std::filesystem::path spiceFolderPath {};
        std::filesystem::path verilogFolderPath {};
        std::filesystem::path gdsFolderPath {};
        std::filesystem::path libFolderPath {};
        std::filesystem::path lefFolderPath {};
        std::filesystem::path simFolderPath {};
        std::filesystem::path tempFolderPath {};
    };

    extern std::unique_ptr<Config const> config;

}

DESERIALIZE_STRUCT(xtaro::Config,
    DE_OPTION_FILED_AS(techPath, tech_path)
    DE_OPTION_FILED_AS(outputPath, output_path)
    DE_OPTION_FILED_AS(buildBackend, build_backend)
    DE_OPTION_FILED_AS(useSetupHoldModel, use_setup_hold_model)
    DE_OPTION_FILED_AS(spiceSimulator, spice_simulator)
    DE_OPTION_FILED_AS(verifyer, verifyer)
);