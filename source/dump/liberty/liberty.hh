#pragma once

#include <characterize/characterizer/setuphold.hh>
#include <characterize/characterizer/delay.hh>
#include <filesystem>
#include <global/env/pvt.hh>
#include <global/env/slewload.hh>
#include <global/env/slews.hh>
#include <format>
#include <fstream>
#include <utility>
#include <vector>

namespace xtaro::dump
{

    class Liberty 
    {
    public:
        Liberty(
            circuit::SRAM* sram,
            PVT pvt,
            std::vector<Number> slews,
            std::vector<Number> loads,
            std::unordered_map<SlewLoad, characterize::DelayResult> delayResults,
            std::unordered_map<Slews, characterize::SetupHoldResult> setupholdResults,
            std::filesystem::path outputPath 
        );

    public:
        void write();

    private:
        void writeLibrary();
        void writeCell();

    private:
        void writeUnits();
        void writeDefaults();
        void writeLUTTemplates();
        void writeBus();
        void writePGPin();

        void writeWordBus();
        void writeWordBusInput();
        void writeWordBusOutput();

        void writeAdderBus();
        void writeSetupHold();
        
        void writeControlPins();
        
    private:
        void writeIndex(int number, const std::vector<Number>& values);

        void writeOutputCharacter(int indent);
        void writeDelayResult(int indent, Number characterize::DelayResult::* attr);

        void writeInputCharacter(int indent);
        void writeSetupHoldResult(int indent, Number characterize::SetupHoldResult::* attr);

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
        circuit::SRAM* _sram;
        PVT _pvt;

        std::vector<Number> _slews;
        std::vector<Number> _loads;

        std::unordered_map<SlewLoad, characterize::DelayResult> _delayResults;
        std::unordered_map<Slews, characterize::SetupHoldResult> _setupholdResults;

        std::ofstream _libfile;
    };

}