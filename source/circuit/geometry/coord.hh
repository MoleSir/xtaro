#pragma once

#include <global/number/number.hh>
#include <parse/gdsii/gdsiicoord.hh>
#include <string>
#include <cstdint>
#include <format>
#include <span>
#include <vector>

namespace xtaro
{

    enum class MirrorType
    {
        None,
        MirrorX,
        MirrorY,
        MirrorXY,
    };

    struct Coord
    {
        Coord();
        Coord(const Number& x, const Number& y);
        
        auto snapToGrid() const -> Coord;
        static auto snapOffsetToGrid(const Number& v) -> Number;
        static auto snapToGrid(std::span<const Coord> coords) -> std::vector<Coord>;

        auto rotate() const -> Coord;
        auto scale(double factor)-> Coord;
        auto scale(double factorX, double factorY)-> Coord;
        auto rotateScale(double factor) -> Coord;
        auto rotateScale(double factorX, double factorY) -> Coord;
        auto transform(const Coord& offset, MirrorType mirror, double rotate) const -> Coord;
        static auto transform(std::span<const Coord> coords, const Coord& offset, MirrorType mirror, double rotate) -> std::vector<Coord>;

        auto floor() const -> Coord;
        auto ceil() const -> Coord;
        auto round() const -> Coord;

        auto max(const Coord& other) const -> Coord;
        auto min(const Coord& other) const -> Coord;

        auto toString() const -> std::string;
        auto toGDSIIMicroCoord() const -> parse::GDSIIMicroCoord;

        Number x;
        Number y;
    };

    auto operator + (const Coord& c1, const Coord& c2) -> Coord;
    auto operator - (const Coord& c1, const Coord& c2) -> Coord;
    auto operator == (const Coord& c1, const Coord& c2) -> bool;
    auto operator != (const Coord& c1, const Coord& c2) -> bool;

    auto operator += (Coord& c1, const Coord& c2) -> void;
    auto operator -= (Coord& c1, const Coord& c2) -> void;

}

template <>
struct std::hash<xtaro::Coord> {
    auto operator() (const xtaro::Coord& c) const noexcept -> std::uint64_t;
};

template <>
struct std::formatter<xtaro::Coord> 
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
	constexpr auto format(const xtaro::Coord& c, FormatContext& ctx) const {
		auto fmt = std::format("({}, {})", m_fmt, m_fmt);
		auto iter = std::vformat_to(ctx.out(), fmt, std::make_format_args(c.x, c.y));
		return iter;
	}

private:
	char m_fmt[16]{};
	size_t m_buffer_len = 0;
};