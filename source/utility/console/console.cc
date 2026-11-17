#include "console.hh"
#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>

namespace xtaro::console
{
    static void print_level(Color color, std::string_view level)
    {
        std::cout << '[';
        printWithColor(level, color);
        std::cout << "]: "; 
    }

    void printWithColor(std::string_view message, Color color) 
    {
        const char* colorInfo = nullptr;
        switch (color) {
            case Color::Black:  colorInfo = "\e[1;30m"; break;
            case Color::Red:    colorInfo = "\e[1;31m"; break;
            case Color::Green:  colorInfo = "\e[1;32m"; break;
            case Color::Yellow: colorInfo = "\e[1;33m"; break;
            case Color::Blue:   colorInfo = "\e[1;34m"; break;
            case Color::Purple: colorInfo = "\e[1;35m"; break;
            case Color::Cyan:   colorInfo = "\e[1;36m"; break;
            case Color::White:  colorInfo = "\e[1;37m"; break;
        }
        std::cout << colorInfo << message << "\e[0m" << std::ends;
    }

    void print(std::string_view message) {
        std::cout << message << std::ends;
    }

    auto inputln(std::string_view prompt) -> std::string  {
        auto line = std::string{};
        std::cout << prompt;
        std::getline(std::cin, line);
        return line;
    }

    auto inputCharNoEcho() -> char {
        struct termios oldt, newt;
        char ch;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        std::cin.get(ch);
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return ch;
    }

    void printlnWithColor(std::string_view message, Color color) {
        printWithColor(message, color);
        std::cout << std::endl;
    }

    void println(std::string_view message) {
        std::cout << message << std::endl;
    }

    void debug(std::string_view message) {
        print_level(Color::Blue, " DEBUG ");
        println(message);
    }

    void info(std::string_view message) {
        print_level(Color::Green, " INFO  ");
        println(message);
    }

    void warning(std::string_view message) {
        print_level(Color::Yellow, "WARNING");
        println(message);
    }

    void error(std::string_view message) {
        print_level(Color::Red, " ERROR ");
        println(message);
    }

    void fatal(std::string_view message) {
        print_level(Color::Red, " FATAL ");
        println(message);
    }

}