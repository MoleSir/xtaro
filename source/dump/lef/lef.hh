#pragma once

#include "circuit/geometry/blockage.hh"
#include <circuit/base/port.hh>
#include <fstream>
#include <filesystem>
#include <string_view>
#include <format>

namespace xtaro::circuit 
{
    class Circuit;
}

namespace xtaro::dump
{

    class Lef 
    {
        static constexpr int UNITS = 2000;

    public:
        Lef(circuit::Circuit* circuit, std::filesystem::path outputPath);

    public:
        void write();

    private:
        void writeHeader(int indent);
        void writePin(int indent, std::string_view name); 
        void writeObstructions(int indent = 0);
        void writeFooter(int indent);

    private:
        static auto portTypeToString(circuit::PortType type) -> std::string_view;
        void writeBlockage(int indent, const circuit::Blockage& blockage);

    private:
        template<typename... Args>
        void writeLinef(int indent, std::format_string<Args...> fmt, Args&&... args) {
            auto str = std::format(std::move(fmt), std::forward<Args>(args)...);
            this->writeLine(indent, str);
        }

        void writeLine(int indent, std::string_view content);
        void writeLine(int indent, char ch);
        void writeEnter();

    private:
        circuit::Circuit* _circuit {nullptr};
        std::ofstream _leffile;
    }; 

}