#pragma once

#include "../function.hh"

namespace xtaro::characterize
{

    class MATSTester : public FunctionTester
    {
    public:
        MATSTester(circuit::SRAM* sram, PVT pvt, SlewLoad slewload);
        virtual ~MATSTester() noexcept = default;

    protected:
        virtual void generateTransactions() override; 
    };

}