#pragma once

#include <filesystem>

namespace xtaro::circuit
{
    class SRAM;
}

namespace xtaro::dump
{
    
    class Verilog
    {
    public:
        Verilog(circuit::SRAM* sram, std::filesystem::path outputFolderPath);

        void write();
        
    private:
        circuit::SRAM* _sram;
        std::filesystem::path _outputFolderPath;
    };

}