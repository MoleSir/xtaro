#include "gdsiistructrue.hh"

namespace xtaro::parse
{

    auto GDSIIStructure::withName(std::string name) -> GDSIIStructure
    {
        return GDSIIStructure {
            .name{std::move(name)},
            .createDate{GDSIIDateTime::now()},
            .modifyDate{GDSIIDateTime::now()},
            .boundarys{},
            .paths{},
            .srefs{},
            .arefs{},
            .texts{},
            .nodes{},
            .boxes{}
        };
    }

    auto GDSIIStructure::toString(std::stringstream& ss, std::size_t indent) const -> void
    {
        auto addIndent = [&ss] (std::size_t indent) {
            for (std::size_t i = 0; i < indent; ++i)
                ss << "    ";
        };

        addIndent(indent), ss << "Structure:\n";
        addIndent(indent + 1), ss << "name: " << this->name << '\n';
        addIndent(indent + 1), ss << "create date: " << this->createDate.toString();
        addIndent(indent + 1), ss << "modify date: " << this->modifyDate.toString();

        for (const auto& e : this->boundarys)
            e->toString(ss, indent + 1);

        for (const auto& e : this->paths)
            e->toString(ss, indent + 1);

        for (const auto& e : this->srefs)
            e->toString(ss, indent + 1);

        for (const auto& e : this->arefs)
            e->toString(ss, indent + 1);

        for (const auto& e : this->texts)
            e->toString(ss, indent + 1);

        for (const auto& e : this->nodes)
            e->toString(ss, indent + 1);

        for (const auto& e : this->boxes)
            e->toString(ss, indent + 1);
    }

    auto GDSIIStructure::toString(std::size_t indent) const -> std::string
    {
        std::stringstream ss {};
        this->toString(ss, indent);
        return ss.str();
    }


}