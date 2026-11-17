#include "./mod.hh"
#include "./config/config.hh"
#include "./tech/tech.hh"

#include <exception>
#include <parse/json/json.hh>
#include <utility/debug/debug.hh>

namespace xtaro::global
{

    auto configSystem(const std::filesystem::path& configFile) -> void
    try
    {
        auto filename = configFile.filename().string();
        if (filename.ends_with("json")) 
        {
            auto json = parse::Json::loadFromFile(configFile);
            configSystem(json);
        }
        else 
        {
            debug::exception("Unsupport config file type");
        }
    }
    catch (const std::exception& e) 
    {
        debug::exceptionIn("Config System", e.what());
    }

    auto configSystem(const parse::Json& json) -> void
    {
        debug::info("Initial config");
        Config::initial(json);

        debug::info("Inital technology");
        Tech::initial(config->techPath);
    }

}