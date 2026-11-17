#include "./debug.hh"
#include <utility/misc/string.hh>
#include <optional>
#include <utility/console/console.hh>
#include <utility/log/log.hh>
#include <cstdlib>
#include <stdexcept>
#include <string>

namespace xtaro::debug {

    static auto debugLevel = DebugLevel::Info;

    auto stringToDebugLevel(std::string_view s) -> std::optional<DebugLevel> {
        auto lows = util::stringToLowercase(s);
        if (lows == "debug") return {DebugLevel::Debug};
        if (lows == "info") return {DebugLevel::Info};
        if (lows == "warning") return {DebugLevel::Warning};
        if (lows == "error") return {DebugLevel::Error};
        if (lows == "fatal") return {DebugLevel::Fatal};
        return std::nullopt;
    }

    static auto debugLevelToNumber(DebugLevel level) -> int {
        switch (level) {
            case DebugLevel::Debug:   return 0;
            case DebugLevel::Info:    return 1;
            case DebugLevel::Warning: return 2;
            case DebugLevel::Error:   return 3;
            case DebugLevel::Fatal:   return 4;
        }
        return -1;
    }

    auto initialLog(const std::filesystem::path& logPath) -> void {
        log::initial(logPath);
        infof("Initial debugger");
    }

    auto setDebugLevel(DebugLevel level) -> void {
        debugLevel = level;
    }

    auto isDebugLevelEnough(DebugLevel level) -> bool {
        return level >= debugLevel;
    }

    auto doDebug(std::string_view message) -> void {
        console::debug(message);
        log::debug(message);
    }

    auto doInfo(std::string_view message) -> void {
        console::info(message);
        log::info(message);
    }

    auto doWarning(std::string_view message) -> void {
        console::warning(message);
        log::warning(message);
    }

    auto doError(std::string_view message) -> void {
        console::error(message);
        log::error(message);
    }

    auto debug(std::string_view message) -> void {
        if (isDebugLevelEnough(DebugLevel::Debug)) {
            doDebug(message);
        }
    }

    auto info(std::string_view message) -> void {
        if (isDebugLevelEnough(DebugLevel::Info)) {
            doInfo(message);
        }
    }

    auto warning(std::string_view message) -> void {
        if (isDebugLevelEnough(DebugLevel::Warning)) {
            doWarning(message);
        }
    }

    auto error(std::string_view message) -> void {
        if (isDebugLevelEnough(DebugLevel::Error)) {
            doError(message);
        }
    }

    [[noreturn]] auto fatal(int exitCode, std::string_view message) -> void {
        console::fatal(message);
        log::fatal(message);
        std::exit(exitCode);
    }

    auto exceptionIn(std::string_view where, std::string_view reason) -> void {
        std::string message {std::format("{} >> {}", where, reason)};
        throw std::runtime_error {std::move(message)};
    }
    
    auto exception(std::string reason) -> void {
        throw std::runtime_error {std::move(reason)};
    }

    auto check(bool condition, std::string message) -> void {
        if (!condition) 
            exception(std::move(message));
    }

    [[noreturn]] auto unreachable(std::string_view message) -> void {
        fatalf(2, "Reach an unreachable place '{}'", message);
    }

    [[noreturn]] auto unimplement(std::string_view message) -> void {
        fatalf(2, "Reach an unimplement place '{}'", message);
    }

}