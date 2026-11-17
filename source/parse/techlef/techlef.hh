#pragma once

#include "./model.hh"

#include <unordered_map>
#include <filesystem>

namespace xtaro::parse 
{

    struct TechLef 
    {
        double version;
        std::unordered_map<std::string, TechLayer> layers;
    
        static auto loadFrom(const std::filesystem::path& path) -> TechLef;
    };

}