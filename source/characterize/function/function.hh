#pragma once

#include "../simulator/sramsimulator.hh"
#include "global/number/number.hh"
#include <memory>
#include <string_view>

namespace xtaro::characterize
{
    class FunctionTester
    {
    public:
        static auto create(std::string_view policy, circuit::SRAM* sram, PVT pvt, SlewLoad slewload) -> std::unique_ptr<FunctionTester>;

    public:
        FunctionTester(circuit::SRAM* sram, PVT pvt, SlewLoad slewload);
        virtual ~FunctionTester() noexcept = default;
    
    public:
        bool test();

    protected:
        virtual void generateTransactions() = 0;

    protected:
        void writeTransactions();
        bool checkSimluationReuslts(const std::unordered_map<std::string, Number>& results) const;

        void addWriteTransaction(unsigned int address, unsigned int word);
        void addReadTransaction(unsigned int address);

        static bool valueApproximate(const Number& n1, const Number& n2);

    protected:
        std::unique_ptr<SRAMSimulator> _simulator;
        std::unordered_map<std::string, Number> _targetMeasResults;
    };
}