#include "./slews.hh"
#include <utility/debug/debug.hh>
#include <format>

namespace xtaro {

    Slews::Slews(Number rs, Number cs) :
        relatedSlew{rs}, constrainedSlew{cs} 
    {
    }

    Slews::Slews() : Slews{Number::zero(), Number::zero()} 
    {
    }

    bool operator == (const Slews& left, const Slews& right) 
    {
        return left.relatedSlew == right.relatedSlew && left.constrainedSlew == right.constrainedSlew;
    }
    
    std::string Slews::toString() const
    {
        return std::format(
            "S{:.3f}_S{:.3f}", 
            this->relatedSlew, this->constrainedSlew
        );
    }

    auto Slews::fromString(std::span<std::string_view const> arguments) -> Slews 
    {
        auto slews = Slews{};
        slews.relatedSlew = Number::fromString(std::string{arguments[0]});
        slews.constrainedSlew = Number::fromString(std::string{arguments[1]});
        return slews;
    }

}

std::size_t std::hash<xtaro::Slews>::operator () (const xtaro::Slews& ss) const noexcept {
    return static_cast<std::size_t>(ss.constrainedSlew.value()) + static_cast<std::size_t>(ss.relatedSlew.value());
}