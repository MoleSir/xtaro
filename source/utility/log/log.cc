#include "./log.hh"
#include <fstream>
#include <cstring>

#include <utility/debug/debug.hh>

namespace xtaro::log {

    static auto logfile = std::ofstream{};

    static auto log(std::string_view level, std::string_view message) -> void;
    static auto logDateTime() -> void;
    static auto logLevel(std::string_view level) -> void;
    static auto logEnable() -> bool;

    auto initial(const std::filesystem::path& logpath) -> void {
        logfile.open(logpath);
        if (!logfile.is_open()) 
            debug::exceptionfIn("Initial log", "Open '{}' failed", logpath.c_str());
    }

    auto debug(std::string_view message) -> void {
        if (!logEnable()) return;
        log(" DEBUG ", message);
    }

    auto info(std::string_view message) -> void {
        if (!logEnable()) return;
        log(" INFO  ", message);
    }

    auto warning(std::string_view message) -> void {
        if (!logEnable()) return;
        log("WARNING", message);
    }

    auto error(std::string_view message) -> void {
        if (!logEnable()) return;
        log(" ERROR ", message);
    }

    auto fatal(std::string_view message) -> void {
        if (!logEnable()) return;
        log(" FATAL ", message);
    }

    static auto log(std::string_view level, std::string_view message) -> void
    {
        logDateTime();
        logLevel(level);
        logfile << message << '\n';
        logfile.flush();
    }

    static auto logDateTime() -> void
    {
        time_t current_ticks = std::time(NULL);
        struct tm* time = std::localtime(&current_ticks);

        char dateTime[32] = {0};
        std::memset(dateTime, 0, sizeof(dateTime));
        std::strftime(dateTime, sizeof(dateTime), "%Y-%m-%d %H:%M:%S", time);
        logfile << '[' << dateTime << ']';
    }

    static auto logLevel(std::string_view level) -> void
    {
        logfile << ' ' << level << " > ";
    }

    static auto logEnable() -> bool {
        return logfile.is_open();
    }

}
