#pragma once

#include "./layer.hh"
#include "tech/property.hh"

#include <global/number/number.hh>
#include <global/env/pvt.hh>
#include <global/env/slewload.hh>
#include <global/env/slews.hh>
#include <filesystem>
#include <optional>
#include <string_view>
#include <tuple>

namespace xtaro
{
    using LayerMask = short;
    
    class TechImpl;

    class Tech 
    {
    public:
        static auto initial(const std::filesystem::path& techPath) -> void;

        // Name's info
        auto libName() const -> std::string_view;
        auto nmosName() const -> std::string_view;
        auto pmosName() const -> std::string_view;

        // Normal value
        auto normalPeriod() const -> Number;
        auto normalVoltage() const -> Number;
        auto normalTemperature() const -> Number;
        auto normalCapacitance() const -> Number;

        // Timing info
        auto dffRiseTime() const -> Number;
        auto dffFallTime() const -> Number;
        auto dffHold() const -> Number;
        auto dffSetup() const -> Number;

        // Folder path 
        auto drcFilePath() const -> std::filesystem::path;
        auto lvsFilePath() const -> std::filesystem::path;
        
        auto spiceLibPath() const -> std::filesystem::path;
        auto gdsiiLibPath() const -> std::filesystem::path;
        auto techLefPath() const -> std::filesystem::path;

        auto nmosModelPath(const std::string& corner) const -> std::filesystem::path;
        auto pmosModelPath(const std::string& corner) const -> std::filesystem::path;

        auto spiceFilePath(const std::string& circuit) const -> std::filesystem::path;
        auto gdsiiFilePath(const std::string& circuit) const -> std::filesystem::path;
        
        auto moduleProperty(const std::string& modname) const -> std::optional<ModuleProperty*>;
        auto moduleProperty(std::string_view modname) const -> std::optional<ModuleProperty*>;

        auto vdd() const -> std::string_view;
        auto gnd() const -> std::string_view;

        // DRC
        auto gridWidth() const -> Number;

        auto layerFromMask(LayerMask mask) const -> Layer;

        auto gdsMetersPerDBunit() const -> Number;
        
        // MARK: Spacing table
        auto layerMask(Layer layer) const -> LayerMask;
        auto layerSpacing(Layer layer, std::optional<Number> width = std::nullopt) const -> Number;
        auto layerWidth(Layer layer) const -> Number;
        auto layerPitch(Layer layer) const -> Number;
        auto layerArea(Layer layer) const -> Number;
        auto layerDirection(Layer layer) const -> RoutingDirection;
        auto layerEnclosureBelow(Layer via) const -> std::tuple<Number, Number>;
        auto layerEnclosureAbove(Layer via) const -> std::tuple<Number, Number>;

        // Pre define
        auto metal1Mask() const -> LayerMask;
        auto metal1Spacing(std::optional<Number> width = std::nullopt) const -> Number;
        auto metal1Width() const -> Number;
        auto metal1Pitch() const -> Number;
        auto metal1Area() const -> Number;
        auto metal1Direction() const -> RoutingDirection;

        auto metal2Mask() const -> LayerMask;
        auto metal2Spacing(std::optional<Number> width = std::nullopt) const -> Number;
        auto metal2Width() const -> Number;
        auto metal2Pitch() const -> Number;
        auto metal2Area() const -> Number;
        auto metal2Direction() const -> RoutingDirection;

        auto metal3Mask() const -> LayerMask;
        auto metal3Spacing(std::optional<Number> width = std::nullopt) const -> Number;
        auto metal3Width() const -> Number;
        auto metal3Pitch() const -> Number;
        auto metal3Area() const -> Number;
        auto metal3Direction() const -> RoutingDirection;

        auto via1Mask() const -> LayerMask;
        auto via1Spacing(std::optional<Number> width = std::nullopt) const -> Number;
        auto via1Width() const -> Number;
        auto via1Pitch() const -> Number;
        auto via1EnclosureM1() const -> std::tuple<Number, Number>;
        auto via1EnclosureM2() const -> std::tuple<Number, Number>;

        auto via2Mask() const -> LayerMask;
        auto via2Spacing(std::optional<Number> width = std::nullopt) const -> Number;
        auto via2Width() const -> Number;
        auto via2Pitch() const -> Number;
        auto via2EnclosureM2() const -> std::tuple<Number, Number>;
        auto via2EnclosureM3() const -> std::tuple<Number, Number>;

        auto via3Mask() const -> LayerMask;
        auto via3Spacing(std::optional<Number> width = std::nullopt) const -> Number;
        auto via3Width() const -> Number;
        auto via3Pitch() const -> Number;
        auto via3EnclosureM3() const -> std::tuple<Number, Number>;
        auto via3EnclosureM4() const -> std::tuple<Number, Number>;

    private:
        std::unique_ptr<TechImpl> _impl {nullptr};
    
    public:
        Tech(std::unique_ptr<TechImpl> impl);
        ~Tech() noexcept;
    };

    extern std::unique_ptr<Tech const> tech;

}
