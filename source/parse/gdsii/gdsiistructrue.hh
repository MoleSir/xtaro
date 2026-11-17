#pragma once

#include "gdsiiprimitives.hh"
#include <vector>
#include <memory>

namespace xtaro::parse 
{

    struct GDSIIStructure
    {        
        static auto withName(std::string name) -> GDSIIStructure;

        std::string name;

        GDSIIDateTime createDate {};
        GDSIIDateTime modifyDate {};

        std::vector<std::unique_ptr<GDSIIBoundary>> boundarys;
        std::vector<std::unique_ptr<GDSIIPath>> paths;
        std::vector<std::unique_ptr<GDSIISref>> srefs;
        std::vector<std::unique_ptr<GDSIIAref>> arefs;
        std::vector<std::unique_ptr<GDSIIText>> texts;
        std::vector<std::unique_ptr<GDSIINode>> nodes;
        std::vector<std::unique_ptr<GDSIIBox>> boxes;

        auto toString(std::size_t indent = 0) const -> std::string;
        auto toString(std::stringstream& ss, std::size_t indent = 0) const -> void;
    };

}