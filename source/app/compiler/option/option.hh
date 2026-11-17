#pragma once

#include <number/number.hh>
#include <parse/serde/mod.hh>

#include <global/env/pvt.hh>
#include <global/env/slewload.hh>
#include <global/env/slews.hh>

#include <filesystem>
#include <string>
#include <vector>
#include <memory>

namespace xtaro::parse { class Json; }

namespace xtaro::compiler
{

    struct Option
    {
        static auto load(const std::filesystem::path& configFile) -> std::unique_ptr<Option>;
        static auto load(const xtaro::parse::Json& configFile) -> std::unique_ptr<Option>;

        int         wordWidth{-1};
        int         addressWidth{-1};
        bool        mbist {false};

        std::vector<std::string> processCorners {};
        std::vector<Number>      supplyVoltages {};
        std::vector<Number>      temperatures {};

        std::optional<std::string> functionTestPolicy {};

        std::vector<PVT> pvts {};
    };

    auto config(const std::filesystem::path& configFile) -> std::unique_ptr<Option>;

}

DESERIALIZE_STRUCT(xtaro::compiler::Option,
    DE_OPTION_FILED_AS(wordWidth, word_width)
    DE_OPTION_FILED_AS(addressWidth, address_width)
    DE_OPTION_FILED(mbist)
    DE_OPTION_FILED_AS(processCorners, process_corners)
    DE_OPTION_FILED_AS(supplyVoltages, supply_voltages)
    DE_OPTION_FILED(temperatures)
    DE_OPTION_FILED_AS(functionTestPolicy, function_test_policy)
)