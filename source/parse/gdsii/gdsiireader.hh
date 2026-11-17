#pragma once

#include "gdsiilayout.hh"
#include "gdsiirecord.hh"
#include "gdsiistructrue.hh"
#include <memory>
#include <fstream>
#include <filesystem>

namespace xtaro::parse
{

    class GDSIIReader
    {
    public:
        GDSIIReader(const std::filesystem::path& filepath);

    public:
        auto read() -> std::unique_ptr<GDSIILayout>;

    private:
        auto readHeader(GDSIILayout* gds) -> void;

    private:
        auto readLibrary(GDSIILayout* gds) -> void;
        auto readBeginLibrary(GDSIILayout* gds) -> void;
        auto readLibraryName(GDSIILayout* gds) -> void;
        auto readLibraryOptions(GDSIILayout* gds) -> void;
        auto readUnits(GDSIILayout* gds) -> void;
        auto readStructures(GDSIILayout* gds) -> void;
        auto readEndLibrary(GDSIILayout* gds) -> void;

        auto readRefLibs(GDSIILayout* gds) -> void;
        auto readFonts(GDSIILayout* gds) -> void;
        auto readAttrtable(GDSIILayout* gds) -> void;
        auto readGenerations(GDSIILayout* gds) -> void;
        auto readFormat(GDSIILayout* gds) -> void;
        
    private:
        auto readStructure() -> std::shared_ptr<GDSIIStructure>;
        auto readBeginStructure(GDSIIStructure* gds) -> void;
        auto readStructureName(GDSIIStructure* gds) -> void;
        auto readEndStructure(GDSIIStructure* gds) -> void;
        
        auto readElements(GDSIIStructure* gds) -> void;
        auto readElementHeader() -> void;
        auto readElementEnd() -> void;

        auto readBoudary() -> std::unique_ptr<GDSIIBoundary>;
        auto readPath() -> std::unique_ptr<GDSIIPath>;
        auto readSref() -> std::unique_ptr<GDSIISref>;
        auto readAref() -> std::unique_ptr<GDSIIAref>;
        auto readText() -> std::unique_ptr<GDSIIText>;
        auto readNode() -> std::unique_ptr<GDSIINode>;
        auto readBox() -> std::unique_ptr<GDSIIBox>;

    private:
        auto readELFLags() -> std::int16_t;
        auto readPlex() -> std::int32_t;
        auto readLayer() -> std::int16_t;
        auto readDataType() -> std::int16_t;
        auto readXY() -> std::vector<GDSIIDBCoord>;
        auto readPathType() -> std::int16_t;
        auto readWidth() -> std::int32_t;
        auto readSName() -> std::string;
        auto readSTrans() -> GDSIITransFlag;
        auto readMag() -> double;
        auto readAngle() -> double;
        auto readColRow() -> std::tuple<std::int16_t, std::int16_t>;
        auto readTextType() -> std::int16_t;
        auto readPresentation() -> std::int16_t;
        auto readASCIIString() -> std::string;
        auto readNodeType() -> std::int16_t;
        auto readBoxType() -> std::int16_t;
    
    private:
        /// Methods for record
        /// 1. Check the current record
        /// 2. dump value of specified type
        /// 3. Jump this record
        auto readInt16Record() -> std::int16_t;
        auto readInt32Record() -> std::int32_t;
        auto readStringRecord() -> std::string;
        auto readDoubleRecord() -> double;

    private:
        /// Methods for bytes stream
        /// 1. Read the current bytes position
        /// 2. dump value of specified type
        /// 3. Jump this bytes
        auto advanceInt16() -> std::int16_t;
        auto advanceInt32() -> std::int32_t;
        auto advanceString(int size) -> std::string;
        auto advanceDouble() -> double;
        auto advanceDate() -> GDSIIDateTime;

    private:
        /// Methods for bytes stream
        auto peekRecordSize() -> std::int16_t;
        auto peekRecordType() -> GDSIIRecordType;
        
        auto checkRecord(GDSIIRecordType type) -> bool;

        auto ensureRecord(std::int16_t size, GDSIIRecordType type) -> void;
        auto ensureRecordSize(std::int16_t size) -> void;
        auto ensureRecordType(GDSIIRecordType type) -> void;

        auto isElementHeaderRecord(GDSIIRecordType type) -> bool;

        auto jumpBytes(std::streamoff size) -> void;
        
        auto leftBytes() -> std::streamsize;

    private:
        void error(std::string_view info);

    private:
        std::ifstream _infile {};    
    };

}