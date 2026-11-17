#include "./option.hh"

#include <parse/json/json.hh>
#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <global/mod.hh>
#include <utility/debug/debug.hh>

namespace xtaro::creator
{

    auto configCretae(const std::filesystem::path& configPath) -> std::unique_ptr<Option>
    {
        parse::Json json {parse::Json::loadFromFile(configPath)};
        global::configSystem(json);
        return Option::load(json);
    } 

    auto Option::load(const std::filesystem::path& optionFile) -> std::unique_ptr<Option>
    {
        parse::Json json {parse::Json::loadFromFile(optionFile)};
        return Option::load(json);
    }

    auto Option::load(const parse::Json& json) -> std::unique_ptr<Option>
    {
        auto option = std::make_unique<Option>();
        parse::Deserialize<parse::Json, Option>::from(json, *option);
        return option;
    }
}