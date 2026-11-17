#include "gdsiiwriter.hh"

namespace xtaro::parse
{

    GDSIIWriter::GDSIIWriter(const std::filesystem::path& filepath) :
        _outfile{filepath, std::ios::binary}
    {
    }

    auto GDSIIWriter::write(const GDSIILayout& layout) -> void
    {
        this->writeHeader(layout.lib());
        this->writeLibrary(layout);
    }

    auto GDSIIWriter::writeHeader(const GDSIILibrary& info) -> void
    {
        this->writeInt16Record(GDSIIRecordType::HEADER, info.version);
    }

    auto GDSIIWriter::writeLibrary(const GDSIILayout& layout) -> void
    {
        this->writeBeginLibrary(layout.lib());
        this->writeLibraryName(layout.lib());
        this->writeLibraryOptions(layout.lib());
        this->writeUnits(layout.lib());
        this->writeStructures(layout);
        this->writeEndLibrary();
    }

    auto GDSIIWriter::writeBeginLibrary(const GDSIILibrary& info) -> void
    {
        this->writeDateTimeRecord(GDSIIRecordType::BGNLIB, info.createDate, info.modifyDate);
    }

    auto GDSIIWriter::writeLibraryName(const GDSIILibrary& info) -> void
    {
        this->writeStringRecord(GDSIIRecordType::LIBNAME, info.name);
    }
    
    auto GDSIIWriter::writeLibraryOptions(const GDSIILibrary& info) -> void
    {
        if (info.reflibs)
            this->writeRefLibs(info.reflibs.value());

        if (info.fonts)
            this->writeFonts(info.fonts.value());

        if (info.attrtable)
            this->writeAttrtable(info.attrtable.value());

        if (info.generations)
            this->writeGenerations(info.generations.value());

        if (info.format) 
            this->writeFormat(info.format.value());
    }
    
    auto GDSIIWriter::writeRefLibs(const std::array<std::string, 2>& reflibs) -> void
    {
        this->writeRecordInfo(94, GDSIIRecordType::REFLIBS);
        this->writeString(reflibs[0], 45);
        this->writeString(reflibs[1], 45);
    }

    auto GDSIIWriter::writeFonts(const std::array<std::string, 4>& fonts) -> void
    {
        this->writeRecordInfo(4 * 44 + 4, GDSIIRecordType::FONTS);
        this->writeString(fonts[0], 44);
        this->writeString(fonts[1], 44);
        this->writeString(fonts[2], 44);
        this->writeString(fonts[3], 44);
    }
    
    auto GDSIIWriter::writeAttrtable(const std::string& attrtable) -> void
    {
        this->writeRecordInfo(48, GDSIIRecordType::ATTRTABLE);
        this->writeString(attrtable, 44);
    }
    
    auto GDSIIWriter::writeGenerations(std::int16_t generations) -> void
    {
        this->writeInt16Record(GDSIIRecordType::GENERATIONS, generations);
    }
    
    auto GDSIIWriter::writeFormat(std::int16_t format) -> void
    {
        this->writeInt16Record(GDSIIRecordType::FORMAT, format);
    }

    auto GDSIIWriter::writeUnits(const GDSIILibrary& info) -> void
    {   
        this->writeRecordInfo(20, GDSIIRecordType::UINTS);
        this->writeDouble(info.usrunitsPerDBunit);
        this->writeDouble(info.metersPerDBunit);
    }

    auto GDSIIWriter::writeStructures(const GDSIILayout& layout) -> void
    {
        for (const auto& structure : layout.structures()) 
            this->writeStructure(structure.second.get());
    }

    auto GDSIIWriter::writeEndLibrary() -> void
    {
        this->writeEmptyRecord(GDSIIRecordType::ENDLIB);
    }

    auto GDSIIWriter::writeStructure(GDSIIStructure* structure) -> void
    {
        this->writeStructureBegin(structure);
        this->writeStructureName(structure);
        this->writeStructureElements(structure);
        this->writeStructureEnd(structure);
    }

    auto GDSIIWriter::writeStructureBegin(GDSIIStructure* structure) -> void
    {
        this->writeDateTimeRecord(GDSIIRecordType::BGNSTR, structure->createDate, structure->modifyDate);
    }   

    auto GDSIIWriter::writeStructureName(GDSIIStructure* structure) -> void
    {
        this->writeStringRecord(GDSIIRecordType::STRNAME, structure->name);
    }
    
    auto GDSIIWriter::writeStructureElements(GDSIIStructure* structure) -> void
    {
        for (const auto& boundary : structure->boundarys)
            this->writeBoundaryElement(boundary.get());

        for (const auto& path : structure->paths)
            this->writePathElement(path.get());

        for (const auto& sref : structure->srefs)
            this->writeSrefElement(sref.get());

        for (const auto& aref : structure->arefs)
            this->writeArefElement(aref.get());

        for (const auto& text : structure->texts)
            this->writeTextElement(text.get());

        for (const auto& node : structure->nodes)
            this->writeNodeElement(node.get());

        for (const auto& box : structure->boxes)
            this->writeBoxElement(box.get());
    }

    auto GDSIIWriter::writeBoundaryElement(GDSIIBoundary* boundary) -> void
    {
        this->writeEmptyRecord(GDSIIRecordType::BOUNDARY);

        if (boundary->elfFlags) this->writeELFLags(boundary->elfFlags.value());
        if (boundary->plex) this->writePlex(boundary->plex.value());
        this->writeLayer(boundary->layer);
        this->writeDataType(boundary->dataType);
        this->writeXY(boundary->xy);

        this->writeElementEnd();
    }

    auto GDSIIWriter::writePathElement(GDSIIPath* path) -> void
    {
        this->writeEmptyRecord(GDSIIRecordType::PATH);

        if (path->elfFlags) this->writeELFLags(path->elfFlags.value());
        if (path->plex) this->writePlex(path->plex.value());
        this->writeLayer(path->layer);
        this->writeDataType(path->dataType);
        if (path->pathType) this->writePathType(path->pathType.value());
        if (path->width) this->writeWidth(path->width.value());
        this->writeXY(path->xy);

        this->writeElementEnd();
    }

    auto GDSIIWriter::writeSrefElement(GDSIISref* sref) -> void
    {
        this->writeEmptyRecord(GDSIIRecordType::SREF);

        if (sref->elfFlags) this->writeELFLags(sref->elfFlags.value());
        if (sref->plex) this->writePlex(sref->plex.value());
        this->writeSName(sref->sName);
        if (sref->sTrans) this->writeSTrans(sref->sTrans.value());
        if (sref->mag) this->writeMag(sref->mag.value());
        if (sref->angle) this->writeAngle(sref->angle.value());
        this->writeXY(sref->xy);

        this->writeElementEnd();
    }

    auto GDSIIWriter::writeArefElement(GDSIIAref* aref) -> void
    {
        this->writeEmptyRecord(GDSIIRecordType::AREF);

        if (aref->elfFlags) this->writeELFLags(aref->elfFlags.value());
        if (aref->plex) this->writePlex(aref->plex.value());
        this->writeSName(aref->sName);
        if (aref->sTrans) this->writeSTrans(aref->sTrans.value());
        if (aref->mag) this->writeMag(aref->mag.value());
        if (aref->angle) this->writeAngle(aref->angle.value());
        this->writeColRow(aref->col, aref->row);
        this->writeXY(aref->xy);

        this->writeElementEnd();
    }

    auto GDSIIWriter::writeTextElement(GDSIIText* text) -> void
    {   
        this->writeEmptyRecord(GDSIIRecordType::TEXT);

        if (text->elfFlags) this->writeELFLags(text->elfFlags.value());
        if (text->plex) this->writePlex(text->plex.value());  
        this->writeLayer(text->layer);
        this->writeTextType(text->textType);
        if (text->presentation) this->writePresentation(text->presentation.value());
        if (text->sTrans) this->writeSTrans(text->sTrans.value());
        if (text->mag) this->writeMag(text->mag.value());
        if (text->angle) this->writeAngle(text->angle.value());
        if (text->pathType) this->writePathType(text->pathType.value());
        if (text->width) this->writeWidth(text->width.value());
        this->writeXY(text->xy);
        this->writeASCIIString(text->string);

        this->writeElementEnd();
    }

    auto GDSIIWriter::writeNodeElement(GDSIINode* node) -> void 
    {
        this->writeEmptyRecord(GDSIIRecordType::NODE);

        if (node->elfFlags) this->writeELFLags(node->elfFlags.value());
        if (node->plex) this->writePlex(node->plex.value());
        this->writeLayer(node->layer);
        this->writeNodeType(node->nodeType);
        this->writeXY(node->xy);

        this->writeElementEnd();
    }

    auto GDSIIWriter::writeBoxElement(GDSIIBox* box) -> void 
    {
        this->writeEmptyRecord(GDSIIRecordType::BOX);

        if (box->elfFlags) this->writeELFLags(box->elfFlags.value());
        if (box->plex) this->writePlex(box->plex.value());
        this->writeLayer(box->layer);
        this->writeBoxType(box->boxType);
        this->writeXY(box->xy);

        this->writeElementEnd();
    }

    auto GDSIIWriter::writeStructureEnd(GDSIIStructure* structure) -> void
    {
        this->writeEmptyRecord(GDSIIRecordType::ENDSTR);
    }

    auto GDSIIWriter::writeElementEnd() -> void
    {
        this->writeEmptyRecord(GDSIIRecordType::ENDELE);
    }

    auto GDSIIWriter::writeELFLags(std::int16_t elfFlags) -> void
    {
        this->writeInt16Record(GDSIIRecordType::ELFLAGS, elfFlags);
    }

    auto GDSIIWriter::writePlex(std::int32_t plex) -> void
    {
        this->writeInt32Record(GDSIIRecordType::PLEX, plex);
    }

    auto GDSIIWriter::writeLayer(std::int16_t layer) -> void
    {
        this->writeInt16Record(GDSIIRecordType::LAYER, layer);
    }

    auto GDSIIWriter::writeDataType(std::int16_t dataType) -> void
    {
        this->writeInt16Record(GDSIIRecordType::DATATYPE, dataType);
    }

    auto GDSIIWriter::writeXY(const std::vector<GDSIIDBCoord>& coords) -> void
    {
        std::int16_t recordSize = 4 + coords.size() * 8;
        this->writeRecordInfo(recordSize, GDSIIRecordType::XY);
        for (const auto& [x, y] : coords)
        {
            this->writeInt32(x);
            this->writeInt32(y);
        }
    }

    auto GDSIIWriter::writePathType(std::int16_t pathType) -> void
    {
        this->writeInt16Record(GDSIIRecordType::PATHTYPE, pathType);
    }

    auto GDSIIWriter::writeWidth(std::int32_t width) -> void
    {
        this->writeInt32Record(GDSIIRecordType::WIDTH, width);
    }

    auto GDSIIWriter::writeSName(const std::string& sName) -> void
    {
        this->writeStringRecord(GDSIIRecordType::SNAME, sName);
    }

    auto GDSIIWriter::writeSTrans(const GDSIITransFlag& sTrans) -> void
    {
        auto value {std::int16_t{0}};
        if (sTrans.x_mirror) value |= 0x8000;
        if (sTrans.magnify) value |= 0x0004; 
        if (sTrans.rotate) value |= 0x0002; 

        this->writeInt16Record(GDSIIRecordType::STRANS, value);
    }

    auto GDSIIWriter::writeMag(double mag) -> void
    {
        this->writeDoubleRecord(GDSIIRecordType::MAG, mag);
    }

    auto GDSIIWriter::writeAngle(double angle) -> void
    {
        this->writeDoubleRecord(GDSIIRecordType::ANGLE, angle);
    }

    auto GDSIIWriter::writeColRow(std::int16_t col, std::int16_t row) -> void
    {
        this->writeRecordInfo(8, GDSIIRecordType::COLROW);
        this->writeInt16(col);
        this->writeInt16(row);
    }

    auto GDSIIWriter::writeTextType(std::int16_t textType) -> void
    {
        this->writeInt16Record(GDSIIRecordType::TEXTTYPE, textType);
    }

    auto GDSIIWriter::writePresentation(std::int16_t presentation) -> void
    {
        this->writeInt16Record(GDSIIRecordType::PRESENTATION, presentation);
    }

    auto GDSIIWriter::writeASCIIString(const std::string& string) -> void
    {
        this->writeStringRecord(GDSIIRecordType::STRING, string);
    }

    auto GDSIIWriter::writeNodeType(std::int16_t nodeType) -> void
    {
        this->writeInt16Record(GDSIIRecordType::NODETYPE, nodeType);
    }

    auto GDSIIWriter::writeBoxType(std::int16_t boxType) -> void
    {
        this->writeInt16Record(GDSIIRecordType::BOXTYPE, boxType);
    }

    auto GDSIIWriter::writeRecordInfo(std::int16_t size, GDSIIRecordType type) -> void
    {
        this->writeInt16(size);
        this->writeInt16(static_cast<std::int16_t>(type));
    }

    auto GDSIIWriter::writeInt16Record(GDSIIRecordType type, std::int16_t value) -> void
    {
        this->writeRecordInfo(6, type);
        this->writeInt16(value);
    }
    
    auto GDSIIWriter::writeInt32Record(GDSIIRecordType type, std::int32_t value) -> void
    {
        this->writeRecordInfo(8, type);
        this->writeInt32(value);
    }

    auto GDSIIWriter::writeDoubleRecord(GDSIIRecordType type, double value) -> void
    {
        this->writeRecordInfo(12, type);
        this->writeDouble(value);
    }

    auto GDSIIWriter::writeStringRecord(GDSIIRecordType type, std::string_view value) -> void
    {
        std::size_t nameSize {value.size()};
        if (nameSize % 2 != 0) nameSize += 1;
        std::int16_t recordSize = nameSize + 4;

        this->writeRecordInfo(recordSize, type);
        this->writeString(value);
    }

    auto GDSIIWriter::writeDateTimeRecord(GDSIIRecordType type, const GDSIIDateTime& create, const GDSIIDateTime& modify) -> void
    {
        this->writeRecordInfo(28, type);
        this->writeDateTime(create);
        this->writeDateTime(modify);
    }

    auto GDSIIWriter::writeEmptyRecord(GDSIIRecordType type) -> void
    {
        this->writeRecordInfo(4, type);
    }

    auto GDSIIWriter::writeInt16(std::int16_t value) -> void
    {
        const char* ptr {reinterpret_cast<const char*>(&value)};
        this->_outfile.write(ptr + 1, 1);
        this->_outfile.write(ptr, 1);
    }
    
    auto GDSIIWriter::writeInt32(std::int32_t value) -> void
    {
        const char* ptr {reinterpret_cast<const char*>(&value)};
        this->_outfile.write(ptr + 3, 1);
        this->_outfile.write(ptr + 2, 1);
        this->_outfile.write(ptr + 1, 1);
        this->_outfile.write(ptr, 1);
    }
    
    auto GDSIIWriter::writeString(std::string_view string) -> void
    {
        std::int16_t size = string.size();
        this->_outfile.write(string.data(), string.size());

        if (size % 2 != 0) this->_outfile.put('\0');
    }

    auto GDSIIWriter::writeString(std::string_view string, std::size_t size) -> void
    {
        if (string.size() < size) 
        {
            this->_outfile.write(string.data(), string.size());
            std::size_t zero_size {size - string.size()};
            for (std::size_t i = 0; i < zero_size; ++i)
                this->_outfile.put('\0');
        } 
        else
        {
            this->_outfile.write(string.data(), size);
        }
    }
    
    auto GDSIIWriter::writeDouble(double value) -> void
    {
        // Convert from normal IEEE format to IBM 370
        // IBM 370: (1)sign (7)exponent (56)mantissa
        // IEEE:    (1)sign (11)exponent (52)mantissa
        std::int64_t data {
            *(reinterpret_cast<std::int64_t*>(&value))
        };

        int64_t sign { (data >> 63) & 0x1LL };
        int64_t exponent { ((data >> 52) & 0x7FFLL) - 1023LL };
        int64_t mantissa { data << 12 };

        if (value == 0) 
        {
            sign = 0;
            exponent = 0;
            mantissa = 0;
        }
        else
        {
            mantissa >>= 1;
            mantissa = mantissa | 0x8000000000000000LL;
            exponent += 1L;
            
            std::int64_t count { (-1LL * exponent) & 0x3LL };
            for (std::int64_t i {0}; i < count; ++i)
            {
                mantissa >>= 1;
                mantissa = mantissa & 0x7fffffffffffffffLL;
            }

            exponent = (exponent + 3LL) >> 2;
            exponent += 64LL;
        }

        std::int64_t newData {
            (sign << 63) | (exponent << 56) | ((mantissa >> 8) & 0xffffffffffffffLL)
        };
        const char* ptr {reinterpret_cast<const char*>(&newData)};

        // Write Data
        for (int i = 7; i >= 0; --i)
            this->_outfile.write(ptr + i, 1);
    }

    auto GDSIIWriter::writeDateTime(const GDSIIDateTime& date) -> void
    {
        this->writeInt16(date.year);
        this->writeInt16(date.month);
        this->writeInt16(date.day);
        this->writeInt16(date.hour);
        this->writeInt16(date.minute);
        this->writeInt16(date.second);
    }
}