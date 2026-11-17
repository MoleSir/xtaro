#include "./option.hh"

#include <utility/console/console.hh>
#include <utility/debug/debug.hh>
#include <utility/misc/string.hh>
#include <utility/misc/file.hh>

#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <global/mod.hh>

#include <parse/serde/mod.hh>
#include <parse/json/json.hh>

#include <memory>
#include <string>

namespace xtaro::compiler
{

    auto config(const std::filesystem::path& configFile) -> std::unique_ptr<Option>
    {
        auto json = xtaro::parse::Json::loadFromFile(configFile);
        global::configSystem(json);
        return Option::load(json);
    }

    static void buildPVTList(std::unique_ptr<Option>& option);

    auto Option::load(const parse::Json& json) -> std::unique_ptr<Option>
    {
        auto option = std::make_unique<Option>();
        xtaro::parse::Deserialize<parse::Json, Option>::from(json, *option);

        buildPVTList(option);

        return option;
    }

    auto Option::load(const std::filesystem::path& configFile) -> std::unique_ptr<Option>
    {   
        auto json = parse::Json::loadFromFile(configFile);
        return Option::load(json);
    }

    static void buildPVTList(std::unique_ptr<Option>& option)
    {        
        for (const std::string& p : option->processCorners )
        {
            for (const auto& v : option->supplyVoltages)
            {
                for (const auto& t : option->temperatures)
                {
                    option->pvts.emplace_back(p, v, t);
                }
            }
        }
    }
}