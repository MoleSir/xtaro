#pragma once

#include <string>
#include <string_view>
#include <format>

namespace xtaro::console {

    enum class Color {
        Black,
        Red,
        Green,
        Yellow,
        Blue,
        Purple,
        Cyan,
        White,
    };

    auto printWithColor(std::string_view message, Color color) -> void;
    auto print(std::string_view message) -> void;

    auto printlnWithColor(std::string_view message, Color color) -> void;
    auto println(std::string_view message) -> void;

    auto debug(std::string_view message) -> void;
    auto info(std::string_view message) -> void;
    auto warning(std::string_view message) -> void;
    auto error(std::string_view message) -> void;
    auto fatal(std::string_view message) -> void;

    auto inputln(std::string_view prompt) -> std::string;
    auto inputCharNoEcho() -> char;

    template <typename... Args>
    auto printf(std::format_string<Args...> fmt, Args &&... args) -> void {
        auto str = std::format(std::move(fmt), std::forward<Args>(args)...);
        print(str);
    }

    template <typename... Args>
    auto printlnf(std::format_string<Args...> fmt, Args &&... args) -> void {
        auto str = std::format(std::move(fmt), std::forward<Args>(args)...);
        println(str);
    }

    template <typename... Args>
    auto debugf(std::format_string<Args...> fmt, Args &&... args) -> void {
        auto str = std::format(std::move(fmt), std::forward<Args>(args)...);
        debug(str);
    }

    template <typename... Args>
    auto infof(std::format_string<Args...> fmt, Args &&... args) -> void {
        auto str = std::format(std::move(fmt), std::forward<Args>(args)...);
        info(str);
    }

    template <typename... Args>
    auto warningf(std::format_string<Args...> fmt, Args &&... args) -> void {
        auto str = std::format(std::move(fmt), std::forward<Args>(args)...);
        warning(str);
    }

    template <typename... Args>
    auto errorf(std::format_string<Args...> fmt, Args &&... args) -> void {
        auto str = std::format(std::move(fmt), std::forward<Args>(args)...);
        error(str);
    }

    template <typename... Args>
    auto fatalf(std::format_string<Args...> fmt, Args &&... args) -> void {
        auto str = std::format(std::move(fmt), std::forward<Args>(args)...);
        fatal(str);
    }

}