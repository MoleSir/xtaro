#include "./interactive/interactor.hh"
#include "../constant.hh"
#include "argparse.h"

#include <exception>
#include <format>
#include <utility/debug/debug.hh>
#include <utility/console/console.hh>
#include <utility/misc/file.hh>
#include <circuit/mod.hh>
#include <global/config/config.hh>

#include <argparse>

#include <optional>
#include <dump/mod.hh>
#include <string_view>
#include <tuple>

using namespace std::literals;

namespace xtaro::compiler 
{

    static auto parse_argument(int argc, const char* argv[]) -> argparse::ArgumentParser
    {
        auto parser = argparse::ArgumentParser{"xtaro_compiler", "SRAM Compiler"};
        parser.add_argument()
            .names({"-c", "--config"})
            .description("config file path")
            .required(true)
            .position(0);
        parser.add_argument()
            .names({"-l", "--log"})
            .description("log file output file")
            .required(false);
        parser.add_argument()
            .names({"-i", "--interactive"})
            .description("use interactive mode")
            .required(false);
        parser.add_argument()
            .names({"-v", "--verbose"})
            .description("verbose level")
            .required(false);

        parser.enable_help();
        auto err = parser.parse(argc, argv);
        if (err) xtaro::debug::exception(err.what());
        return parser;
    }

    static auto directCompile(std::string_view configFilepath) -> void
    {
        auto xtaro = Interactor{};

        xtaro.interpretef("config {}", configFilepath);
        xtaro.interprete("compile");
        if (xtaro.option()->functionTestPolicy.has_value())
        {
            auto testPass = xtaro.interpretef("function_test {} {} {} {} {} {}",
                *xtaro.option()->functionTestPolicy,
                "TT"sv,
                tech->normalVoltage(),
                tech->normalTemperature(),
                tech->dffRiseTime(),
                tech->normalCapacitance()
            );

            if (!testPass) 
            {
                debug::infof("Your SRAM test failed, do yout still want to generate output files?");
                auto outputFile = true;
                while (true) 
                {
                    auto answer = console::inputln("Input yes(Y/y) or no(N/n): ");
                    if (answer == "yes" || answer == "y" || answer == "Y") 
                    {
                        outputFile = true;
                        break;
                    } 
                    else if (answer == "no" || answer == "N" || answer == "n")
                    {
                        outputFile = false;
                        break;
                    }
                }

                if (!outputFile)
                    return;
            }
        }
        xtaro.interprete("write --spice --verilog --liberty");
    }

    static auto interactCompile(std::optional<std::string_view> sourcePath) -> void
    {
        debug::info("Run compile mode in interactive");

        auto xtaro = Interactor{};
        if (sourcePath.has_value())
        {
            try
            {
                xtaro.interprete(std::format("source {}", sourcePath.value()));
            }
            catch (const std::exception& err) 
            {
                debug::error(err.what());
            }
            catch (const InteractiveExit& err)
            {
                debug::info("Exit from interactive mode.");
            }
            return;
        }
        else
        {
            while (true)
            {            
                try
                {
                    auto input = xtaro.getCommandInput();
                    xtaro.interprete(std::move(input));
                }
                catch (const std::exception& err) 
                {
                    debug::error(err.what());
                }
                catch (const InteractiveExit& err)
                {
                    debug::info("Exit from interactive mode.");
                    break;
                }
            }
        }
    }

    static auto interactInfo(argparse::ArgumentParser& argparser) -> std::tuple<bool, std::optional<std::string>> {
        if (!argparser.exists("i")) 
        {
            return std::make_tuple(false, std::nullopt);
        } 
        else 
        {
            auto sourceFilepath = argparser.get<std::string>("i");
            if (sourceFilepath.empty())
                return std::make_tuple(true, std::nullopt);
            else
                return std::make_tuple(true, std::optional{std::move(sourceFilepath)});
        }
    }

    static void initSystem(argparse::ArgumentParser& argparser) 
    {
        if (argparser.exists("log")) 
            debug::initialLog(argparser.get<std::string>("log"));
        else
            debug::initialLog("./xtaro_compiler.log");
        
        if (argparser.exists("verbose")) 
        {
            auto levelStr = argparser.get<std::string>("verbose");
            auto level = debug::stringToDebugLevel(levelStr);
            if (level.has_value())
                debug::setDebugLevel(*level);
            else
                debug::exceptionf("No support debug level '{}'", levelStr);
        }

        circuit::initial();
    }

    static void printHelp(argparse::ArgumentParser& argparser) 
    {
        console::println("");
        console::printlnWithColor(xtarologo, console::Color::Blue);
        console::printlnf("\nOpen source: {}\n", xtarorepo);
        argparser.print_help();
    }

    void main(int argc, const char* argv[]) 
    {
        auto argparser = parse_argument(argc, argv);
        if (argparser.exists("help"))
            return printHelp(argparser);

        initSystem(argparser);
        auto [isInteract, sourcePath] = interactInfo(argparser);
        if (!isInteract) 
            directCompile(argparser.get<std::string>("config"));
        else
            interactCompile(sourcePath);
    }

}