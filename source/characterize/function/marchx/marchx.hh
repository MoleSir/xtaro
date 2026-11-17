#pragma once

#include "../function.hh"

namespace xtaro::characterize
{

    class MarchXTester : public FunctionTester
    {
    public:
        MarchXTester(circuit::SRAM* sram, PVT pvt, SlewLoad slewload);
        virtual ~MarchXTester() noexcept = default;

    protected:
        virtual void generateTransactions() override; 
    };

}