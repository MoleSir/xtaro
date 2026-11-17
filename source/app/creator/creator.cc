#include "./option/option.hh"
#include "../constant.hh"
#include "argparse.h"
#include <circuit/base/circuit.hh>
#include <global/number/number.hh>

#include <circuit/mod.hh>
#include <characterize/simulator/circuitsimulator.hh>
#include <utility/debug/debug.hh>
#include <utility/console/console.hh>
#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <dump/mod.hh>
#include <verify/mod.hh>
#include <argparse>
#include <memory>
#include <string_view>

namespace xtaro::creator
{

    static auto parse_argument(int argc, const char* argv[]) -> argparse::ArgumentParser
    {
        auto parser = argparse::ArgumentParser{"xtaro_creator", "Creator Circuit"};
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
            .names({"-v", "--verbose"})
            .description("verbose level")
            .required(false);

        parser.enable_help();
        auto err = parser.parse(argc, argv);
        if (err) xtaro::debug::exception(err.what());
        return parser;
    }

    static auto loadOption(std::string_view configFilepath) -> std::unique_ptr<Option>
    try
    {        
        return configCretae(configFilepath);
    }
    catch (const std::exception& err)
    {
        debug::exceptionIn("Load option", err.what());
    }

    static auto writeCircuit(const std::unique_ptr<Option>& option) -> circuit::Circuit*
    try
    {
        auto circuit = circuit::circuit(
            option->circuit.type, 
            option->circuit.arguments
        );

        dump::writeSpice(circuit);
        if (config->buildBackend)
        {
            dump::writeGDSII(circuit);
            dump::writeLef(circuit);
        }
    
        return circuit;
    }
    catch (const std::exception& err)
    {
        debug::exceptionIn("Write circuit", err.what());
    }

    static auto verifyCircuit(circuit::Circuit* circuit) -> void 
    try
    {
        verify::runDrc("klayout", circuit, false, false);
        // verify::runLvs("klayout", circuit, false);
    }
    catch (const std::exception& err)
    {
        debug::exceptionIn("Verify circuit", err.what());
    }

    static auto writeStimulations(circuit::Circuit* circuit, const Stimulations& stimulations) -> void
    try
    {
        debug::info("Write spice stimulations");
        auto simulator = characterize::CircuitSimulator {
            config->spiceFolderPath / std::format("{}_test.sp", circuit->name()),
            circuit, 
            std::move(stimulations.environment.pvt)
        };

        if (stimulations.addCLK) 
            simulator.writeClock(stimulations.environment.period, stimulations.environment.slew);

        for (const auto& [portName, value] : stimulations.dcStimuls) 
        {
            auto voltage = (value.type == VolatgeType::Analog ? value.voltage : value.voltage * stimulations.environment.pvt.voltage);
            simulator.writeDCStimulation(portName, voltage); 
        }

        for (const auto& [portName, value] : stimulations.periodStimuls) 
        {
            auto voltages = std::vector<Number>{};
            for (const auto v : value.voltages) 
                voltages.emplace_back( value.type == VolatgeType::Analog ? v : v * stimulations.environment.pvt.voltage );
            simulator.writePeriodStimulation(
                portName, 
                voltages, 
                stimulations.environment.period, 
                stimulations.environment.slew, 
                value.timeBias
            );
        }

        for (const auto& [portName, value] : stimulations.pwlStimuls) 
        {
            auto voltages = std::map<Number, Number>{};
            for (const auto [t, v] : value.voltages)
                voltages.emplace(t, value.type == VolatgeType::Analog ? v : v * stimulations.environment.pvt.voltage);
            simulator.writePWLStimulation(portName, voltages);
        }

        simulator.writeContent('\n');
        simulator.writeContent('*');
        // TODO
        simulator.writeTrans(10._n, 0._n, 100._p);
    }
    catch (const std::exception& err)
    {
        debug::exceptionIn("Write stimulations", err.what());
    }

    static void initSystem(argparse::ArgumentParser& argparser) 
    {
        if (argparser.exists("log")) 
            debug::initialLog(argparser.get<std::string>("log"));
        else
            debug::initialLog("./xtaro_creator.log");

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
        auto option = loadOption(argparser.get<std::string>("config"));
        auto circuit = writeCircuit(option);
        verifyCircuit(circuit);

        if (option->stimulations.has_value())
            writeStimulations(circuit, option->stimulations.value());           
    }

}