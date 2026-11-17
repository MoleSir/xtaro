#pragma once

#include "./gdsiicoord.hh"
#include <optional>
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <ctime>
#include <sstream>

namespace xtaro::parse
{

    struct GDSIITransFlag 
    {
        bool x_mirror {false};
        bool magnify {false};
        bool rotate {false};
    };

    struct GDSIIDateTime
    {
        std::int16_t year;
        std::int16_t month;
        std::int16_t day;
        std::int16_t hour;
        std::int16_t minute;
        std::int16_t second;

        static auto now() -> GDSIIDateTime;

        auto toString(std::size_t indent = 0) const -> std::string;
        auto toString(std::stringstream& ss, std::size_t indent = 0) const -> void;
    };

    struct GDSIIBoundary 
    {
        GDSIIBoundary() = default;
        GDSIIBoundary(std::int16_t layer, std::vector<GDSIIDBCoord> xy);
        
        auto toString(std::size_t indent = 0) const -> std::string;
        auto toString(std::stringstream& ss, std::size_t indent = 0) const -> void;

        std::optional<std::int16_t> elfFlags {};
        std::optional<std::int32_t> plex {};
        std::int16_t layer {-1};
        std::int16_t dataType {0};
        std::vector<GDSIIDBCoord> xy {};
    };

    struct GDSIIPath 
    {
        GDSIIPath() = default;
        GDSIIPath(std::int16_t layer, std::vector<GDSIIDBCoord> coords, std::int32_t width);

        auto toString(std::size_t indent = 0) const -> std::string;
        auto toString(std::stringstream& ss, std::size_t indent = 0) const -> void;

        std::optional<std::int16_t> elfFlags {};
        std::optional<std::int32_t> plex {};
        std::int16_t layer {-1};
        std::optional<std::int16_t> purposeLayer {};
        std::int16_t dataType {0};
        std::optional<std::int16_t> pathType {};
        std::optional<std::int32_t> width {};
        std::vector<GDSIIDBCoord> xy {};
    };

    enum class GDSIIMirror
    {
        MirrorX,
        MirrorY,
        MirrorXY,
    };

    struct GDSIISref 
    {
        GDSIISref() = default;
        GDSIISref(std::string sName, const GDSIIDBCoord& coord);

        auto toString(std::size_t indent = 0) const -> std::string;
        auto toString(std::stringstream& ss, std::size_t indent = 0) const -> void;

        std::optional<std::int16_t> elfFlags {};
        std::optional<std::int32_t> plex {};
        std::string sName {};
        std::optional<GDSIITransFlag> sTrans {};
        std::optional<double> mag {};
        std::optional<double> angle {};
        std::vector<GDSIIDBCoord> xy {};
    };

    struct GDSIIAref 
    {
        GDSIIAref() = default;
        GDSIIAref(std::string sName, std::int16_t col, std::int16_t row, const GDSIIDBCoord& coord);

        auto toString(std::size_t indent = 0) const -> std::string;
        auto toString(std::stringstream& ss, std::size_t indent = 0) const -> void;

        std::optional<std::int16_t> elfFlags {};
        std::optional<std::int32_t> plex {};
        std::string sName {};
        std::optional<GDSIITransFlag> sTrans {};
        std::optional<double> mag {};
        std::optional<double> angle {};
        std::int16_t col {-1};
        std::int16_t row {-1};
        std::vector<GDSIIDBCoord> xy {};
    };

    struct GDSIIText 
    {
        GDSIIText() = default;
        GDSIIText(std::int16_t layer, const GDSIIDBCoord& coord, std::string text);

        auto toString(std::size_t indent = 0) const -> std::string;
        auto toString(std::stringstream& ss, std::size_t indent = 0) const -> void;

        std::optional<std::int16_t> elfFlags {};
        std::optional<std::int32_t> plex {};
        std::int16_t layer {-1};
        std::int16_t textType {0};
        std::optional<std::int16_t> presentation {};
        std::optional<std::int16_t> pathType {};
        std::optional<std::int32_t> width {};
        std::optional<GDSIITransFlag> sTrans {};
        std::optional<double> mag {};
        std::optional<double> angle {};
        std::vector<GDSIIDBCoord> xy {};
        std::string string {};
    };

    struct GDSIINode
    {
        GDSIINode() = default;
        GDSIINode(std::int16_t layer, const GDSIIDBCoord& coord);

        auto toString(std::size_t indent = 0) const -> std::string;
        auto toString(std::stringstream& ss, std::size_t indent = 0) const -> void;

        std::optional<std::int16_t> elfFlags {};
        std::optional<std::int32_t> plex {};
        std::int16_t layer {-1};
        std::int16_t nodeType {0};
        std::vector<GDSIIDBCoord> xy {};
    };

    struct GDSIIBox
    {
        GDSIIBox() = default;
        GDSIIBox(std::int16_t layer, const GDSIIDBCoord& coord);

        auto toString(std::size_t indent = 0) const -> std::string;
        auto toString(std::stringstream& ss, std::size_t indent = 0) const -> void;

        std::optional<std::int16_t> elfFlags {};
        std::optional<std::int32_t> plex {};
        std::int16_t layer {-1};
        std::int16_t boxType {0};
        std::vector<GDSIIDBCoord> xy {};
    };

    struct GDSIILibrary
    {
        std::int16_t version {-1};
        GDSIIDateTime createDate {GDSIIDateTime::now()};
        GDSIIDateTime modifyDate {GDSIIDateTime::now()};

        std::string name {"MYLIB.lib"};

        std::optional<std::array<std::string, 2>> reflibs {};
        std::optional<std::array<std::string, 4>> fonts {};
        std::optional<std::string> attrtable {};
        std::optional<std::int16_t> generations {};
        std::optional<std::int16_t> format {};

        double usrunitsPerDBunit {0.001};
        double metersPerDBunit {5e-10};
    };

}