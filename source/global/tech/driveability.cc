#include "./driveability.hh"
#include <utility/debug/debug.hh>

namespace xtaro 
{

    auto driveAbilityFromString(std::string_view s) -> DriveAbility 
    {
        if (s == "x0") return DriveAbility::X0;
        if (s == "x1") return DriveAbility::X1;
        if (s == "x2") return DriveAbility::X2;
        if (s == "x3") return DriveAbility::X3;
        debug::exceptionf("'{}' is not a valid DriverAbility", s);
    }
    
}