#include "./interactor.hh"

#include <exception>
#include <format>
#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <utility/console/console.hh>
#include <utility/misc/string.hh>
#include <utility/misc/sys.hh>
#include <utility/debug/debug.hh>

#include <span>
#include <cstdlib>
#include <cstring>
#include <cstdio>

namespace xtaro::compiler
{

    bool Interactor::interprete(std::string input)
    {
        debug::debugf("Interprete '{}'", input);
        auto [method, arguments] = this->parseInput(std::move(input));
        return this->execute(method, arguments);
    }

    auto Interactor::getCommandInput() -> std::string
    {
        console::print("xtaro> ");

        std::string input {};
        auto history {this->_historyCommands.begin()};

        bool finish{false};
        while (!finish)
        {
            auto ch = console::inputCharNoEcho();

            switch (ch)
            {
            case '\n':
            {
                console::println("");
                finish = true;
                break;
            }
            case 127:
            {
                if (input.empty()) break;
                input.pop_back();
                std::putchar('\b');
                std::putchar(' ');
                std::putchar('\b');
                break;
            }
            case '\33':
            {
                // Page Up and Page Down start with '\33', follow by '[A' or '[B'
                console::inputCharNoEcho();
                ch = console::inputCharNoEcho();
                
                switch (ch)
                {
                // Page Up
                case 'A':
                {
                    auto current = history;
                    if (++history == this->_historyCommands.end())
                        history = current;
                    break;
                }
                // Page Up
                case 'B':
                {
                    if (history != this->_historyCommands.begin())
                        --history;
                    break;
                }
                // Page Left
                case 'D':
                {
                    break;
                }
                case 'C':
                {
                    break;
                }
                default: break;
                }

                this->clearCurrentInput(input);
                input.assign(*history);
                console::print(input);

                continue;
            }
            default:
            {
                input.push_back(ch);
                std::putchar(ch);
                break;
            }
            }
        }

        return input;
    }

    void Interactor::checkConfig() const
    {
        if (this->_option.get() == nullptr)
            debug::exceptionIn("Check config", "The system has not been configed~");
    }

    void Interactor::checkCompile() const
    {
        if (this->_sram == nullptr)
            debug::exceptionIn("Check compile", "SRAM has not been compiled~");
    }

    void Interactor::clearCurrentInput(std::string& input)
    {
        for (std::size_t i = 0; i < input.size(); ++i)
        {
            std::putchar('\b');
            std::putchar(' ');
            std::putchar('\b');
        }
        input.clear();
    }

    bool Interactor::execute(CommandMethod method, std::span<std::string_view const> arguments)
    {
        if (method == nullptr) return false;
        return (this->*method)(arguments);
    }

    auto Interactor::parseInput(std::string input) -> std::tuple<CommandMethod, std::vector<std::string_view>>
    {
        if (input.empty())
            return std::make_tuple(nullptr, std::vector<std::string_view>{});

        // Save command line
        auto front = ++this->_historyCommands.begin();
        if (input != *front)
            this->_historyCommands.emplace(front, std::move(input));

        auto tokens = util::splitAll(' ', *(++this->_historyCommands.begin()));

        // First token is command
        auto commandName = tokens[0];
        auto result {Interactor::methodTable.find(commandName)};
        if (result == Interactor::methodTable.end())
        {
            debug::errorf("Unkonw command: '{}'~", tokens[0]);
            return std::make_tuple(nullptr, std::vector<std::string_view>{});
        }

        std::vector<std::string_view> arguments {};
        // Left tokens are arguments   
        for (std::size_t i = 1; i < tokens.size(); ++i)
            arguments.emplace_back(tokens[i]);       
        
        return std::make_tuple(result->second.method, std::move(arguments));
    }

    void Interactor::throwError(std::string_view command, const std::exception& err)
    {
        debug::exceptionIn(std::format("In command '{}'", command), err.what());
    }

}
