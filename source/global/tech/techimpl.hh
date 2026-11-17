#include "./property.hh"
#include "global/tech/layer.hh"
#include "global/number/number.hh"
#include "parse/techlef/model.hh"
#include <optional>
#include <parse/techlef/techlef.hh>
#include <parse/serde/de.hh>
#include <utility/debug/debug.hh>
#include <parse/serde/mod.hh>

#include <cstdlib>
#include <filesystem>
#include <string>
#include <unordered_map>

namespace xtaro {

    struct MOSPath
    {
        std::filesystem::path nmos;
        std::filesystem::path pmos;
    };

    struct TechImpl
    {
        std::string libName;
        std::string nmosName;
        std::string pmosName;
        
        Number normalPeriod;
        Number normalVoltage;
        Number normalTemperature;
        Number normalCapacitance;

        Number dffRiseTime;
        Number dffFallTime;
        Number dffHold;
        Number dffSetup;

        std::unordered_map<std::string, MOSPath> mosModelPaths;

        // May get from .lef?
        Number gridWidth;
        Number gdsMetersPerDBunit;

        std::filesystem::path spiceLibPath;
        std::filesystem::path gdsiiLibPath;
        std::filesystem::path techLefPath; 
        std::filesystem::path drcFilePath;
        std::filesystem::path lvsFilePath;
        std::optional<std::filesystem::path> libPropertyPath;

        // ============================================================= //

        std::unordered_map<Layer, parse::TechLayer> layerInfos {};
        std::optional<LibProperty> libProperty {};
        std::string vddName {"vdd"};
        std::string gndName {"gnd"};

        std::unordered_map<std::string, std::filesystem::path> spicePaths {};
        std::unordered_map<std::string, std::filesystem::path> gdsiiPaths {};

        // ============================================================= //

        auto getLayerInfo(Layer layer) -> const parse::TechLayer&;
        auto assignCompletePath(const std::filesystem::path& techPath) -> void;
        auto check() -> void;
        auto buildTechlib() -> void;
    };

}

DESERIALIZE_STRUCT(xtaro::MOSPath,
    DE_FILED(nmos)
    DE_FILED(pmos)
)

DESERIALIZE_STRUCT(xtaro::TechImpl,
    DE_FILED_AS(libName, lib_name)
    DE_FILED_AS(nmosName, nmos_name)
    DE_FILED_AS(pmosName, pmos_name)
    DE_FILED_AS(normalPeriod, normal_period)
    DE_FILED_AS(normalVoltage, normal_voltage)
    DE_FILED_AS(normalTemperature, normal_temperature)
    DE_FILED_AS(dffRiseTime, dff_in_cap)
    DE_FILED_AS(dffRiseTime, dff_rise_time)
    DE_FILED_AS(dffRiseTime, dff_fall_time)
    DE_FILED_AS(dffSetup, dff_setup)
    DE_FILED_AS(dffHold, dff_hold)
    DE_FILED_AS(mosModelPaths, mos_model_paths)
    DE_FILED_AS(gridWidth, grid_width)
    DE_FILED_AS(gdsMetersPerDBunit, gds_meters_per_dbunit)
    DE_FILED_AS(spiceLibPath, spice_lib_path)
    DE_FILED_AS(gdsiiLibPath, gdsii_lib_path)
    DE_FILED_AS(techLefPath, tech_lef_path)
    DE_OPTION_FILED_AS(drcFilePath, drc_file_path)
    DE_OPTION_FILED_AS(lvsFilePath, lvs_file_path)
    DE_OPTION_FILED_AS(libPropertyPath, lib_property_path)
)