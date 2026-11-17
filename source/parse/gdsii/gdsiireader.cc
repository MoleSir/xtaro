#include "gdsiireader.hh"
#include "gdsiilayout.hh"
#include "gdsiirecord.hh"
#include "gdsiiprimitives.hh"
#include "utility/debug/debug.hh"
#include <exception>
#include <format>

namespace xtaro::parse
{

    #define TRY try

    #define THROW_EXCETPION_WITH_CONTEXT(context)\
    catch (const std::exception& err) {\
        debug::exceptionIn(context, err.what());\
    }

    GDSIIReader::GDSIIReader(const std::filesystem::path& filepath) :
        _infile{filepath, std::ios::binary}
    {
    }

    auto GDSIIReader::read() -> std::unique_ptr<GDSIILayout>
    TRY
    {
        auto gds = std::make_unique<GDSIILayout>();
        this->readHeader(gds.get());
        this->readLibrary(gds.get());
        gds->initialize();
        
        return gds;
    }
    THROW_EXCETPION_WITH_CONTEXT("Parse gdsii")

    auto GDSIIReader::readHeader(GDSIILayout* gds) -> void
    TRY
    {
        this->ensureRecord(6, GDSIIRecordType::HEADER);
        gds->lib().version = this->readInt16Record();
    }
    THROW_EXCETPION_WITH_CONTEXT("Read Header")

    auto GDSIIReader::readLibrary(GDSIILayout* gds) -> void
    TRY
    {
        this->readBeginLibrary(gds);
        this->readLibraryName(gds);

        // REFLIBS \ FONTS \ ATTRTABLE \ GENERATIONS \ FORMAT
        this->readLibraryOptions(gds);
        this->readUnits(gds);

        this->readStructures(gds);

        this->readEndLibrary(gds);
    }
    THROW_EXCETPION_WITH_CONTEXT("Read library")

    auto GDSIIReader::readBeginLibrary(GDSIILayout* gds) -> void
    TRY
    {
        this->ensureRecord(28, GDSIIRecordType::BGNLIB);
        this->jumpBytes(4);

        gds->lib().createDate = this->advanceDate();
        gds->lib().modifyDate = this->advanceDate();
    }
    THROW_EXCETPION_WITH_CONTEXT("Read library begin")

    auto GDSIIReader::readLibraryName(GDSIILayout* gds) -> void
    TRY
    {
        this->ensureRecordType(GDSIIRecordType::LIBNAME);
        gds->lib().name = (this->readStringRecord());
    }
    THROW_EXCETPION_WITH_CONTEXT("Read library name")

    auto GDSIIReader::readLibraryOptions(GDSIILayout* gds) -> void
    TRY
    {
        while (true)
        {
            GDSIIRecordType type {this->peekRecordType()};
            switch (type)
            {
            case GDSIIRecordType::REFLIBS:
                this->readRefLibs(gds); break; 
            case GDSIIRecordType::FONTS:
                this->readFonts(gds); break;
            case GDSIIRecordType::ATTRTABLE:
                this->readAttrtable(gds); break;
            case GDSIIRecordType::GENERATIONS:
                this->readGenerations(gds); break;
            case GDSIIRecordType::FORMAT:
                this->readFormat(gds); break;
            default: return;
            }
        }
    }
    THROW_EXCETPION_WITH_CONTEXT("Read library options")

    auto GDSIIReader::readRefLibs(GDSIILayout* gds) -> void
    TRY
    {
        this->ensureRecord(94, GDSIIRecordType::REFLIBS);
        this->jumpBytes(4);
        gds->lib().reflibs = std::array<std::string, 2> {
            this->advanceString(45),
            this->advanceString(45),
        };
    }
    THROW_EXCETPION_WITH_CONTEXT("Read library refs")

    auto GDSIIReader::readFonts(GDSIILayout* gds) -> void
    TRY
    {
        this->ensureRecordSize(4 * 44 + 4);
        this->jumpBytes(4);
        gds->lib().fonts = std::array<std::string, 4> {
            this->advanceString(44),
            this->advanceString(44),
            this->advanceString(44),
            this->advanceString(44),
        };
    }
    THROW_EXCETPION_WITH_CONTEXT("Read library fonts")
    
    auto GDSIIReader::readAttrtable(GDSIILayout* gds) -> void
    TRY
    {
        this->ensureRecordSize(48);
        this->jumpBytes(4);
        gds->lib().attrtable = this->advanceString(44);
    }
    THROW_EXCETPION_WITH_CONTEXT("Read library attrtable")

    auto GDSIIReader::readGenerations(GDSIILayout* gds) -> void
    TRY
    {
        gds->lib().generations = this->readInt16Record();
    }
    THROW_EXCETPION_WITH_CONTEXT("Read library generations")

    auto GDSIIReader::readFormat(GDSIILayout* gds) -> void
    TRY
    {
        gds->lib().format = this->readInt16Record();
    }
    THROW_EXCETPION_WITH_CONTEXT("Read library fotmat")

    auto GDSIIReader::readUnits(GDSIILayout* gds) -> void
    TRY
    {
        this->ensureRecord(20, GDSIIRecordType::UINTS);
        this->jumpBytes(4);
        gds->lib().usrunitsPerDBunit = this->advanceDouble();
        gds->lib().metersPerDBunit = this->advanceDouble();
    }
    THROW_EXCETPION_WITH_CONTEXT("Read units")

    auto GDSIIReader::readEndLibrary(GDSIILayout* gds) -> void
    TRY
    {
        this->ensureRecord(4, GDSIIRecordType::ENDLIB);
        this->jumpBytes(4);
    }
    THROW_EXCETPION_WITH_CONTEXT("Read library end")

    auto GDSIIReader::readStructures(GDSIILayout* gds) -> void
    TRY
    {
        while (this->checkRecord(GDSIIRecordType::BGNSTR))
        {
            auto structure { this->readStructure() };   
            auto name { structure->name };  
            gds->insertStructures(std::move(name), std::move(structure));
        }
    }
    THROW_EXCETPION_WITH_CONTEXT("Read structures")

    auto GDSIIReader::readStructure() -> std::shared_ptr<GDSIIStructure>
    {
        auto structure { std::make_shared<GDSIIStructure>() };
        TRY
        {
            this->readBeginStructure(structure.get());
            this->readStructureName(structure.get());
        }
        THROW_EXCETPION_WITH_CONTEXT("Read structure")
        
        TRY 
        {
            this->readElements(structure.get());

            this->readEndStructure(structure.get());

            return structure;
        }
        THROW_EXCETPION_WITH_CONTEXT(std::format("Read structure '{}'", structure->name))
    }

    auto GDSIIReader::readBeginStructure(GDSIIStructure* structure) -> void
    TRY
    {
        this->ensureRecord(28, GDSIIRecordType::BGNSTR);
        this->jumpBytes(4);

        structure->modifyDate = this->advanceDate();
        structure->createDate = this->advanceDate();
    }
    THROW_EXCETPION_WITH_CONTEXT("Read begin structure")

    
    auto GDSIIReader::readStructureName(GDSIIStructure* structure) -> void
    TRY
    {
        this->ensureRecordType(GDSIIRecordType::STRNAME);
        structure->name.assign(this->readStringRecord());
    }
    THROW_EXCETPION_WITH_CONTEXT("Read structure name")

    auto GDSIIReader::readElements(GDSIIStructure* structure) -> void
    TRY
    {
        while (true)
        {
            GDSIIRecordType type {this->peekRecordType()};
            if (!this->isElementHeaderRecord(type))
                break;

            switch (type)
            {
            case GDSIIRecordType::BOUNDARY:
                structure->boundarys.push_back(this->readBoudary()); break;
            case GDSIIRecordType::PATH :
                structure->paths.push_back(this->readPath()); break;
            case GDSIIRecordType::SREF:
                structure->srefs.push_back(this->readSref()); break;
            case GDSIIRecordType::AREF:
                structure->arefs.push_back(this->readAref()); break;
            case GDSIIRecordType::TEXT:
                structure->texts.push_back(this->readText()); break;
            case GDSIIRecordType::NODE:
                structure->nodes.push_back(this->readNode()); break;
            case GDSIIRecordType::BOX:
                structure->boxes.push_back(this->readBox()); break;
            default: std::exit(1); // MARK
            }
        }
    }
    THROW_EXCETPION_WITH_CONTEXT("Read structure elements")

    auto GDSIIReader::readEndStructure(GDSIIStructure* gds) -> void
    TRY
    {
        this->ensureRecord(4, GDSIIRecordType::ENDSTR);
        this->jumpBytes(4);
    }
    THROW_EXCETPION_WITH_CONTEXT("Read end structure")

    auto GDSIIReader::readElementHeader() -> void
    TRY
    {
        this->ensureRecordSize(4);
        this->jumpBytes(4);
    }
    THROW_EXCETPION_WITH_CONTEXT("Read element header")

    auto GDSIIReader::readElementEnd() -> void
    TRY
    {
        this->ensureRecord(4, GDSIIRecordType::ENDELE);
        this->jumpBytes(4);
    }
    THROW_EXCETPION_WITH_CONTEXT("Read structure end")

    auto GDSIIReader::readBoudary() -> std::unique_ptr<GDSIIBoundary>
    TRY
    {
        this->readElementHeader();

        auto boudary { std::make_unique<GDSIIBoundary>() };
        while (true)
        {
            GDSIIRecordType type {this->peekRecordType()};            
            switch (type)
            {
            case GDSIIRecordType::ELFLAGS: boudary->elfFlags.emplace(this->readELFLags()); break;
            case GDSIIRecordType::PLEX: boudary->plex.emplace(this->readPlex()); break;
            case GDSIIRecordType::LAYER: boudary->layer = this->readLayer(); break;
            case GDSIIRecordType::DATATYPE: boudary->dataType = this->readDataType(); break;
            case GDSIIRecordType::XY: boudary->xy = this->readXY(); break;
            default: this->readElementEnd(); return boudary;
            }
        }

        return nullptr;
    }
    THROW_EXCETPION_WITH_CONTEXT("Read boundary")

    auto GDSIIReader::readPath() -> std::unique_ptr<GDSIIPath>
    TRY
    {
        this->readElementHeader();

        auto path { std::make_unique<GDSIIPath>() };
        while (true)
        {
            GDSIIRecordType type {this->peekRecordType()};
            switch (type)
            {
            case GDSIIRecordType::ELFLAGS: path->elfFlags.emplace(this->readELFLags()); break;
            case GDSIIRecordType::PLEX: path->plex.emplace(this->readPlex()); break;
            case GDSIIRecordType::LAYER: path->layer = this->readLayer(); break;
            case GDSIIRecordType::TEXTTYPE: path->purposeLayer = this->readTextType(); break;
            case GDSIIRecordType::DATATYPE: path->dataType = this->readDataType(); break;
            case GDSIIRecordType::PATHTYPE: path->pathType.emplace(this->readPathType()); break;
            case GDSIIRecordType::WIDTH: path->width.emplace(this->readWidth()); break;
            // MARK!
            case GDSIIRecordType::XY: path->xy = this->readXY(); break;
            case GDSIIRecordType::BGNEXTN: this->readInt32Record(); break;
            case GDSIIRecordType::ENDEXTN: this->readInt32Record(); break;
            default: this->readElementEnd(); return path;
            }
        }

        return nullptr;
    }
    THROW_EXCETPION_WITH_CONTEXT("Read path")

    auto GDSIIReader::readSref() -> std::unique_ptr<GDSIISref>
    TRY
    {
        this->readElementHeader();

        auto sref { std::make_unique<GDSIISref>() };
        while (true)
        {
            GDSIIRecordType type {this->peekRecordType()};
            switch (type)
            {
            case GDSIIRecordType::ELFLAGS: sref->elfFlags.emplace(this->readELFLags()); break;
            case GDSIIRecordType::PLEX: sref->plex.emplace(this->readPlex()); break;
            case GDSIIRecordType::SNAME: sref->sName = this->readSName(); break;
            case GDSIIRecordType::STRANS: sref->sTrans.emplace(this->readSTrans()); break;
            case GDSIIRecordType::MAG: sref->mag.emplace(this->readMag()); break;
            case GDSIIRecordType::ANGLE: sref->angle.emplace(this->readAngle()); break;
            case GDSIIRecordType::XY: sref->xy = this->readXY(); break;
            default: this->readElementEnd(); return sref;
            }
        }

        return nullptr;
    }
    THROW_EXCETPION_WITH_CONTEXT("Read sref")

    auto GDSIIReader::readAref() -> std::unique_ptr<GDSIIAref>
    TRY
    {
        this->readElementHeader();

        auto aref { std::make_unique<GDSIIAref>() };
        while (true)
        {
            GDSIIRecordType type {this->peekRecordType()};
            switch (type)
            {
            case GDSIIRecordType::ELFLAGS: aref->elfFlags.emplace(this->readELFLags()); break;
            case GDSIIRecordType::PLEX: aref->plex.emplace(this->readPlex()); break;
            case GDSIIRecordType::SNAME: aref->sName = this->readSName(); break;
            case GDSIIRecordType::STRANS: aref->sTrans.emplace(this->readSTrans()); break;
            case GDSIIRecordType::MAG: aref->mag.emplace(this->readMag()); break;
            case GDSIIRecordType::ANGLE: aref->angle.emplace(this->readAngle()); break;
            case GDSIIRecordType::COLROW: {
                auto [col, row] { this->readColRow() };
                aref->col = col;
                aref->row = row;
                break;
            }
            case GDSIIRecordType::XY: aref->xy = this->readXY(); break;
            default: this->readElementEnd(); return aref;
            }
        }

        return nullptr;
    }
    THROW_EXCETPION_WITH_CONTEXT("Read aref")

    auto GDSIIReader::readText() -> std::unique_ptr<GDSIIText>
    TRY
    {
        this->readElementHeader();

        auto text { std::make_unique<GDSIIText>() };
        while (true)
        {
            GDSIIRecordType type {this->peekRecordType()};
            switch (type)
            {
            case GDSIIRecordType::ELFLAGS: text->elfFlags.emplace(this->readELFLags()); break;
            case GDSIIRecordType::PLEX: text->plex.emplace(this->readPlex()); break;
            case GDSIIRecordType::LAYER: text->layer = this->readLayer(); break;
            case GDSIIRecordType::TEXTTYPE: text->textType = this->readTextType(); break;
            case GDSIIRecordType::PRESENTATION: text->presentation.emplace(this->readPresentation()); break;
            case GDSIIRecordType::PATHTYPE: text->pathType.emplace(this->readPathType()); break;
            case GDSIIRecordType::WIDTH: text->width.emplace(this->readWidth()); break;
            case GDSIIRecordType::STRANS: text->sTrans.emplace(this->readSTrans()); break;
            case GDSIIRecordType::MAG: text->mag.emplace(this->readMag()); break;
            case GDSIIRecordType::ANGLE: text->angle.emplace(this->readAngle()); break;
            case GDSIIRecordType::XY: text->xy = this->readXY(); break;
            case GDSIIRecordType::STRING: text->string = this->readStringRecord(); break;
            default: this->readElementEnd(); return text;
            }
        }

        return nullptr;
    }
    THROW_EXCETPION_WITH_CONTEXT("Read text")

    auto GDSIIReader::readNode() -> std::unique_ptr<GDSIINode>
    TRY
    {
        this->readElementHeader();

        auto node { std::make_unique<GDSIINode>() };
        while (true)
        {
            GDSIIRecordType type {this->peekRecordType()};            
            switch (type)
            {
            case GDSIIRecordType::ELFLAGS: node->elfFlags.emplace(this->readELFLags()); break;
            case GDSIIRecordType::PLEX: node->plex.emplace(this->readPlex()); break;
            case GDSIIRecordType::LAYER: node->layer = this->readLayer(); break;
            case GDSIIRecordType::NODETYPE: node->nodeType = this->readDataType(); break;
            case GDSIIRecordType::XY: node->xy = this->readXY(); break;
            default: this->readElementEnd(); return node;
            }
        }

        return nullptr;
    }
    THROW_EXCETPION_WITH_CONTEXT("Read node")

    auto GDSIIReader::readBox() -> std::unique_ptr<GDSIIBox>
    TRY
    {
        this->readElementHeader();

        auto box { std::make_unique<GDSIIBox>() };
        while (true)
        {
            GDSIIRecordType type {this->peekRecordType()};            
            switch (type)
            {
            case GDSIIRecordType::ELFLAGS: box->elfFlags.emplace(this->readELFLags()); break;
            case GDSIIRecordType::PLEX: box->plex.emplace(this->readPlex()); break;
            case GDSIIRecordType::LAYER: box->layer = this->readLayer(); break;
            case GDSIIRecordType::BOXTYPE: box->boxType = this->readDataType(); break;
            case GDSIIRecordType::XY: box->xy = this->readXY(); break;
            default: this->readElementEnd(); return box;
            }
        }

        return nullptr;
    }
    THROW_EXCETPION_WITH_CONTEXT("Read box")

    auto GDSIIReader::readELFLags() -> std::int16_t
    {
        return this->readInt16Record();
    }

    auto GDSIIReader::readPlex() -> std::int32_t
    {
        return this->readInt32Record();
    }

    auto GDSIIReader::readLayer() -> std::int16_t
    {
       return this->readInt16Record();
    }

    auto GDSIIReader::readDataType() -> std::int16_t
    {
        return this->readInt16Record();
    }

    auto GDSIIReader::readXY() -> std::vector<GDSIIDBCoord>
    {
        std::int16_t recordSize {this->advanceInt16()};
        this->jumpBytes(2);
        std::int16_t positionSize = (recordSize - 4) / 8;
        std::vector<GDSIIDBCoord> coords {};
        for (std::int16_t i = 0; i < positionSize; ++i)
        {
            std::int32_t x {this->advanceInt32()};
            std::int32_t y {this->advanceInt32()};
            coords.emplace_back(x, y);
        }

        return coords;
    }

    auto GDSIIReader::readPathType() -> std::int16_t
    {
        return this->readInt16Record();
    }

    auto GDSIIReader::readWidth() -> std::int32_t
    {
        return this->readInt32Record();
    }

    auto GDSIIReader::readSName() -> std::string
    {
        return this->readStringRecord();
    }

    auto GDSIIReader::readSTrans() -> GDSIITransFlag
    {
        auto value {this->readInt16Record()};
        return GDSIITransFlag {
            .x_mirror = (value & 0x8000) != 0,
            .magnify = (value & 0x0004) != 0,
            .rotate = (value & 0x0002) != 0,  
        };
    }

    auto GDSIIReader::readMag() -> double
    {
        return this->readDoubleRecord();
    }

    auto GDSIIReader::readAngle() -> double
    {
        return this->readDoubleRecord();
    }
    
    auto GDSIIReader::readColRow() -> std::tuple<std::int16_t, std::int16_t>
    {
        this->ensureRecordSize(8);
        this->jumpBytes(4);
        auto col {this->advanceInt16()};
        auto row {this->advanceInt16()};
        return {col, row};
    }

    auto GDSIIReader::readTextType() -> std::int16_t
    {
        return this->readInt16Record();
    }

    auto GDSIIReader::readPresentation() -> std::int16_t
    {
        return this->readInt16Record();
    }

    auto GDSIIReader::readASCIIString() -> std::string
    {
        return this->readStringRecord();
    }

    auto GDSIIReader::readNodeType() -> std::int16_t
    {
        return this->readInt16Record();
    }

    auto GDSIIReader::readBoxType() -> std::int16_t
    {
        return this->readInt16Record();
    }

    auto GDSIIReader::readInt16Record() -> std::int16_t
    {
        this->ensureRecordSize(6);
        this->jumpBytes(4);
        return this->advanceInt16();
    }

    auto GDSIIReader::readInt32Record() -> std::int32_t
    {
        this->ensureRecordSize(8);
        this->jumpBytes(4);
        return this->advanceInt32();
    }
    
    auto GDSIIReader::readStringRecord() -> std::string
    {
        std::int16_t size {this->advanceInt16()};
        this->jumpBytes(2);
        return this->advanceString(size - 4);
    }
    
    auto GDSIIReader::readDoubleRecord() -> double
    {
        this->ensureRecordSize(12);
        this->jumpBytes(4);
        return this->advanceDouble();
    }

    auto GDSIIReader::advanceInt16() -> std::int16_t
    {
        std::int16_t res {0};
        char* ptr {reinterpret_cast<char*>(&res)};
        this->_infile.read(ptr + 1, 1);
        this->_infile.read(ptr, 1);
        return res;
    }

    auto GDSIIReader::advanceInt32() -> std::int32_t
    {
        std::int32_t res {0};
        char* ptr {reinterpret_cast<char*>(&res)};
        this->_infile.read(ptr + 3, 1);
        this->_infile.read(ptr + 2, 1);
        this->_infile.read(ptr + 1, 1);
        this->_infile.read(ptr, 1);
        return res;
    }

    auto GDSIIReader::advanceString(int size) -> std::string
    {
        auto bytes { std::vector<char>(size + 1, 0) };
        this->_infile.read(bytes.data(), size);          
        return std::string{bytes.data()};
    }

    auto GDSIIReader::advanceDouble() -> double
    {
        // Read bytes
        std::int64_t data {0};
        char* bytes {reinterpret_cast<char*>(&data)};
        for (int i = 7; i >= 0; --i)
            this->_infile.read(bytes + i, 1);

        // Convert from IBM 370 format to normal IEEE
        // IBM 370: (1)sign (7)exponent (56)mantissa
        // IEEE:    (1)sign (11)exponent (52)mantissa
        int64_t sign { (data >> 63) & 0x1LL };
        int64_t exponent { (data >> 56) & 0x7FLL };
        int64_t mantissa { data << 8 };

        if (mantissa == 0) 
            return 0.0;

        exponent = ((exponent - 64LL) * 4LL) + 1023LL;
        while ((mantissa & 0x8000000000000000LL) == 0)
        {
            mantissa <<= 1;
            exponent -= 0x1LL;
        }
        mantissa <<= 1;
        exponent -= 0x1LL;

        std::int64_t newData {
            (sign << 63) | (exponent << 52) | ((mantissa >> 12) & 0xfffffffffffffLL)
        };
        
        return *reinterpret_cast<double*>(&newData);
    }

    auto GDSIIReader::advanceDate() -> GDSIIDateTime
    {
        std::int16_t year {this->advanceInt16()};
        std::int16_t month {this->advanceInt16()};
        std::int16_t day {this->advanceInt16()};
        std::int16_t hour {this->advanceInt16()};
        std::int16_t minute {this->advanceInt16()};
        std::int16_t second {this->advanceInt16()};

        return GDSIIDateTime {
            .year = year,
            .month = month,
            .day = day,
            .hour = hour,
            .minute = minute,
            .second = second,
        };
    }

    auto GDSIIReader::peekRecordSize() -> std::int16_t
    {
        std::int16_t size {this->advanceInt16()};
        this->jumpBytes(-2);
        return size;
    }

    auto GDSIIReader::peekRecordType() -> GDSIIRecordType
    {
        this->jumpBytes(2);
        GDSIIRecordType type {
            static_cast<GDSIIRecordType>(this->advanceInt16())
        };
        this->jumpBytes(-4);
        return type;
    }

    auto GDSIIReader::checkRecord(GDSIIRecordType type) -> bool
    {
        return this->peekRecordType() == type;
    }

    auto GDSIIReader::ensureRecord(std::int16_t size, GDSIIRecordType type) -> void
    TRY
    {
        this->ensureRecordType(type);
        this->ensureRecordSize(size);
    }
    THROW_EXCETPION_WITH_CONTEXT(std::format("Ensure record size: '{}' type: '{:X}' failed", size, int(type)));

    auto GDSIIReader::ensureRecordSize(std::int16_t size) -> void
    {
        std::int16_t realSize {this->peekRecordSize()};
        if (realSize != size)
            debug::exceptionf(
                "Except record size '{}' but got '{}'", 
                size,
                realSize
            );
    }

    auto GDSIIReader::ensureRecordType(GDSIIRecordType type) -> void
    {
        if (this->peekRecordType() != type)
            debug::exceptionf(
                "Except record type '{:X}' but got '{:X}'", 
                static_cast<int>(type), 
                static_cast<int>(this->peekRecordType())
            );
    }

    auto GDSIIReader::isElementHeaderRecord(GDSIIRecordType type) -> bool
    {
        return 
            type == GDSIIRecordType::BOUNDARY || 
            type == GDSIIRecordType::PATH || 
            type == GDSIIRecordType::SREF || 
            type == GDSIIRecordType::AREF || 
            type == GDSIIRecordType::TEXT || 
            type == GDSIIRecordType::NODE || 
            type == GDSIIRecordType::BOX;
    }

    auto GDSIIReader::jumpBytes(std::streamoff size) -> void
    {
        this->_infile.seekg(size, std::ios::cur);
    }

    auto GDSIIReader::error(std::string_view info) -> void
    {
        // 
    }

    auto GDSIIReader::leftBytes() -> std::streamsize 
    {
        std::streampos begin {this->_infile.tellg()};
        this->_infile.seekg(0, std::ios::end);
        std::streampos end {this->_infile.tellg()};
        this->_infile.seekg(begin, std::ios::beg);
        return end - begin;
    }
}