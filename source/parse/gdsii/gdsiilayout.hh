#pragma once

#include "gdsiicoord.hh"
#include "gdsiiprimitives.hh"
#include "gdsiistructrue.hh"
#include "gdsiitransform.hh"

#include <cstdint>
#include <memory>
#include <optional>
#include <tuple>
#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include <filesystem>

namespace xtaro::parse
{

    class GDSIIReader;

    class GDSIILayout 
    {
        friend GDSIIReader;
    public:
        GDSIILayout();
        GDSIILayout(std::string name);

        auto version(std::int16_t v) -> GDSIILayout&;
        auto libname(std::string name) -> GDSIILayout&;
        auto reflib(std::array<std::string, 2> libs) -> GDSIILayout&;
        auto fonts(std::array<std::string, 4> fs) -> GDSIILayout&;
        auto usrunitsPerDBunit(double v) -> GDSIILayout&;
        auto metersPerDBunit(double v) -> GDSIILayout&;

    public:
        /// Load or Save Layout Object
        static auto loadFromFile(const std::filesystem::path& filepath) -> std::unique_ptr<GDSIILayout>;
        void writeGDSII(const std::filesystem::path& filepath) const;
        void writeTxt(const std::filesystem::path& filepath) const;

    public:
        /// Build Layout Object
        void addInstance(const GDSIILayout& layout, const GDSIIMicroCoord& offset, std::optional<GDSIIMirror> mirror = std::nullopt, std::optional<double> angle = std::nullopt);
        void addInstance(const std::unique_ptr<GDSIILayout>& layout, const GDSIIMicroCoord& offset, std::optional<GDSIIMirror> mirror = std::nullopt, std::optional<double> angle = std::nullopt);

        void addRectangle(std::int16_t layer, const GDSIIMicroCoord& offset, double width, double height);        
        void addPath(std::int16_t layer, const std::vector<GDSIIMicroCoord>& coords, double width);
        void addText(std::int16_t layer, const GDSIIMicroCoord& offset, std::string text);

        void insertStructures(std::string name, std::shared_ptr<GDSIIStructure> structure);

    public:
        auto measureSize() const -> std::tuple<double, double>;
        auto measureLayerSize(std::int16_t layer) const -> std::tuple<double, double>;
        auto pinInfo(const std::string& pinName) const -> const std::vector<std::tuple<std::int16_t, GDSIIMicroRect>>&;

    public:
        /// Attributes for Layout Object
        auto lib() -> GDSIILibrary& { return *(this->_lib); }
        auto lib() const -> const GDSIILibrary& { return *(this->_lib); }

        auto structures() -> std::unordered_map<std::string, std::shared_ptr<GDSIIStructure>>& { return this->_structures; }
        auto structures() const -> const std::unordered_map<std::string, std::shared_ptr<GDSIIStructure>>& { return this->_structures; }

        auto toString(std::size_t indent = 0) const -> std::string;
        auto toString(std::stringstream& ss, std::size_t indent = 0) const -> void;

    private:
        auto getStrcutureTransformInfos() const -> std::vector<transform::Info>;
        auto doGetStrcutureTransformInfos(
            std::string structName,
            const GDSIIDBCoord& offset,
            double angle,
            double mag,
            std::vector<transform::Matrix>& transformMatrixs,
            std::vector<transform::Info>& transformInfos
        ) const -> void;

        auto measureStructureSize(const std::string& sName, const transform::Matrix& matrix) const -> std::optional<transform::Rect>;
        auto measureStructureSizeInLayer(std::int16_t layer, const std::string& sName, const transform::Matrix& matrix) const -> std::optional<transform::Rect>;

        void initialize();
        auto assignRootStructure() -> void;
        auto buildPinsBoundary() -> void;

    private:
        auto micronToDatabase(double microns) const -> std::int32_t;
        auto databaseToMicron(std::int32_t units) const -> double;
        auto micronCoordToDatabaseCoord(const GDSIIMicroCoord& coord) const -> GDSIIDBCoord;
        auto databseCoordToMicronCoord(const GDSIIDBCoord& coord) const -> GDSIIMicroCoord;

    private:
        std::unique_ptr<GDSIILibrary> _lib;

        std::shared_ptr<GDSIIStructure> _rootStrcutrue;
        std::unordered_map<std::string, std::shared_ptr<GDSIIStructure>> _structures;
        std::unordered_map<std::string, std::vector<std::tuple<std::int16_t, GDSIIMicroRect>>> _pinsInfo;
    };

}