#include "./tech.hh"
#include "./techimpl.hh"

#include "global/number/number.hh"
#include "parse/json/json.hh"
#include "parse/techlef/model.hh"
#include "utility/misc/file.hh"
#include <cstdlib>
#include <filesystem>
#include <format>
#include <memory>
#include <optional>
#include <parse/techlef/techlef.hh>
#include <string>
#include <string_view>
#include <parse/serde/de.hh>
#include <utility/debug/debug.hh>

namespace xtaro 
{
    extern std::unique_ptr<Tech const> tech;

    std::unique_ptr<Tech const> tech = std::unique_ptr<Tech const>{nullptr};

    static constexpr std::string_view techInfoFile = "tech.json";

    auto Tech::initial(const std::filesystem::path& techPath) -> void
    try
    {
        debug::debugf("Load tech from '{}'", techPath.string());
        auto techlef = techPath / techInfoFile;
        if (!util::fileExists(techlef))
            debug::exceptionf("Tech info file {} no exit in '{}'", techInfoFile, techPath.string());

        // Deser from file
        auto techimpl 
            = parse::deserializeFrom<parse::Json, std::unique_ptr<TechImpl>>(techlef);

        techimpl->assignCompletePath(techPath);
        techimpl->check();
        techimpl->buildTechlib();

        // Assign global
        tech = std::make_unique<Tech>(std::move(techimpl));
    } 
    catch (const std::exception& e) 
    {
        debug::exceptionIn("Load Technology", e.what());
    }

    Tech::Tech(std::unique_ptr<TechImpl> impl) :
        _impl{std::move(impl)} 
    {
    } 

    Tech::~Tech() noexcept = default;

    // Name's info
    auto Tech::libName() const -> std::string_view {
        return this->_impl->libName;
    }
    
    auto Tech::nmosName() const -> std::string_view{
        return this->_impl->nmosName;
    }
    
    auto Tech::pmosName() const -> std::string_view{
        return this->_impl->pmosName;
    }
    
    auto Tech::normalPeriod() const -> Number{
        return this->_impl->normalPeriod;
    }
    
    auto Tech::normalVoltage() const -> Number{
        return this->_impl->normalVoltage;
    }
    
    auto Tech::normalTemperature() const -> Number{
        return this->_impl->normalTemperature;
    }
    
    auto Tech::normalCapacitance() const -> Number {
        return this->_impl->normalCapacitance;
    }

    auto Tech::dffRiseTime() const -> Number{
        return this->_impl->dffRiseTime;
    }

    auto Tech::dffFallTime() const -> Number{
        return this->_impl->dffFallTime;
    }

    auto Tech::dffHold() const -> Number{
        return this->_impl->dffHold;
    }

    auto Tech::dffSetup() const -> Number {
        return this->_impl->dffSetup;
    }

    auto Tech::drcFilePath() const -> std::filesystem::path {
        return this->_impl->drcFilePath;
    }

    auto Tech::lvsFilePath() const -> std::filesystem::path {
        return this->_impl->lvsFilePath;
    }

    auto Tech::spiceLibPath() const -> std::filesystem::path{
        return this->_impl->spiceLibPath;
    }

    auto Tech::gdsiiLibPath() const -> std::filesystem::path{
        return this->_impl->gdsiiLibPath;
    }

    auto Tech::techLefPath() const -> std::filesystem::path{
        return this->_impl->techLefPath;
    }

    auto Tech::gridWidth() const -> Number {
        return this->_impl->gridWidth;
    }

    auto Tech::gdsMetersPerDBunit() const -> Number {
        return this->_impl->gdsMetersPerDBunit;
    }

    auto Tech::nmosModelPath(const std::string& corner) const -> std::filesystem::path {
        auto res = this->_impl->mosModelPaths.find(corner);
        if (res == this->_impl->mosModelPaths.end()) 
            debug::exceptionf("No exit corner '{}' in model path", corner);
        return res->second.nmos;
    }

    auto Tech::pmosModelPath(const std::string& corner) const -> std::filesystem::path {
        auto res = this->_impl->mosModelPaths.find(corner);
        if (res == this->_impl->mosModelPaths.end()) 
            debug::exceptionf("No exit corner '{}' in model path", corner);
        return res->second.pmos;
    }

    template <typename Value, typename F>
    auto getFromMap(
        const std::unordered_map<std::string, Value>& map, 
        const std::string& key,
        F f
    ) -> const Value& {
        auto res = map.find(key);
        if (res == map.end())
            debug::exception(f());
        return res->second;
    }

    auto Tech::spiceFilePath(const std::string& circuit) const -> std::filesystem::path {
        return getFromMap(this->_impl->spicePaths, circuit, [&](){ return std::format("No exit circuit '{}' in spice lib", circuit); });
    }

    auto Tech::gdsiiFilePath(const std::string& circuit) const -> std::filesystem::path {
        return getFromMap(this->_impl->gdsiiPaths, circuit, [&](){ return std::format("No exit circuit '{}' in gdsii lib", circuit); });
    }

    auto Tech::vdd() const -> std::string_view {
        return this->_impl->vddName;
    }

    auto Tech::gnd() const -> std::string_view {
        return this->_impl->gndName;
    }

    auto Tech::moduleProperty(const std::string& modname) const -> std::optional<ModuleProperty*> {
        if (!this->_impl->libProperty.has_value()) return std::nullopt;
        auto res = this->_impl->libProperty.value().modules.find(modname);
        if (res == this->_impl->libProperty.value().modules.end()) 
            return std::nullopt;
        else
            return {&res->second};
    }

    auto Tech::moduleProperty(std::string_view modname) const -> std::optional<ModuleProperty*> {
        return this->moduleProperty(std::string{modname});
    }

    auto Tech::layerFromMask(LayerMask mask) const -> Layer {
        for (auto& [layer, info] : this->_impl->layerInfos) {
            if (info.mask == mask) 
                return layer;
        }
        debug::exceptionf("No exit layer with mask '{}'", mask);
    }

    auto Tech::layerMask(Layer layer) const -> LayerMask {
        auto layerInfo = this->_impl->getLayerInfo(layer);
        return layerInfo.mask;
    }

    auto Tech::layerSpacing(Layer layer, std::optional<Number> width) const -> Number {
        auto layerInfo = this->_impl->getLayerInfo(layer);
        // MARK: Spacing table
        if (layerInfo.spacing.has_value()) 
            return Number::micro(*layerInfo.spacing);
        debug::exceptionf("Layer '{}' has no attr spacing", layer);
    }

    auto Tech::layerWidth(Layer layer) const -> Number {
        auto layerInfo = this->_impl->getLayerInfo(layer);
        if (layerInfo.width.has_value()) 
            return Number::micro(*layerInfo.width);
        debug::exceptionf("Layer '{}' has no attr width", layer);
    }

    auto Tech::layerPitch(Layer layer) const -> Number {
        auto layerInfo = this->_impl->getLayerInfo(layer);
        if (layerInfo.pitch.has_value()) 
            return Number::micro(*layerInfo.pitch);
        debug::exceptionf("Layer '{}' has no attr pitch", layer);
    }

    auto Tech::layerArea(Layer layer) const -> Number{
        auto layerInfo = this->_impl->getLayerInfo(layer);
        if (layerInfo.area.has_value()) 
            return Number::micro(*layerInfo.area);
        debug::exceptionf("Layer '{}' has no attr area", layer);
    }

    auto Tech::layerDirection(Layer layer) const -> RoutingDirection {
        auto layerInfo = this->_impl->getLayerInfo(layer);
        if (layerInfo.dir.has_value()) 
            return ((*layerInfo.dir) == parse::TechLayerDir::Horizontal) ? RoutingDirection::Hori : RoutingDirection::Vert;
        debug::exceptionf("Layer '{}' has no attr direction", layer);
    }

    auto Tech::layerEnclosureBelow(Layer via) const -> std::tuple<Number, Number> {
        auto layerInfo = this->_impl->getLayerInfo(via);
        if (layerInfo.type == "CUT" || layerInfo.type == "cut") {
            if (layerInfo.enclosureBelow.has_value()) {
                auto [overhang1, overhang2] = *layerInfo.enclosureBelow;
                return {
                    Number::micro(overhang1),
                    Number::micro(overhang2)
                };
            }
            debug::exceptionf("Via '{}' has no attr EnclosureBelow", via);
        }
        debug::exceptionf("Try get enclosure from '{}' which is not a via layer", via);
    }

    auto Tech::layerEnclosureAbove(Layer via) const -> std::tuple<Number, Number> {
        auto layerInfo = this->_impl->getLayerInfo(via);
        if (layerInfo.type == "CUT" || layerInfo.type == "cut") {
            if (layerInfo.enclosureAbove.has_value()) {
                auto [overhang1, overhang2] = *layerInfo.enclosureAbove;
                return {
                    Number::micro(overhang1),
                    Number::micro(overhang2)
                };
            }
            debug::exceptionf("Via '{}' has no attr EnclosureAbove", via);
        }
        debug::exceptionf("Try get enclosure from '{}' which is not a via layer", via);
    }

    auto Tech::metal1Mask() const -> LayerMask{
        return this->layerMask(Layer::Metal1);
    }

    auto Tech::metal1Spacing(std::optional<Number> width) const -> Number{
        return this->layerSpacing(Layer::Metal1);
    }

    auto Tech::metal1Width() const -> Number{
        return this->layerWidth(Layer::Metal1);
    }

    auto Tech::metal1Pitch() const -> Number{
        return this->layerPitch(Layer::Metal1);
    }

    auto Tech::metal1Area() const -> Number{
        return this->layerArea(Layer::Metal1);
    }

    auto Tech::metal1Direction() const -> RoutingDirection{
        return this->layerDirection(Layer::Metal1);
    }

    auto Tech::metal2Mask() const -> LayerMask{
        return this->layerMask(Layer::Metal2);
    }

    auto Tech::metal2Spacing(std::optional<Number> width) const -> Number{
        return this->layerSpacing(Layer::Metal2);
    }

    auto Tech::metal2Width() const -> Number{
        return this->layerWidth(Layer::Metal2);
    }

    auto Tech::metal2Pitch() const -> Number{
        return this->layerPitch(Layer::Metal2);
    }

    auto Tech::metal2Area() const -> Number{
        return this->layerArea(Layer::Metal2);
    }

    auto Tech::metal2Direction() const -> RoutingDirection{
        return this->layerDirection(Layer::Metal2);
    }

    auto Tech::metal3Mask() const -> LayerMask{
        return this->layerMask(Layer::Metal3);
    }

    auto Tech::metal3Spacing(std::optional<Number> width) const -> Number{
        return this->layerSpacing(Layer::Metal3);
    }

    auto Tech::metal3Width() const -> Number{
        return this->layerWidth(Layer::Metal3);
    }

    auto Tech::metal3Pitch() const -> Number{
        return this->layerPitch(Layer::Metal3);
    }

    auto Tech::metal3Area() const -> Number{
        return this->layerArea(Layer::Metal3);
    }

    auto Tech::metal3Direction() const -> RoutingDirection{
        return this->layerDirection(Layer::Metal3);
    }

    auto Tech::via1Mask() const -> LayerMask{
        return this->layerMask(Layer::Via1);
    }

    auto Tech::via1Spacing(std::optional<Number> width) const -> Number{
        return this->layerSpacing(Layer::Via1);
    }

    auto Tech::via1Width() const -> Number{
        return this->layerWidth(Layer::Via1);
    }

    auto Tech::via1Pitch() const -> Number{
        return this->layerPitch(Layer::Via1);
    }

    auto Tech::via1EnclosureM1() const -> std::tuple<Number, Number> {
        return this->layerEnclosureBelow(Layer::Via1);
    }

    auto Tech::via1EnclosureM2() const -> std::tuple<Number, Number> {
        return this->layerEnclosureAbove(Layer::Via1);
    }

    auto Tech::via2Mask() const -> LayerMask{
        return this->layerMask(Layer::Via2);
    }

    auto Tech::via2Spacing(std::optional<Number> width) const -> Number{
        return this->layerSpacing(Layer::Via2);
    }

    auto Tech::via2Width() const -> Number{
        return this->layerWidth(Layer::Via2);
    }

    auto Tech::via2Pitch() const -> Number{
        return this->layerPitch(Layer::Via2);
    }

    auto Tech::via2EnclosureM2() const -> std::tuple<Number, Number>{
        return this->layerEnclosureBelow(Layer::Via2);
    }

    auto Tech::via2EnclosureM3() const -> std::tuple<Number, Number>{
        return this->layerEnclosureAbove(Layer::Via2);
    }

    auto Tech::via3Mask() const -> LayerMask{
        return this->layerMask(Layer::Via3);
    }

    auto Tech::via3Spacing(std::optional<Number> width) const -> Number{
        return this->layerSpacing(Layer::Via3);
    }

    auto Tech::via3Width() const -> Number{
        return this->layerWidth(Layer::Via3);
    }

    auto Tech::via3Pitch() const -> Number{
        return this->layerPitch(Layer::Via3);
    }

    auto Tech::via3EnclosureM3() const -> std::tuple<Number, Number> {
        return this->layerEnclosureBelow(Layer::Via3);
    }

    auto Tech::via3EnclosureM4() const -> std::tuple<Number, Number> {
        return this->layerEnclosureAbove(Layer::Via3);
    }


}