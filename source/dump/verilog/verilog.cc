#include "./verilog.hh"

#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <utility/debug/debug.hh>
#include <circuit/module/sram.hh>
#include <utility/misc/file.hh>

#include <format>
#include <fstream>
#include <iostream>
#include <filesystem>

namespace xtaro::dump
{

    Verilog::Verilog(circuit::SRAM* sram, std::filesystem::path outputFolderPath) :
        _sram{sram},
        _outputFolderPath{std::move(outputFolderPath)}
    {
    }

    void Verilog::write()
    {
        static auto verilogTemplatePath = std::filesystem::path{"./source/dump/template/verilog"};

        if (!this->_sram->mbist())
        {
            auto outfile = std::ofstream{ this->_outputFolderPath / std::format("{}.v", this->_sram->name().data()) }; 
            outfile << "module sram #(\n";
            outfile << "    parameter ADDR_WIDTH = " << this->_sram->addressWidth() << ',' << '\n';
            outfile << "    parameter DATA_WIDTH = " << this->_sram->wordWidth() << ',' << '\n';

            auto sramFilePath = verilogTemplatePath / "sram_no_bist.v";
            outfile << util::readFile(sramFilePath);
        }
        else
        {
            std::filesystem::copy_file(verilogTemplatePath / "bist/bist_check.v", this->_outputFolderPath / "bist_check.v");
            std::filesystem::copy_file(verilogTemplatePath / "bist/bist_state_machine.v", this->_outputFolderPath / "bist_state_machine.v");
            std::filesystem::copy_file(verilogTemplatePath / "bist/bist.v", this->_outputFolderPath / "bist.v");
            std::filesystem::copy_file(verilogTemplatePath / "sram/sram.v", this->_outputFolderPath / "sram.v");

            auto outfile = std::ofstream{this->_outputFolderPath / std::format("{}.v", this->_sram->name().data())};
            outfile << "module sram_bist #(\n";
            outfile << "    parameter ADDR_WIDTH = " << this->_sram->addressWidth() << ',' << '\n';
            outfile << "    parameter DATA_WIDTH = " << this->_sram->wordWidth() << ',' << '\n';
            
            auto sramFilePath = verilogTemplatePath / "sram_bist.v";
            outfile << util::readFile(sramFilePath);
        }
    }

}