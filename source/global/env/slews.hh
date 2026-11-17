#pragma once

#include <parse/serde/mod.hh>
#include <global/number/number.hh>

#include <string>
#include <span>

namespace xtaro
{

    struct Slews
    {
        Slews(Number rs, Number cs);
        Slews();

        static auto fromString(std::span<std::string_view const> arguments) -> Slews;
        std::string toString() const;

        Number relatedSlew;
        Number constrainedSlew;
    };

    bool operator == (const Slews& left, const Slews& right);

}

DESERIALIZE_STRUCT(xtaro::Slews,
    DE_FILED(relatedSlew)
    DE_FILED(constrainedSlew)
)

template <>
struct std::hash<xtaro::Slews> {
    std::size_t operator () (const xtaro::Slews& ss) const noexcept;
};