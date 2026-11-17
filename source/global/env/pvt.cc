#include "./pvt.hh"
#include <global/number/number.hh>
#include <utility/misc/string.hh>
#include <utility/debug/debug.hh>
#include <format>
#include <string>

namespace xtaro
{

    PVT::PVT(std::string p, Number v, Number t) :
        process{std::move(p)}, voltage{v}, temperature{t} 
    {
    }

    PVT::PVT(std::string p, double v, double t) :
        process{std::move(p)}, voltage{Number::origin(v)}, temperature{Number::origin(t)} 
    {
    }

    PVT::PVT() : PVT{"", Number::zero(), Number::zero()} {}

    std::string PVT::toString() const
    {
        return std::format("P{}_V{:.3f}_T{:.3f}", this->process, this->voltage, this->temperature);
    }

    auto PVT::fromString(std::span<std::string_view const> arguments) -> PVT
    {
        auto pvt = PVT{std::string{arguments[0]}, -1.0, -1.0};
        pvt.voltage = Number::origin(util::stringToDouble(arguments[1]));
        pvt.temperature = Number::origin(util::stringToDouble(arguments[2]));
        return pvt;
    }

}