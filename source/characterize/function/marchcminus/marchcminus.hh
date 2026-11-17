#pragma once

#include "../function.hh"

namespace xtaro::characterize
{

    class MarchCMinusTester : public FunctionTester
    {
    public:
        MarchCMinusTester(circuit::SRAM* sram, PVT pvt, SlewLoad slewload);
        virtual ~MarchCMinusTester() noexcept = default;

    protected:
        virtual void generateTransactions() override; 
    };

}