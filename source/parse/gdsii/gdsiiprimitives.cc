#include "gdsiiprimitives.hh"
#include <sstream>
#include <chrono>
#include <ctime>

namespace xtaro::parse
{

    GDSIIBoundary::GDSIIBoundary(std::int16_t layer, std::vector<GDSIIDBCoord> xy) :
        layer{layer},
        xy{std::move(xy)}
    {
    }

    auto GDSIIBoundary::toString(std::stringstream& ss, std::size_t indent) const -> void
    {
        auto addIndent = [&ss] (std::size_t indent) {
            for (std::size_t i = 0; i < indent; ++i)
                ss << "    ";
        };

        addIndent(indent);
        ss << "Boundary Element\n";

        auto attrIndent {indent + 1};

        // For each attr
        if (this->elfFlags) addIndent(attrIndent), ss << "elf_flags: " << this->elfFlags.value() << '\n';
        if (this->plex) addIndent(attrIndent), ss << "plex: " << this->plex.value() << '\n';
        addIndent(attrIndent), ss << "layer: " << this->layer << '\n';
        addIndent(attrIndent), ss << "data_type: " << this->dataType << '\n';

        // auto [x, y] { this->xy.at(0) };
        // ss << "coordinate: [" << x << ", " << y << ']' << '\n';
        addIndent(attrIndent);
        ss << "xy: [";
        for (const auto& [x, y] : this->xy) 
            ss << '(' << x << ", " << y << "), ";
        ss << "]\n";     
    }

    auto GDSIIBoundary::toString(std::size_t indent) const -> std::string
    {
        std::stringstream ss {};
        this->toString(ss, indent);
        return ss.str();
    }

    ///////////////////////////////////////////////////////////////////////////

    GDSIIPath::GDSIIPath(std::int16_t layer, std::vector<GDSIIDBCoord> coords, std::int32_t width) :
        layer{layer},
        xy{std::move(coords)},
        width{width}
    {
    }

    auto GDSIIPath::toString(std::stringstream& ss, std::size_t indent) const -> void
    {
        auto addIndent = [&ss] (std::size_t indent) {
            for (std::size_t i = 0; i < indent; ++i)
                ss << "    ";
        };

        addIndent(indent);
        ss << "Path Element\n";

        auto attrIndent {indent + 1};

        // For each attr
        if (this->elfFlags) addIndent(attrIndent), ss << "elf_flags: " << this->elfFlags.value() << '\n';
        if (this->plex) addIndent(attrIndent), ss << "plex: " << this->plex.value() << '\n';
        
        addIndent(attrIndent), ss << "layer: " << this->layer << '\n';
        if (this->purposeLayer) addIndent(attrIndent), ss << "purpose layer: " << this->purposeLayer.value() << '\n';
        addIndent(attrIndent), ss << "data_type: " << this->dataType << '\n';

        if (this->pathType) addIndent(attrIndent), ss << "path_type: " << this->pathType.value() << '\n';
        if (this->width) addIndent(attrIndent), ss << "width: " << this->width.value() << '\n';

        addIndent(attrIndent);
        ss << "xy: [";
        for (const auto& [x, y] : this->xy) 
            ss << '(' << x << ", " << y << "), ";
        ss << "]\n";     
    }

    auto GDSIIPath::toString(std::size_t indent) const -> std::string
    {
        std::stringstream ss {};
        this->toString(ss, indent);
        return ss.str();
    }

    //////////////////////////////////////////////////////////////////////////

    GDSIISref::GDSIISref(std::string sName, const GDSIIDBCoord& coord) :
        sName{std::move(sName)},
        xy{coord}
    {
    }

    auto GDSIISref::toString(std::stringstream& ss, std::size_t indent) const -> void
    {
        auto addIndent = [&ss] (std::size_t indent) {
            for (std::size_t i = 0; i < indent; ++i)
                ss << "    ";
        };

        addIndent(indent);
        ss << "Sref Element\n";

        auto attrIndent {indent + 1};

        // For each attr
        if (this->elfFlags) addIndent(attrIndent), ss << "elf_flags: " << this->elfFlags.value() << '\n';
        if (this->plex) addIndent(attrIndent), ss << "plex: " << this->plex.value() << '\n';

        addIndent(attrIndent), ss << "s_name: " << this->sName << '\n';
        if (this->sTrans) 
        {
            addIndent(attrIndent); 
            ss << "s_trans:";
            if (this->sTrans.value().x_mirror) ss << " x_mirror";
            if (this->sTrans.value().magnify) ss << " magnify";
            if (this->sTrans.value().rotate) ss << " rotate";
            ss << '\n';
        }

        if (this->mag) addIndent(attrIndent), ss << "mag: " << this->mag.value() << '\n';
        if (this->angle) addIndent(attrIndent), ss << "angle: " << this->angle.value() << '\n';

        auto [x, y] { this->xy.at(0) };
        addIndent(attrIndent), ss << "coordinate: [" << x << ", " << y << ']' << '\n';   
    }

    auto GDSIISref::toString(std::size_t indent) const -> std::string
    {
        std::stringstream ss {};
        this->toString(ss, indent);
        return ss.str();
    }

    ////////////////////////////////////////////////////////////////////////

    GDSIIAref::GDSIIAref(std::string sName, std::int16_t col, std::int16_t row, const GDSIIDBCoord& coord) :
        sName{std::move(sName)},
        col{col},
        row{row},
        xy{coord}
    {
    }

    auto GDSIIAref::toString(std::stringstream& ss, std::size_t indent) const -> void
    {
        auto addIndent = [&ss] (std::size_t indent) {
            for (std::size_t i = 0; i < indent; ++i)
                ss << "    ";
        };

        addIndent(indent);
        ss << "Aref Element\n";

        auto attrIndent {indent + 1};

        // For each attr
        if (this->elfFlags) addIndent(attrIndent), ss << "elf_flags: " << this->elfFlags.value() << '\n';
        if (this->plex) addIndent(attrIndent), ss << "plex: " << this->plex.value() << '\n';

        addIndent(attrIndent), ss << "s_name: " << this->sName << '\n';
        if (this->sTrans) 
        {
            addIndent(attrIndent); 
            ss << "s_trans:";
            if (this->sTrans.value().x_mirror) ss << " x_mirror";
            if (this->sTrans.value().magnify) ss << " magnify";
            if (this->sTrans.value().rotate) ss << " rotate";
            ss << '\n';
        }

        if (this->mag) addIndent(attrIndent), ss << "mag: " << this->mag.value() << '\n';
        if (this->angle) addIndent(attrIndent), ss << "angle: " << this->angle.value() << '\n';

        addIndent(attrIndent), ss << "col: " << this->col << '\n';
        addIndent(attrIndent), ss << "row: " << this->row << '\n';

        auto [x, y] { this->xy.at(0) };
        addIndent(attrIndent), ss << "coordinate: [" << x << ", " << y << ']' << '\n';   
    }

    auto GDSIIAref::toString(std::size_t indent) const -> std::string
    {
        std::stringstream ss {};
        this->toString(ss, indent);
        return ss.str();
    }

    /////////////////////////////////////////////////////////////////////////////////

    GDSIIText::GDSIIText(std::int16_t layer, const GDSIIDBCoord& coord, std::string text) :
        layer{layer},
        xy{coord},
        string{std::move(text)}
    {
    }

    auto GDSIIText::toString(std::stringstream& ss, std::size_t indent) const -> void
    {
        auto addIndent = [&ss] (std::size_t indent) {
            for (std::size_t i = 0; i < indent; ++i)
                ss << "    ";
        };

        addIndent(indent);
        ss << "Text Element\n";

        auto attrIndent {indent + 1};

        // For each attr
        if (this->elfFlags) addIndent(attrIndent), ss << "elf_flags: " << this->elfFlags.value() << '\n';
        if (this->plex) addIndent(attrIndent), ss << "plex: " << this->plex.value() << '\n';

        addIndent(attrIndent), ss << "layer: " << this->layer << '\n';
        addIndent(attrIndent), ss << "data_type: " << this->textType << '\n';

        if (this->presentation) addIndent(attrIndent), ss << "presentation: " << this->presentation.value() << '\n';
        if (this->pathType) addIndent(attrIndent), ss << "text_type: " << this->pathType.value() << '\n';
        if (this->width) addIndent(attrIndent), ss << "width: " << this->width.value() << '\n';
        if (this->sTrans) 
        {
            addIndent(attrIndent); 
            ss << "s_trans:";
            if (this->sTrans.value().x_mirror) ss << " x_mirror";
            if (this->sTrans.value().magnify) ss << " magnify";
            if (this->sTrans.value().rotate) ss << " rotate";
            ss << '\n';
        }
        if (this->mag) addIndent(attrIndent), ss << "mag: " << this->mag.value() << '\n';
        if (this->angle) addIndent(attrIndent), ss << "angle: " << this->angle.value() << '\n';

        auto [x, y] { this->xy.at(0) };
        addIndent(attrIndent), ss << "coordinate: [" << x << ", " << y << ']' << '\n';   

        addIndent(attrIndent), ss << "string: " << this->string << '\n';
    }
    
    auto GDSIIText::toString(std::size_t indent) const -> std::string
    {
        std::stringstream ss {};
        this->toString(ss, indent);
        return ss.str();
    }

    /////////////////////////////////////////////////////////////////////////

    GDSIINode::GDSIINode(std::int16_t layer, const GDSIIDBCoord& coord) :
        layer{layer},
        xy{coord}
    {
    }

    auto GDSIINode::toString(std::stringstream& ss, std::size_t indent) const -> void
    {
        auto addIndent = [&ss] (std::size_t indent) {
            for (std::size_t i = 0; i < indent; ++i)
                ss << "    ";
        };

        addIndent(indent);
        ss << "Node Element\n";

        auto attrIndent {indent + 1};

        // For each attr
        if (this->elfFlags) addIndent(attrIndent), ss << "elf_flags: " << this->elfFlags.value() << '\n';
        if (this->plex) addIndent(attrIndent), ss << "plex: " << this->plex.value() << '\n';
        addIndent(attrIndent), ss << "layer: " << this->layer << '\n';
        addIndent(attrIndent), ss << "data_type: " << this->nodeType << '\n';

        // auto [x, y] { this->xy.at(0) };
        // ss << "coordinate: [" << x << ", " << y << ']' << '\n';
        addIndent(attrIndent);
        ss << "xy: [";
        for (const auto& [x, y] : this->xy) 
            ss << '(' << x << ", " << y << "), ";
        ss << "]\n";     
    }

    auto GDSIINode::toString(std::size_t indent) const -> std::string
    {
        std::stringstream ss {};
        this->toString(ss, indent);
        return ss.str();
    }

    ////////////////////////////////////////////////////////////////////////////

    GDSIIBox::GDSIIBox(std::int16_t layer, const GDSIIDBCoord& coord) :
        layer{layer},
        xy{coord}
    {
    }

    auto GDSIIBox::toString(std::stringstream& ss, std::size_t indent) const -> void
    {
        auto addIndent = [&ss] (std::size_t indent) {
            for (std::size_t i = 0; i < indent; ++i)
                ss << "    ";
        };

        addIndent(indent);
        ss << "Box Element\n";

        auto attrIndent {indent + 1};

        // For each attr
        if (this->elfFlags) addIndent(attrIndent), ss << "elf_flags: " << this->elfFlags.value() << '\n';
        if (this->plex) addIndent(attrIndent), ss << "plex: " << this->plex.value() << '\n';
        addIndent(attrIndent), ss << "layer: " << this->layer << '\n';
        addIndent(attrIndent), ss << "data_type: " << this->boxType << '\n';

        // auto [x, y] { this->xy.at(0) };
        // ss << "coordinate: [" << x << ", " << y << ']' << '\n';
        addIndent(attrIndent);
        ss << "xy: [";
        for (const auto& [x, y] : this->xy) 
            ss << '(' << x << ", " << y << "), ";
        ss << "]\n";     
    }

    auto GDSIIBox::toString(std::size_t indent) const -> std::string
    {
        std::stringstream ss {};
        this->toString(ss, indent);
        return ss.str();
    }
    
    /////////////////////////////////////////////////////////////////////////////////

    auto GDSIIDateTime::now() -> GDSIIDateTime
    {
        auto now {std::chrono::system_clock::now()};
        std::time_t nowTime {std::chrono::system_clock::to_time_t(now)};
        std::tm* loaclTime {std::localtime(&nowTime)};

        return GDSIIDateTime {
            .year = static_cast<std::int16_t>(loaclTime->tm_year + 1900),
            .month = static_cast<std::int16_t>(loaclTime->tm_mon + 1),
            .day = static_cast<std::int16_t>(loaclTime->tm_mday),
            .hour = static_cast<std::int16_t>(loaclTime->tm_hour),
            .minute = static_cast<std::int16_t>(loaclTime->tm_min),
            .second = static_cast<std::int16_t>(loaclTime->tm_sec),
        };
    }
    
    auto GDSIIDateTime::toString(std::stringstream& ss, std::size_t indent) const -> void
    {
        for (std::size_t i = 0; i < indent; ++i)
            ss << "    ";
        ss << this->year << '-' << this->month << '-' << this->day << ' ' 
           << this->hour << ':' << this->minute << ':' << this->second << '\n';
    }

    auto GDSIIDateTime::toString(std::size_t indent) const -> std::string
    {
        std::stringstream ss {};
        this->toString(ss, indent);
        return ss.str();
    }

}