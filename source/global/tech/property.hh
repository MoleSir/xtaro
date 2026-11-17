#pragma once

#include <string>
#include <unordered_map>
#include <parse/serde/de.hh>
#include <vector>

namespace xtaro 
{

    struct SourceProperty 
    {
        std::string vdd;
        std::string gnd;
    };

    struct ModuleProperty 
    {
        std::string name;
        std::vector<std::string> portsOrder;
    };

    struct LibProperty 
    {
        SourceProperty source;
        std::unordered_map<std::string, ModuleProperty> modules;
    };

}

DESERIALIZE_STRUCT(xtaro::SourceProperty,
    DE_FILED(vdd)
    DE_FILED(gnd)
)

DESERIALIZE_STRUCT(xtaro::ModuleProperty,
    DE_FILED(name)
    DE_FILED_AS(portsOrder, ports_order)
)

DESERIALIZE_STRUCT(xtaro::LibProperty,
    DE_FILED(source)
    DE_FILED(modules)
)