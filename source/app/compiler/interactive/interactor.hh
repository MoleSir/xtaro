#pragma once

#include <format>
#include <global/env/pvt.hh>
#include <global/env/slewload.hh>
#include <global/env/slews.hh>
#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include "../option/option.hh"

#include <string>
#include <span>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <list>
#include <utility>

namespace xtaro::circuit {
    class SRAM;
}

namespace xtaro::compiler
{

    class InteractiveExit {};

    class Interactor
    {
        using CommandMethod = bool (Interactor::*) (std::span<std::string_view const>);

        struct Command 
        {
            std::string_view name;
            std::string_view usage;
            std::string_view description;
            Interactor::CommandMethod method;
        };
        
        static const std::unordered_map<std::string_view, Command> methodTable;

    public:
        auto getCommandInput() -> std::string;
        bool interprete(std::string input);

        template <typename... Args>
        bool interpretef(std::format_string<Args...> fmt, Args &&... args) {
            auto input = std::format(std::move(fmt), std::forward<Args>(args)...);
            return this->interprete(std::move(input));
        }

        auto option() const -> const std::unique_ptr<Option>&
        { return this->_option; }

    private:
        auto parseInput(std::string input) -> std::tuple<CommandMethod, std::vector<std::string_view>>;
        bool execute(CommandMethod method, std::span<std::string_view const> arguments);

    public:
        /*
            source <commandfile>
        */
        bool source(std::span<std::string_view const>);

        /*
            config <optionfile>
        */
        bool config(std::span<std::string_view const>);

        /*
            set <var_name> <var_value>
        */
        bool set(std::span<std::string_view const>);

        /*
            compile
        */
        bool compile(std::span<std::string_view const>);
        
        /*
            reset
        */
        bool reset(std::span<std::string_view const>);

        /*
            write [--spice] [--verilog] [--liberty]
        */
        bool write(std::span<std::string_view const>);
        
        /*
            command <process> <voltage> <temperature> <slew> <load>
        */
        bool functionTest(std::span<std::string_view const>);
        
        /*
            command <process> <voltage> <temperature> <slew> <load>
        */
        bool calculateDelay(std::span<std::string_view const>);
        
        /*
            setuphold <process> <voltage> <temperature> <slew> <slew>
        */
        bool calculateSetuphold(std::span<std::string_view const>);
        
        bool exit(std::span<std::string_view const>);
        bool clear(std::span<std::string_view const>);

        /*
            help [command_name]
        */
        bool help(std::span<std::string_view const>);

        /*
            logo [color]
        */
        bool logo(std::span<std::string_view const>);

    private:
        void checkConfig() const;
        void checkCompile() const;

    private:
        static void throwError(std::string_view command, const std::exception& err);
        static void clearCurrentInput(std::string& commandLine);

    private:
        std::list<std::string> _historyCommands{"", ""};

        circuit::SRAM* _sram{nullptr};
        std::unique_ptr<Option> _option{nullptr};
    };

}