#include "./argument.hh"

#include <circuit/component/mos.hh>
#include <global/tech/driveability.hh>
#include <global/number/number.hh>
#include <utility/debug/debug.hh>

#include <string>
#include <exception>

namespace xtaro::circuit 
{

    auto Argument::buildByString(std::string_view str) -> Argument
    {
        try
        {
            auto number = Number::fromString(std::string{str});
            return Argument{number};
        }
        catch (const std::exception&)
        {
            try
            {
                auto dc = driveAbilityFromString(str);
                return Argument{dc};
            }
            catch (const std::exception&)
            {
                try
                {
                    if (str == "NMOS")
                        return Argument{MOSType::NMOS};
                    else if (str == "PMOS")
                        return Argument{MOSType::PMOS};
                    debug::exception("");
                }
                catch (const std::exception&)
                {
                    return Argument{std::move(str)};
                }
            }
        }
    }

}