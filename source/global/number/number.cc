#include "./number.hh"
#include <functional>
#include <utility/debug/debug.hh>
#include <parse/serde/mod.hh>

#include <cassert>
#include <cstdlib>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <regex>
#include <cmath>

namespace xtaro
{    
    auto unitToExpoent(Unit unit) -> int
    {
        switch (unit) {
            case Unit::Mega: return 6;
            case Unit::Kilo: return 3;
            case Unit::Origin: return 0;
            case Unit::Milli: return -3;
            case Unit::Micro: return -6;
            case Unit::Nano: return -9;
            case Unit::Pico: return -12;
        }
        debug::unreachable();        
    }

    auto unitToRatio(Unit unit) -> double 
    {
        switch (unit) {
            case Unit::Mega: return 1e6;
            case Unit::Kilo: return 1e3;
            case Unit::Origin: return 1e0;
            case Unit::Milli: return 1e-3;
            case Unit::Micro: return 1e-6;
            case Unit::Nano: return 1e-9;
            case Unit::Pico: return 1e-12;
        }
        debug::unreachable();
    }

    auto ratioToUnit(double ratio) -> std::optional<Unit> 
    {
        if (ratio == 1e6) return Unit::Mega;
        if (ratio == 1e3) return Unit::Kilo;
        if (ratio == 1) return Unit::Origin;
        if (ratio == 1e-3) return Unit::Milli;
        if (ratio == 1e-6) return Unit::Micro;
        if (ratio == 1e-9) return Unit::Nano;
        if (ratio == 1e-12) return Unit::Pico;
        return std::nullopt;
    }

    auto unitToChar(Unit unit) -> std::string_view 
    {
        switch (unit) {
            case Unit::Mega: return "M";
            case Unit::Kilo: return "K";
            case Unit::Origin: return "";
            case Unit::Milli: return "m";
            case Unit::Micro: return "u";
            case Unit::Nano: return "n";
            case Unit::Pico: return "p";
        }
        debug::unreachable();
    }

    auto Number::unitValue(Unit unit) const -> double
    {
        // auto srcRatio = unitToRatio(this->_unit);
        // auto tarRatio = unitToRatio(unit);
        // auto bias = srcRatio / tarRatio;
        // return this->_value * bias;

        auto srcExp = unitToExpoent(this->_unit);
        auto tarExp = unitToExpoent(unit);

        if (srcExp == tarExp)
        {
            return this->_value;
        }
        else if (srcExp > tarExp) 
        {
            auto ratio = std::round(std::pow(10, srcExp - tarExp));
            return this->_value * ratio;
        }
        else
        {
            auto ratio = std::round(std::pow(10, tarExp - srcExp));
            // debug::debugf("{}", ratio);
            return this->_value / ratio;
        }
    }

    auto Number::microValue() const -> double
    {
        return this->unitValue(Unit::Micro);
    }

    auto Number::originValue() const -> double
    {
        return this->unitValue(Unit::Origin);
    }

    auto Number::toUnit(Unit unit) const -> Number
    {
        auto srcRatio = unitToRatio(this->_unit);
        auto tarRatio = unitToRatio(unit);
        auto bias = srcRatio / tarRatio;
        return Number{this->_value * bias, unit};
    }

    auto Number::toOrigin() const -> Number
    {
        return this->toUnit(Unit::Origin);
    }

    auto Number::toMicro() const -> Number
    {
        return this->toUnit(Unit::Micro);
    }

    auto Number::toNano() const -> Number
    {
        return this->toUnit(Unit::Nano);
    }

    auto Number::abs() const -> Number
    {
        return Number{fabs(this->_value), this->_unit};
    }

    auto Number::floor() const -> Number
    {
        return Number{std::floor(this->_value), this->_unit};
    }

    auto Number::ceil() const -> Number
    {
        return Number{std::ceil(this->_value), this->_unit};
    }

    auto Number::round() const -> Number
    {
        return Number{std::round(this->_value), this->_unit};
    }

    auto Number::max(const Number& n) const -> Number
    {
        return *this > n ? *this : n;
    }

    auto Number::min(const Number& n) const -> Number
    {
        return *this < n ? *this : n;
    }
    
    auto Number::divide(const Number& n) const -> double
    {
        return this->originValue() / n.originValue();
    }

    auto Number::approximate(const Number& n, const double epsi) const -> bool
    {
        auto [nn1, nn2] = uniformNumbers(*this, n);
        assert(nn1.unit() == nn2.unit());
        return std::abs(
            (nn1.value() - nn2.value()) / nn1.value()
        ) < epsi;
    }

    auto Number::value() const -> double 
    {
        // return this->_value * unitToRatio(this->_unit);
        return this->_value;
    }

    auto Number::unit() const -> Unit 
    {
        return this->_unit;
    }

    auto Number::fromString(const std::string& str) -> Number
    {   
        std::regex regex(R"(^([+-]?(\d+(\.\d*)?|\.\d+))(M|k|m|u|n|p)?$)");
        std::smatch match;

        if (std::regex_match(str, match, regex)) {
            auto value = std::stod(match[1].str()); 
            auto suffix  =  match[4].str();
            auto unit = Unit::Origin;
            if (suffix == "M") unit = Unit::Mega; 
            else if (suffix == "M") unit = Unit::Mega;
            else if (suffix == "K") unit = Unit::Kilo;
            else if (suffix == "") unit = Unit::Origin;
            else if (suffix == "m") unit = Unit::Milli;
            else if (suffix == "u") unit = Unit::Micro;
            else if (suffix == "n") unit = Unit::Nano;
            else if (suffix == "p") unit = Unit::Pico;
            return Number{value, unit};
        }
        debug::exceptionfIn("Parse `Number`", "'{}' is not a valid number literal", str);
    }

    auto Number::toString() const -> std::string
    {
        return std::format("{}", *this);
    }

    auto operator == (const Number& n1, const Number& n2) -> bool
    {
        auto [nn1, nn2] = uniformNumbers(n1, n2);
        assert(nn1.unit() == nn2.unit());
        return nn1.value() == nn2.value();
    }

    auto operator != (const Number& n1, const Number& n2) -> bool
    {
        return !(n1 == n2);
    }

    auto operator >  (const Number& n1, const Number& n2) -> bool
    {
        auto [nn1, nn2] = uniformNumbers(n1, n2);
        assert(nn1.unit() == nn2.unit());
        return nn1.value() > nn2.value();
    }

    auto operator >= (const Number& n1, const Number& n2) -> bool
    {
        return n1 > n2 || n1 == n2;
    }

    auto operator <  (const Number& n1, const Number& n2) -> bool
    {
        auto [nn1, nn2] = uniformNumbers(n1, n2);
        assert(nn1.unit() == nn2.unit());
        return nn1.value() < nn2.value();
    }  

    auto operator <= (const Number& n1, const Number& n2) -> bool
    {
        return n1 < n2 || n1 == n2;
    }

    auto operator +  (const Number& n1, const Number& n2) -> Number
    {
        auto [nn1, nn2] = uniformNumbers(n1, n2);
        assert(nn1.unit() == nn2.unit());
        return Number{nn1.value() + nn2.value(), nn1.unit()};
    }
    
    auto operator -  (const Number& n1, const Number& n2) -> Number
    {
        auto [nn1, nn2] = uniformNumbers(n1, n2);
        assert(nn1.unit() == nn2.unit());
        return Number{nn1.value() - nn2.value(), nn1.unit()};
    }

    auto operator *  (const Number& n1, const Number& n2) -> Number
    {
        auto [nn1, nn2] = uniformNumbers(n1, n2);
        assert(nn1.unit() == nn2.unit());
        return Number{nn1.value() * nn2.value(), nn1.unit()};
    }

    auto operator /  (const Number& n1, const Number& n2) -> Number
    {
        auto [nn1, nn2] = uniformNumbers(n1, n2);
        assert(nn1.unit() == nn2.unit());
        return Number{nn1.value() / nn2.value(), nn1.unit()};
    }

    auto operator +  (const double k, const Number& n) -> Number
    {
        auto [nn1, nn2] = uniformNumbers(Number::origin(k), n);
        assert(nn1.unit() == nn2.unit());
        return Number{nn1.value() + nn2.value(), nn1.unit()};
    }

    auto operator +  (const Number& n, const double k) -> Number
    {
        return k + n;
    }

    auto operator -  (const double k, const Number& n) -> Number
    {
        auto [nn1, nn2] = uniformNumbers(Number::origin(k), n);
        assert(nn1.unit() == nn2.unit());
        return Number{nn1.value() - nn2.value(), nn1.unit()};
    }

    auto operator -  (const Number& n, const double k) -> Number
    {
        return k - n;
    }

    auto operator *  (const double k, const Number& n) -> Number
    {
        return Number{k * n.value(), n.unit()};
    }

    auto operator *  (const Number& n, const double k) -> Number
    {
        return k * n;
    }

    auto operator /  (const double k, const Number& n) -> Number
    {
        return Number{k / n.value(), n.unit()};
    }

    auto operator /  (const Number& n, const double k) -> Number
    {
        return Number{n.value() / k, n.unit()};
    }

    auto operator += (Number& n1, const Number& n2) -> void
    {
        n1 = n1 + n2;
    }
    
    auto operator -= (Number& n1, const Number& n2) -> void
    {
        n1 = n1 - n2;
    }

    auto operator -  (const Number& n) -> Number
    {
        return -1.0 * n;
    }

    auto uniformNumbers(const Number& n1, const Number& n2) -> std::tuple<Number, Number> 
    {
        double ratio1 = unitToRatio(n1.unit());
        double ratio2 = unitToRatio(n2.unit());
        
        if (ratio1 == ratio2) 
        {
            return {n1, n2};
        }
        else if (ratio1 > ratio2) 
        {
            auto newVal1 = n1.value() * (ratio1 / ratio2);
            return {Number{newVal1, n2.unit()}, n2};
        }
        else  // r1 < r2
        {
            auto newVal2 = n2.value() * (ratio2 / ratio1);
            return {n1, Number{newVal2, n1.unit()}};
        }
    }

}

auto std::hash<xtaro::Number>::operator() (const xtaro::Number& n) const noexcept -> std::uint64_t
{
    return std::hash<double>{}(n.value()) ^ std::hash<int>{}(static_cast<int>(n.unit()));
}