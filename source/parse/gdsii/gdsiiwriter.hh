#pragma once

#include "gdsiilayout.hh"
#include "gdsiirecord.hh"
#include "gdsiistructrue.hh"
#include <fstream>
#include <filesystem>

namespace xtaro::parse
{

    class GDSIIWriter 
    {
    public:
        GDSIIWriter(const std::filesystem::path& filepath);

    public:
        auto write(const GDSIILayout& layout) -> void;

    private:
        auto writeHeader(const GDSIILibrary& info) -> void;

    private:
        auto writeLibrary(const GDSIILayout& gds) -> void;
        auto writeBeginLibrary(const GDSIILibrary& gds) -> void;
        auto writeLibraryName(const GDSIILibrary& gds) -> void;
        auto writeLibraryOptions(const GDSIILibrary& gds) -> void;
        auto writeUnits(const GDSIILibrary& gds) -> void;
        auto writeStructures(const GDSIILayout& gds) -> void;
        auto writeEndLibrary() -> void;

        auto writeRefLibs(const std::array<std::string, 2>& reflibs) -> void;
        auto writeFonts(const std::array<std::string, 4>& fonts) -> void;
        auto writeAttrtable(const std::string& attrtable) -> void;
        auto writeGenerations(std::int16_t generations) -> void;
        auto writeFormat(std::int16_t format) -> void;
        
    private:
        auto writeStructure(GDSIIStructure* structure) -> void;
        auto writeStructureBegin(GDSIIStructure* structure) -> void;
        auto writeStructureName(GDSIIStructure* structure) -> void;
        
        auto writeStructureElements(GDSIIStructure* structure) -> void;
        auto writeStructureEnd(GDSIIStructure* structure) -> void;

        auto writeBoundaryElement(GDSIIBoundary* boundary) -> void;
        auto writePathElement(GDSIIPath* path) -> void;
        auto writeSrefElement(GDSIISref* sref) -> void;
        auto writeArefElement(GDSIIAref* aref) -> void;
        auto writeTextElement(GDSIIText* text) -> void;
        auto writeNodeElement(GDSIINode* node) -> void;
        auto writeBoxElement(GDSIIBox* box) -> void;
        
        auto writeElementEnd() -> void;

    private:
        auto writeELFLags(std::int16_t elfFlags) -> void;
        auto writePlex(std::int32_t plex) -> void;
        auto writeLayer(std::int16_t layer) -> void;
        auto writeDataType(std::int16_t dataType) -> void;
        auto writeXY(const std::vector<GDSIIDBCoord>& coords) -> void;
        auto writePathType(std::int16_t pathType) -> void;
        auto writeWidth(std::int32_t width) -> void;
        auto writeSName(const std::string& sName) -> void;
        auto writeSTrans(const GDSIITransFlag& sTrans) -> void;
        auto writeMag(double mag) -> void;
        auto writeAngle(double angle) -> void;
        auto writeColRow(std::int16_t col, std::int16_t row) -> void;
        auto writeTextType(std::int16_t textType) -> void;
        auto writePresentation(std::int16_t presentation) -> void;
        auto writeASCIIString(const std::string& string) -> void;
        auto writeNodeType(std::int16_t nodeType) -> void;
        auto writeBoxType(std::int16_t boxType) -> void;

    private:
        auto writeRecordInfo(std::int16_t size, GDSIIRecordType type) -> void;

        auto writeInt16Record(GDSIIRecordType type, std::int16_t value) -> void;
        auto writeInt32Record(GDSIIRecordType type, std::int32_t value) -> void;
        auto writeDoubleRecord(GDSIIRecordType type, double value) -> void;
        auto writeStringRecord(GDSIIRecordType type, std::string_view value) -> void;
        auto writeDateTimeRecord(GDSIIRecordType type, const GDSIIDateTime& create, const GDSIIDateTime& modify) -> void;
        auto writeEmptyRecord(GDSIIRecordType type) -> void;

    private:
        auto writeInt16(std::int16_t value) -> void;
        auto writeInt32(std::int32_t value) -> void;
        auto writeString(std::string_view string) -> void;
        auto writeString(std::string_view string, std::size_t size) -> void;
        auto writeDouble(double value) -> void;
        auto writeDateTime(const GDSIIDateTime& date) -> void;

    private:
        std::ofstream _outfile {};
    };

}