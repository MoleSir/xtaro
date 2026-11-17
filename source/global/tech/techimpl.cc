#include "./techimpl.hh"
#include "global/tech/layer.hh"
#include "parse/json/json.hh"
#include "utility/debug/debug.hh"
#include "utility/misc/file.hh"
#include <unordered_map>

namespace xtaro 
{

    auto TechImpl::getLayerInfo(Layer layer) -> const parse::TechLayer& {
        auto res = this->layerInfos.find(layer);
        if (res == this->layerInfos.end()) 
            debug::exceptionf("'{}' is not exit", layer);
        return res->second;
    }

    auto TechImpl::assignCompletePath(const std::filesystem::path& techPath) -> void 
    {
        this->spiceLibPath = techPath / this->spiceLibPath;
        this->gdsiiLibPath = techPath / this->gdsiiLibPath;
        this->techLefPath = techPath / this->techLefPath;
        this->drcFilePath = techPath / this->drcFilePath;
        this->lvsFilePath = techPath  / this->lvsFilePath;
        
        if (this->libPropertyPath.has_value()) {
            this->libPropertyPath.emplace(
                techPath / (*this->libPropertyPath)
            );
        }

        for (auto& [_, mosPath] : this->mosModelPaths) 
        {
            mosPath.nmos = techPath / mosPath.nmos;
            mosPath.pmos = techPath / mosPath.pmos;
        }
    }

    auto TechImpl::check() -> void
    {
        if (!util::directoryExists(this->spiceLibPath))
            debug::exceptionf("No exit spice lib '{}'", this->spiceLibPath.string());

        if (!util::directoryExists(this->gdsiiLibPath))
            debug::exceptionf("No exit gdsii lib '{}'", this->gdsiiLibPath.string());

        if (!util::fileExists(this->techLefPath))
            debug::exceptionf("No exit tech lef '{}'", this->techLefPath.string());

        for (auto& [name, mosPath] : this->mosModelPaths) 
        {
            if (!util::fileExists(mosPath.nmos))
                debug::exceptionf("No exit nmos model '{}' in '{}' corner", mosPath.nmos.string(), name);
            if (!util::fileExists(mosPath.pmos))
                debug::exceptionf("No exit nmos podel '{}' in '{}' corner", mosPath.pmos.string(), name);
        }
    }

    auto TechImpl::buildTechlib() -> void
    {
        // Load tech lef
        auto techlef = parse::TechLef::loadFrom(this->techLefPath);
        for (auto& [layername, techlayer] : techlef.layers) {
            auto layer = layerFromString(layername);
            if (layer.has_value())
                this->layerInfos.emplace(*layer, std::move(techlayer));
        }

        // Load lib module property
        if (this->libPropertyPath.has_value()) {
            const auto& path = *this->libPropertyPath;
            auto json = parse::Json::loadFromFile(path);
            parse::Deserialize<parse::Json, std::optional<LibProperty>>::from(json, this->libProperty);
            this->vddName = this->libProperty.value().source.vdd;
            this->gndName = this->libProperty.value().source.gnd;
        }

        // Load all module
        namespace fs = std::filesystem;
        for (const auto& entry : fs::directory_iterator(this->spiceLibPath)) {
            if (fs::is_regular_file(entry)) {
                auto path = entry.path();
                if (path.extension() == ".sp") {
                    auto filename = path.stem().string();
                    this->spicePaths.emplace(std::move(filename), std::move(path));
                }
            }
        }

        namespace fs = std::filesystem;
        for (const auto& entry : fs::directory_iterator(this->gdsiiLibPath)) {
            if (fs::is_regular_file(entry)) {
                auto path = entry.path();
                if (path.extension() == ".gds") {
                    auto filename = path.stem().string();
                    this->gdsiiPaths.emplace(std::move(filename), std::move(path));
                }
            }
        }
    }

}