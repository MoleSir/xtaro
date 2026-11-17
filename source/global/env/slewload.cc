#include "./slewload.hh"
#include <global/number/number.hh>
#include <utility/misc/string.hh>
#include <format>
#include <string>

namespace xtaro {

    SlewLoad::SlewLoad(Number s, Number l) :
        slew{s}, load{l} 
    {
    }

    SlewLoad::SlewLoad() : SlewLoad{Number::zero(), Number::zero()} 
    {
    }

    std::string SlewLoad::toString() const
    {
        return std::format(
            "S{:.3f}_L{:.3f}", 
            this->slew, this->load
        );
    }

    bool operator == (const SlewLoad& left, const SlewLoad& right) 
    {
        return left.load == right.load && left.slew == right.slew;
    }

    auto SlewLoad::fromString(std::span<std::string_view const> arguments) -> SlewLoad 
    {
        auto slewload = SlewLoad{};
        slewload.slew = Number::fromString(std::string{arguments[0]});
        slewload.load = Number::fromString(std::string{arguments[1]});
        return slewload;
    }

}

std::size_t std::hash<xtaro::SlewLoad>::operator () (const xtaro::SlewLoad& sl) const noexcept {
    return static_cast<std::size_t>(sl.load.value()) + static_cast<std::size_t>(sl.slew.value());
}
