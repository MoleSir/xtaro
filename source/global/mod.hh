#pragma once

#include <filesystem>

namespace xtaro::parse { class Json; }

namespace xtaro::global
{

    auto configSystem(const std::filesystem::path& configFile) -> void;
    auto configSystem(const parse::Json& json) -> void;

}