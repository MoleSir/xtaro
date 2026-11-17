#pragma once

#include <cstdlib>
#include <optional>
#include <string_view>
#include <format>
#include <filesystem>
#include <utility>
#include <cassert>

namespace xtaro::debug
{

    enum class DebugLevel {
        Debug,
        Info,
        Warning,
        Error,
        Fatal,
    };

    auto stringToDebugLevel(std::string_view) -> std::optional<DebugLevel>;

    auto initialLog(const std::filesystem::path& logPath) -> void;
    auto setDebugLevel(DebugLevel level) -> void;
    auto isDebugLevelEnough(DebugLevel level) -> bool;

    auto debug(std::string_view message) -> void;
    auto info(std::string_view message) -> void;
    auto warning(std::string_view message) -> void;
    auto error(std::string_view message) -> void;

    auto doDebug(std::string_view message) -> void;
    auto doInfo(std::string_view message) -> void;
    auto doWarning(std::string_view message) -> void;
    auto doError(std::string_view message) -> void;

    [[noreturn]] auto fatal(int exitCode, std::string_view message) -> void;

    [[noreturn]] auto exception(std::string reason) -> void;
    [[noreturn]] auto exceptionIn(std::string_view where, std::string_view reason) -> void;

    auto check(bool condition, std::string message) -> void;

    [[noreturn]] auto unreachable(std::string_view mes = "") -> void;
    [[noreturn]] auto unimplement(std::string_view mes = "") -> void;

    template <typename... Args>
    auto debugf(std::format_string<Args...> fmt, Args &&... args) -> void {
        if (isDebugLevelEnough(DebugLevel::Debug)) {
            auto str = std::format(std::move(fmt), std::forward<Args>(args)...);
            doDebug(str);
        }
    }

    template <typename... Args>
    auto infof(std::format_string<Args...> fmt, Args &&... args) -> void {
        if (isDebugLevelEnough(DebugLevel::Info)) {
            auto str = std::format(std::move(fmt), std::forward<Args>(args)...);
            doInfo(str);
        }
    }

    template <typename... Args>
    auto warningf(std::format_string<Args...> fmt, Args &&... args) -> void {
        if (isDebugLevelEnough(DebugLevel::Warning)) {
            auto str = std::format(std::move(fmt), std::forward<Args>(args)...);
            doWarning(str);
        }
    }

    template <typename... Args>
    auto errorf(std::format_string<Args...> fmt, Args &&... args) -> void {
        if (isDebugLevelEnough(DebugLevel::Error)) {
            auto str = std::format(std::move(fmt), std::forward<Args>(args)...);
            doError(str);
        }
    }

    template <typename... Args>
    [[noreturn]] auto fatalf(int exitCode, std::format_string<Args...> fmt, Args &&... args) -> void {
        auto str = std::format(std::move(fmt), std::forward<Args>(args)...);
        fatal(exitCode, str);
        std::exit(exitCode);
    }

    template <typename... Args>
    [[noreturn]] auto exceptionf(std::format_string<Args...> fmt, Args &&... args) -> void {
        auto reason = std::format(std::move(fmt), std::forward<Args>(args)...);
        exception(std::move(reason));
    }

    template <typename... Args>
    [[noreturn]] auto exceptionfIn(std::string_view where, std::format_string<Args...> fmt, Args &&... args) -> void {
        auto reason = std::format(std::move(fmt), std::forward<Args>(args)...);
        exceptionIn(where, reason);
    }

    template <typename... Args>
    auto checkf(bool condition, std::format_string<Args...> fmt, Args &&... args) -> void {
        if (!condition)
            exceptionf(std::move(fmt), std::forward<Args>(args)...);
    }

}