#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string_view>
#include <utility/misc/formator.hh>
#include <vector>

namespace xtaro 
{

    enum class Layer 
    {
        // Front end
        Poly = 0,
        Active,
        Contact,
        NWell,
        PWell,
        NImplant,
        PImplant,

        Metal1,
        Via1,
        Metal2,
        Via2,
        Metal3,
        Via3,
        Metal4,
        Via4,
        Metal5,
        Via5,
        Metal6,

        Text,
        PRBoundary,
    };

    auto layerBelow(Layer layer) -> Layer;
    auto layerAbove(Layer layer) -> Layer;
    auto layerFromString(std::string_view) -> std::optional<Layer>;

    auto layerIsVia(Layer layer) -> bool;
    auto layerIsRouting(Layer layer) -> bool;
    auto layerIsMasterslice(Layer layer) -> bool;
    
    auto layerIsFrontend(Layer layer) -> bool;
    auto layerIsBacktend(Layer layer) -> bool;

    auto layerStacks(Layer routing1, Layer routing2) -> std::vector<Layer>;

    enum class RoutingDirection
    {
        Vert,
        Hori,
    };

}

FORMAT_ENUM(xtaro::Layer,
    Poly, Active, NWell, PWell, NImplant, PImplant,
    Metal1, Via1, Metal2, Via2, Metal3, Via3, Metal4, Via4, Metal5, Via5, Metal6,
    Text, PRBoundary
)

namespace std 
{

    template <>
    struct hash<xtaro::Layer> {
        auto operator () (const xtaro::Layer& layer) const noexcept -> std::uint64_t;
    };

}