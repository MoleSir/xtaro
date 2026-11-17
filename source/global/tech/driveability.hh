#pragma once

#include <string_view>
#include <utility/misc/formator.hh>

namespace xtaro 
{

    enum class DriveAbility 
    {
        X0, X1, X2, X3
    };

    auto driveAbilityFromString(std::string_view s) -> DriveAbility;

}

FORMAT_ENUM_AS(xtaro::DriveAbility,
    ENUM_VALUE_AS(X0, "x0")
    ENUM_VALUE_AS(X1, "x1")
    ENUM_VALUE_AS(X2, "x2")
    ENUM_VALUE_AS(X3, "x3")
)