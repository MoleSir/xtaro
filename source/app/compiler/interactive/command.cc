#include "./interactor.hh"
#include "../option/option.hh"

#include <circuit/mod.hh>
#include <circuit/module/sram.hh>
#include <characterize/mod.hh>
#include <dump/mod.hh>

#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <utility/misc/string.hh>
#include <utility/misc/sys.hh>
#include <utility/misc/file.hh>
#include <utility/debug/debug.hh>
#include <utility/console/console.hh>

#include <fstream>
#include <cstring>

namespace xtaro::compiler
{

    #define COMMAND(n, u, d, m)\
    {n, Interactor::Command{.name = n, .usage = u, .description = d, .method = m} },

    const std::unordered_map<std::string_view, Interactor::Command> Interactor::methodTable {
        COMMAND(
            "source", 
            "source <filepath>", 
            "Read each line in the file as a command input and excute them by order. If one command failed, the rest commands won't be executed.", 
            &Interactor::source
        )
        COMMAND(
            "config",
            "config <json_filepath>",
            "Read the json and load the user options and initial tech message.",
            &Interactor::config
        )
        COMMAND(
            "compile",
            "compile",
            "After 'config' success, create SRAM by user options.",
            &Interactor::compile
        )
        COMMAND(
            "reset",
            "reset",
            "Reset the System.",
            &Interactor::reset
        )
        COMMAND(
            "write",
            "write [--spice] [--verilog] [--liberty]",
            "After compiling the SRAM, write the specify file representing the SRAM.",
            &Interactor::write
        )
        COMMAND(
            "function_test",
            "function_test <process> <voltage> <temperature> <slew> <load>",
            "After compiling the SRAM, execute function test for SRAM in specify environment.",
            &Interactor::functionTest
        )
        COMMAND(
            "delay",
            "delay <process> <voltage> <temperature> <slew> <load>",
            "After compiling the SRAM, get delay infomation for SRAM in specify environment.",
            &Interactor::calculateDelay
        )
        COMMAND(
            "setuphold",
            "setuphold <process> <voltage> <temperature> <related_slew> <constrained_slew>",
            "After compiling the SRAM, get setup-hold infomation for SRAM in specify environment.",
            &Interactor::calculateSetuphold
        )
        COMMAND(
            "exit",
            "exit",
            "Exit XTaro.",
            &Interactor::exit
        )
        COMMAND(
            "clear",
            "clear",
            "Clear the terminal.",
            &Interactor::clear
        )
        COMMAND(
            "help",
            "help <command>",
            "Display hint for command",
            &Interactor::help
        )
        COMMAND(
            "logo",
            "logo [color]",
            "Show XTaro logo in specify color or default blue.",
            &Interactor::logo
        )
    };

    bool Interactor::source(std::span<std::string_view const> arguments)      
    try
    {
        if (arguments.size() != 1)
            debug::exceptionIn("Parse argument", "Needs one argument as command file~");

        const std::string filepath {arguments.front()};
        std::ifstream infile {filepath};
        if (!infile.is_open())
            debug::exceptionfIn("Open input file", "Can't read '{}'", filepath.c_str());
        
        std::string line;
        while (std::getline(infile, line))
        {
            if (line.empty()) continue;
            if (!this->interprete(std::move(line))) return false;
        }
        return true;
    } 
    catch (const std::exception& err)
    {
        Interactor::throwError("source", err);
        return false;
    }

    bool Interactor::config(std::span<std::string_view const> arguments)
    try
    {
        if (arguments.size() == 0)
            debug::exceptionIn("Parse argument", "Needs one argument as option file~");

        this->_option = compiler::config(arguments[0]);

        return true;
    }
    catch (const std::exception& err)
    {
        Interactor::throwError("config", err);
        // TODO: Reset option and tech
        return false;
    }

    bool Interactor::set(std::span<std::string_view const> arguments)
    try
    {
        return true;
    }
    catch (const std::exception& err)
    {
        Interactor::throwError("set", err);
        return false;
    }

    bool Interactor::compile(std::span<std::string_view const> arguments)
    try
    {
        this->checkConfig();

        debug::infof("Generate SRAM circuit");
        circuit::SRAM::Arguments argument {
            this->_option->addressWidth, 
            this->_option->wordWidth, 
            this->_option->mbist
        };
        this->_sram = dynamic_cast<circuit::SRAM*>(circuit::circuit(argument));

        return true;
    }
    catch (const std::exception& err) 
    {
        Interactor::throwError("compile", err);
        this->_sram = nullptr;
        return false;
    }

    bool Interactor::reset(std::span<std::string_view const> arguments)
    try
    {
        this->_sram = nullptr;
        auto _ = this->_option.release();
        circuit::reset();
        return true;
    }
    catch (const std::exception& err) 
    {
        Interactor::throwError("reset", err);
        return false;
    }

    bool Interactor::write(std::span<std::string_view const> arguments)
    try
    {
        this->checkCompile();

        auto spice = false;
        auto verilog = false;
        auto liberty = false;
        auto gdsii = false;
        auto lef = false;
        
        for (const auto& argument : arguments)
        {
            if (argument == "--spice")
                spice = true;
            else if (argument == "--verilog")
                verilog = true;
            else if (argument == "--liberty")
                liberty = true;
            else if (argument == "--lef")
            {
                if (!xtaro::config->buildBackend)
                    debug::warning("The backend do not build!");
                else
                    lef = true;
            }
            else if (argument == "--gdsii")
            {
                if (!xtaro::config->buildBackend)
                    debug::warning("The backend do not build!");
                else
                    gdsii = true;
            }

            else
                debug::exceptionfIn("Parse argument", "Unkown argument '{}' in 'write' command~", argument);
        }

        if (spice) dump::writeSpice(this->_sram);
        if (verilog) dump::writeVerilog(this->_sram);
        if (liberty) dump::writeLibertys(this->_sram, this->_option->pvts);
        if (gdsii) dump::writeGDSII(this->_sram);
        if (lef) dump::writeLef(this->_sram);

        return true;
    }
    catch (const std::exception& err) 
    {
        Interactor::throwError("write", err);
        return false;
    }

    bool Interactor::functionTest(std::span<std::string_view const> arguments)
    try
    {
        this->checkCompile();
        if (arguments.size() != 6)
            debug::exceptionIn("Parse argument", "Command arguments need six arguments~");

        auto policy = arguments[0];
        auto pvt = PVT::fromString(arguments.subspan(1, 3));
        auto slewload = SlewLoad::fromString(arguments.subspan(4, 2));
        return characterize::functionTest(policy, this->_sram, pvt, slewload);
    }
    catch (const std::exception& err)
    {
        Interactor::throwError("function", err);
        return false;
    }

    bool Interactor::calculateDelay(std::span<std::string_view const> arguments)
    try
    {
        this->checkCompile();
        if (arguments.size() != 5)
            debug::exceptionIn("Parse argument", "Command arguments need five arguments~");
            
        auto pvt = PVT::fromString(arguments.subspan(0, 3));
        auto slewload = SlewLoad::fromString(arguments.subspan(3, 2));
        characterize::displayDelay(this->_sram, pvt, slewload);

        return true;
    }
    catch (const std::exception& err)
    {
        Interactor::throwError("delay", err);
        return false;
    }

    bool Interactor::calculateSetuphold(std::span<std::string_view const> arguments)
    try
    {
        this->checkCompile();
        if (arguments.size() != 5)
            debug::exceptionIn("Parse argument", "Command arguments need five arguments~");

        auto pvt = PVT::fromString(arguments.subspan(0, 3));
        auto slews = Slews::fromString(arguments.subspan(3, 2));
        characterize::disaplySetuphold(pvt, slews);

        return true;
    }
    catch (const std::exception& err)
    {
        Interactor::throwError("setuphold", err);
        return false;
    }

    bool Interactor::exit(std::span<std::string_view const> arguments)
    {
        throw InteractiveExit{};
        return false;
    }

    bool Interactor::clear(std::span<std::string_view const> arguments)
    {
        util::execute("clear");
        return true;
    }

    bool Interactor::logo(std::span<std::string_view const> arguments)
    try 
    {
        // TODO
        return true;
    }
    catch (const std::exception& err)
    {
        Interactor::throwError("logo", err);
        return false;
    }

    bool Interactor::help(std::span<std::string_view const> arguments)
    try
    {
        if (arguments.size() == 0)
        {
            for (auto& [method, message] : Interactor::methodTable) 
            {
                console::printlnf("These are some hint for '{}':", method);
                console::printlnf("    Usage: {}", message.usage);
                console::printlnf("    {}", message.description);
            }
        }

        const auto& commandName = arguments.front();
        auto result = Interactor::methodTable.find(commandName);
        if (result == Interactor::methodTable.end()) 
        {
            debug::exceptionfIn("Search command", "Unkonw command '{}'", commandName);
        }
        else
        {
            console::printlnf("These are some hint for '{}':", commandName);
            console::printlnf("    Usage: {}", result->second.usage);
            console::printlnf("    {}", result->second.description);
        }

        return true;
    }
    catch (const std::exception& err)
    {
        Interactor::throwError("help", err);
        return false;
    }

}
