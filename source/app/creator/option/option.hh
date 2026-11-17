#pragma once

#include <parse/serde/de.hh>
#include <circuit/factory/argument.hh>
#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <global/number/number.hh>
#include <parse/serde/mod.hh>

#include <global/env/pvt.hh>
#include <global/env/slewload.hh>
#include <global/env/slews.hh>

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <optional>
#include <map>

namespace xtaro::parse { class Json; };

namespace xtaro::creator
{

    enum class VolatgeType 
    { 
        Digit, 
        Analog 
    };

    struct DCStimulation
    {
        VolatgeType type { VolatgeType::Digit };
        Number voltage {};
    };

    struct PeriodStimulation
    {
        VolatgeType type { VolatgeType::Digit };
        Number timeBias {Number::zero()};
        std::vector<Number> voltages {};
    };

    struct PWLStimulation
    {
        VolatgeType type { VolatgeType::Digit };
        std::map<Number, Number> voltages {};
    };

    struct Environment
    {
        PVT pvt {
            "TT", 
            tech->normalVoltage(),
            tech->normalTemperature()
        };
        Number slew {0.01 * tech->normalPeriod()};
        Number load {Number::zero()};
        Number period {tech->normalPeriod()};
    };

    struct Stimulations
    {
        Environment environment {};
     
        bool addCLK { false };

        std::map<std::string, DCStimulation> dcStimuls {};
        std::map<std::string, PeriodStimulation> periodStimuls {};
        std::map<std::string, PWLStimulation> pwlStimuls {};
    };

    struct CircuitInfo 
    {
        std::string type;
        std::unordered_map<std::string, circuit::Argument> arguments;
    };

    struct Option
    {
        static auto load(const std::filesystem::path& configFile) -> std::unique_ptr<Option>;
        static auto load(const parse::Json& json) -> std::unique_ptr<Option>;
        
        CircuitInfo circuit {};
        std::optional<Stimulations> stimulations {};
    };

    auto configCretae(const std::filesystem::path& configPath) -> std::unique_ptr<Option>;   

}

DESERIALIZE_ENUM(xtaro::creator::VolatgeType, 
    DE_VALUE_AS(Digit, "digit") 
    DE_VALUE_AS(Analog, "analog") 
)

DESERIALIZE_STRUCT(xtaro::creator::DCStimulation,
    DE_OPTION_FILED(type)
    DE_FILED(voltage)
)

DESERIALIZE_STRUCT(xtaro::creator::PeriodStimulation,
    DE_OPTION_FILED(type)
    DE_OPTION_FILED_AS(timeBias, time_bias)
    DE_FILED(voltages)
)

DESERIALIZE_STRUCT(xtaro::creator::PWLStimulation,
    DE_OPTION_FILED(type)
    DE_FILED(voltages)
)

DESERIALIZE_STRUCT(xtaro::creator::Environment,
    DE_OPTION_FILED(pvt)
    DE_OPTION_FILED(slew)
    DE_OPTION_FILED(load)
    DE_OPTION_FILED(period)
)

DESERIALIZE_STRUCT(xtaro::creator::Stimulations,
    DE_OPTION_FILED(environment)
    DE_OPTION_FILED_AS(addCLK, add_clk)
    DE_OPTION_FILED_AS(dcStimuls, dc_stimulations)
    DE_OPTION_FILED_AS(periodStimuls, period_stimulations)
    DE_OPTION_FILED_AS(pwlStimuls, pwl_stimulations)
)

DESERIALIZE_STRUCT(xtaro::creator::CircuitInfo,
    DE_FILED(type)
    DE_FILED(arguments)
)

DESERIALIZE_STRUCT(xtaro::creator::Option,
    DE_FILED(circuit)
    DE_OPTION_FILED(stimulations)
)