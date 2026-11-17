#include "./layer.hh"
#include "utility/misc/string.hh"
#include <optional>
#include <utility/debug/debug.hh>
#include <cstdint>
#include <functional>
#include <vector>

namespace xtaro 
{
    
    // MARK: Contact 's below and above
    auto layerBelow(Layer layer) -> Layer 
    {
        auto hash = std::hash<Layer>{};
        auto layerIndex = hash(layer);
        if (layerIndex == 0) {
            debug::exceptionf("No layer below '{}'", layer);
        } 
        return static_cast<Layer>(layerIndex - 1);
    }

    auto layerAbove(Layer layer) -> Layer
    {
        auto hash = std::hash<Layer>{};
        auto max = hash(Layer::Metal6);
        auto layerIndex = hash(layer);
        if (layer == Layer::Metal6) {
            debug::exceptionf("No layer above '{}'", layer);
        } 
        return static_cast<Layer>(layerIndex + 1);
    }

    auto layerFromString(std::string_view name) -> std::optional<Layer>
    {
        auto lowecaseName = util::stringToLowercase(name);

        #define CHECK_METAL_LAYER(n, i)\
        n == "metal"#i || n == "m"#i

        #define CHECK_VIA_LAYER(n, i)\
        n == "via"#i || n == "v"#i

        if (CHECK_METAL_LAYER(lowecaseName, 1)) return Layer::Metal1;
        if (CHECK_METAL_LAYER(lowecaseName, 2)) return Layer::Metal2;
        if (CHECK_METAL_LAYER(lowecaseName, 3)) return Layer::Metal3;
        if (CHECK_METAL_LAYER(lowecaseName, 4)) return Layer::Metal4;
        if (CHECK_METAL_LAYER(lowecaseName, 5)) return Layer::Metal5;
        if (CHECK_METAL_LAYER(lowecaseName, 6)) return Layer::Metal6;

        if (CHECK_VIA_LAYER(lowecaseName, 1)) return Layer::Via1;
        if (CHECK_VIA_LAYER(lowecaseName, 2)) return Layer::Via2;
        if (CHECK_VIA_LAYER(lowecaseName, 3)) return Layer::Via3;
        if (CHECK_VIA_LAYER(lowecaseName, 4)) return Layer::Via4;
        if (CHECK_VIA_LAYER(lowecaseName, 5)) return Layer::Via5;

        if (lowecaseName == "poly") return Layer::Poly;    
        if (lowecaseName == "active") return Layer::Active;    
        if (lowecaseName == "nwell") return Layer::NWell;    
        if (lowecaseName == "pwell") return Layer::PWell;
        if (lowecaseName == "nimplant") return Layer::NImplant;
        if (lowecaseName == "pimplant") return Layer::PImplant;

        if (lowecaseName == "text") return Layer::Text;
        if (lowecaseName == "boundary" || lowecaseName == "prboundary") return Layer::PRBoundary;
    
        debug::warningf("'{}' is not a supported layer name", name);

        #undef CHECK_METAL_LAYER
        #undef CHECK_VIA_LAYER

        return std::nullopt;
    } 

    auto layerIsVia(Layer layer) -> bool 
    {
        return 
            layer == Layer::Via1 ||
            layer == Layer::Via2 ||
            layer == Layer::Via3 ||
            layer == Layer::Via4 ||
            layer == Layer::Via5;
    }

    auto layerIsRouting(Layer layer) -> bool
    {
        return 
            layer == Layer::Metal1 ||
            layer == Layer::Metal2 ||
            layer == Layer::Metal3 ||
            layer == Layer::Metal4 ||
            layer == Layer::Metal5;
    }

    auto layerIsMasterslice(Layer layer) -> bool
    {
        return 
            layer == Layer::Poly ||
            layer == Layer::Active ||
            layer == Layer::Contact ||
            layer == Layer::NWell ||
            layer == Layer::PWell ||
            layer == Layer::NImplant ||
            layer == Layer::PImplant;
    }
    
    auto layerIsFrontend(Layer layer) -> bool
    {
        return layerIsMasterslice(layer);
    }

    auto layerIsBacktend(Layer layer) -> bool
    {
        return layerIsRouting(layer) || layerIsVia(layer);
    }

    auto layerStacks(Layer routing1, Layer routing2) -> std::vector<Layer>
    {
        debug::checkf(layerIsRouting(routing1), "'{}' is not an routing layer", routing1);
        debug::checkf(layerIsRouting(routing2), "'{}' is not an routing layer", routing2);

        if (routing1 == routing2)
            return {};

        auto hash = std::hash<Layer>{};
        auto hash1 = hash(routing1);
        auto hash2 = hash(routing2);

        auto minRouting = hash1 < hash2 ? routing1 : routing2; 
        auto maxRouting = hash1 > hash2 ? routing1 : routing2;

        // Metal1 => Metal2: {Via1}
        // Metal1 => Metal3: {Via1, Metal2, Via2}
        auto layer = layerAbove(minRouting);
        auto layers = std::vector<Layer>{};
        while (layer != maxRouting)
        {
            layers.emplace_back(layer);
            layer = layerAbove(layer);
        }

        return layers;
    }

}

namespace std
{

    auto hash<xtaro::Layer>::operator () (const xtaro::Layer& layer) const noexcept -> std::uint64_t {
        return static_cast<std::uint64_t>(layer);
    }

}