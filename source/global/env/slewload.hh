
#pragma once

#include <parse/serde/mod.hh>
#include <global/number/number.hh>

#include <string>
#include <span>

namespace xtaro
{
    
    struct SlewLoad 
    {
        SlewLoad(Number s, Number l);
        SlewLoad();

        static auto fromString(std::span<std::string_view const> arguments) -> SlewLoad;
        std::string toString() const;

        Number slew;
        Number load;
    };

    bool operator == (const SlewLoad& left, const SlewLoad& right);

}

DESERIALIZE_STRUCT(xtaro::SlewLoad,
    DE_FILED(slew)
    DE_FILED(load)
)

template <>
struct std::hash<xtaro::SlewLoad> {
    std::size_t operator () (const xtaro::SlewLoad& sl) const noexcept;
};
