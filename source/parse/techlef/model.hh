#pragma once 

#include <optional>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

namespace xtaro::parse 
{

    enum class TechLayerDir { Horizontal, Vertical };

    struct ParallelSpacingTable
    {
        std::vector<double> lengths;
        std::vector<double> widths;
        std::vector<std::vector<double>> spacings;
    };

    using SpacingTable = std::variant<
        ParallelSpacingTable
    >;

    /*
    
        Routing Layer  
        - name
        - mask
        - direction
        - width
        - pitch
        - area
        - spacing

        Via Layer
        - name
        - mask
        - width
        - pitch
        - spacing
        - enclosure below
        - enclosure above
    
    */

    struct TechLayer 
    {
        std::string name;
        std::string type;
        int mask;

        std::optional<TechLayerDir> dir;
        std::optional<double> width;
        std::optional<double> pitch;
        std::optional<double> spacing;
        std::optional<SpacingTable> spacingTable;
        std::optional<double> area;
        std::optional<std::tuple<double, double>> enclosureBelow;
        std::optional<std::tuple<double, double>> enclosureAbove;
    };

}