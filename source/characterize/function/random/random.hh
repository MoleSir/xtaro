#pragma once

#include "../function.hh"

namespace xtaro::characterize
{

    class RandomTester : public FunctionTester
    {
    public:
        RandomTester(circuit::SRAM* sram, PVT pvt, SlewLoad slewload);
        virtual ~RandomTester() noexcept = default;

    protected:
        virtual void generateTransactions() override; 

    private:
        auto generateRandomAddresses() -> std::vector<unsigned int>;

    private:
        const int _readAddressSize;
    };

}