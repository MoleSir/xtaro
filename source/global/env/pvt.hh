#pragma once

#include <parse/serde/mod.hh>
#include <utility/misc/string.hh>
#include <global/number/number.hh>

#include <string>
#include <span>

namespace xtaro
{
    struct PVT
    {
        PVT();
        PVT(std::string p, Number v, Number t);
        PVT(std::string p, double v, double t);

        static auto fromString(std::span<std::string_view const> arguments) -> PVT;
        std::string toString() const;

        std::string process;
        Number      voltage;
        Number      temperature;
    };

}

DESERIALIZE_STRUCT(xtaro::PVT,
    DE_FILED(process)
    DE_FILED(voltage)
    DE_FILED(temperature)
)