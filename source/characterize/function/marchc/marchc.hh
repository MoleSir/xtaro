#pragma once

#include "../function.hh"

namespace xtaro::characterize
{

    class MarchCTester : public FunctionTester
    {
    public:
        MarchCTester(circuit::SRAM* sram, PVT pvt, SlewLoad slewload);
        virtual ~MarchCTester() noexcept = default;

    protected:
        virtual void generateTransactions() override; 
    };

}