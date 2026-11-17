#pragma once

#include <exception>
#include <cstdint>

#include <parse/serde/mod.hh>
#include <format>
#include <string>
#include <string_view>
#include <tuple>

namespace xtaro 
{

    enum class Unit
    {
        Mega = 0,   // M
        Kilo,       // k
        Origin,     // 
        Milli,      // m
        Micro,      // mu
        Nano,       //  n
        Pico,       // p
    };

    class Number
    {
    public:
        constexpr static auto mega(double value) -> Number 
        {
            return Number{value, Unit::Mega};
        }

        constexpr static auto kilo(double value) -> Number
        {
            return Number{value, Unit::Kilo};
        }

        constexpr static auto origin(double value) -> Number
        {
            return Number{value, Unit::Origin};
        }

        constexpr static auto milli(double value) -> Number
        {
            return Number{value, Unit::Milli};
        }

        constexpr static auto micro(double value) -> Number
        {
            return Number{value, Unit::Micro};
        }

        constexpr static auto nano(double value) -> Number
        {
            return Number{value, Unit::Nano};
        }

        constexpr static auto pico(double value) -> Number
        {
            return Number{value, Unit::Pico};
        }

        constexpr static auto zero() -> Number 
        {
            return Number::origin(0);
        }

        constexpr Number(double value, Unit unit) :
            _value{value}, _unit{unit}
        {
        }

        constexpr Number() : Number{0.0, Unit::Origin} {}

    public:
        auto abs() const -> Number;
        auto floor() const -> Number;
        auto ceil() const -> Number;
        auto round() const -> Number; 
        auto max(const Number& n) const -> Number;
        auto min(const Number& n) const -> Number;
        auto divide(const Number& n) const -> double;
        auto approximate(const Number& n, const double epsi = 10e-6) const -> bool;

    public:
        auto unitValue(Unit unit) const -> double;
        auto microValue() const -> double;
        auto originValue() const -> double;

    public:
        auto toUnit(Unit unit) const -> Number;
        auto toOrigin() const -> Number;
        auto toMicro() const -> Number;
        auto toNano() const -> Number;

    public:
        static auto fromString(const std::string& str) -> Number;
        auto toString() const -> std::string;

    public:
        auto value() const -> double;
        auto unit() const -> Unit;

    private:
        double _value;
        Unit   _unit;
    };

    constexpr auto operator"" _M(long double v) -> Number
    {
        return Number::mega(v);
    }

    constexpr auto operator"" _k(long double v) -> Number
    {
        return Number::kilo(v);
    }

    constexpr auto operator"" _o(long double v) -> Number
    {
        return Number::origin(v);
    }

    constexpr auto operator"" _m(long double v) -> Number
    {
        return Number::milli(v);
    }

    constexpr auto operator"" _u(long double v) -> Number
    {
        return Number::micro(v);
    }

    constexpr auto operator"" _n(long double v) -> Number
    {
        return Number::nano(v);
    }
    
    constexpr auto operator"" _p(long double v) -> Number
    {
        return Number::pico(v);
    }

    auto operator == (const Number& n1, const Number& n2) -> bool;
    auto operator != (const Number& n1, const Number& n2) -> bool;
    auto operator >  (const Number& n1, const Number& n2) -> bool;
    auto operator >= (const Number& n1, const Number& n2) -> bool;
    auto operator <  (const Number& n1, const Number& n2) -> bool;
    auto operator <= (const Number& n1, const Number& n2) -> bool;

    auto operator +  (const Number& n1, const Number& n2) -> Number;
    auto operator -  (const Number& n1, const Number& n2) -> Number;
    auto operator *  (const Number& n1, const Number& n2) -> Number;
    auto operator /  (const Number& n1, const Number& n2) -> Number;

    auto operator +  (const double k, const Number& n) -> Number;
    auto operator +  (const Number& n, const double k) -> Number;
    auto operator -  (const double k, const Number& n) -> Number;
    auto operator -  (const Number& n, const double k) -> Number;
    auto operator *  (const double k, const Number& n) -> Number;
    auto operator *  (const Number& n, const double k) -> Number;
    auto operator /  (const double k, const Number& n) -> Number;
    auto operator /  (const Number& n, const double k) -> Number;

    auto operator += (Number& n1, const Number& n2) -> void;
    auto operator -= (Number& n1, const Number& n2) -> void;

    auto operator -  (const Number& n) -> Number;

    auto unitToChar(Unit unit) -> std::string_view;

    auto uniformNumbers(const Number& n1, const Number& n2) -> std::tuple<Number, Number>;

}

template <>
struct std::hash<xtaro::Number> {
    auto operator() (const xtaro::Number& n) const noexcept -> std::uint64_t;
};

template <typename Deserializer>
struct xtaro::parse::Deserialize<Deserializer, xtaro::Number> {
    static void from(const Deserializer& dsr, xtaro::Number& n) {
        auto str = std::string{};
        try 
        {
            xtaro::parse::Deserialize<Deserializer, std::string>::from(dsr, str);
        }
        catch (const std::exception&)
        {
            auto v = 0.0;
            try 
            {
                xtaro::parse::Deserialize<Deserializer, double>::from(dsr, v);
            }
            catch (const std::exception&)
            {
                auto v = 0;
                xtaro::parse::Deserialize<Deserializer, int>::from(dsr, v);
                n = Number::origin(static_cast<double>(v));
                return;
            }
            n = Number::origin(v);
            return;
        }
        n = Number::fromString(str);
    }
};

template <>
struct std::formatter<xtaro::Number> 
{
    template<typename ParseContext>
	constexpr auto parse(ParseContext& ctx) {
		m_fmt[m_buffer_len++] = '{';
		auto iter = ctx.begin();
		if (iter == ctx.end() || *iter == '}') {
			m_fmt[m_buffer_len++] = '}';
			return iter;
		}
		m_fmt[m_buffer_len++] = ':';
		for (; iter != ctx.end() && *iter != '}'; ++iter)
			m_fmt[m_buffer_len++] = *iter;
		m_fmt[m_buffer_len++] = '}';
		return iter;
	}

    template<typename FormatContext>
	constexpr auto format(const xtaro::Number& n, FormatContext& ctx) const {
		std::string fmt{};
		fmt += m_fmt, fmt += "{}";
		auto iter = std::vformat_to(ctx.out(), fmt, std::make_format_args(n.value(), xtaro::unitToChar(n.unit())));
		return iter;
	}

private:
	char m_fmt[16]{};
	size_t m_buffer_len = 0;
};