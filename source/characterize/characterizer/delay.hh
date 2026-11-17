#pragma once

#include "../simulator/sramsimulator.hh"

namespace xtaro::characterize
{

    /*

        For each output bit, delay_hl, delay_lh, slew_lh and slew_hl four delay info need be cal

        - delay_lh: Read '1'
        - delay_hl: Read '0' (delay_lh = delay_hl)
        - slew_lh: Read '1'
        - delay_hl: Read '0'

        So we just need to write a address to 0 and read it.

        1. Choise some address.
        2. For All port, wirte it to 0 and read it, measure how many time it cost from clk to dout
           and how many time it cost in falling.
        3. Average the times and get final delay

    */

    struct DelayResult 
    {
        Number delayHL;
        Number delayLH;
        Number slewHL;
        Number slewLH;
    };

    class DelayCharacterizer
    {
    public:
        DelayCharacterizer(circuit::SRAM* sram, PVT pvt, SlewLoad slewload);
        
    public:
        auto analyze() -> DelayResult;

    private:
        auto generateRandomAddresses() -> std::unordered_set<unsigned int>;
        
        void writeTransactionsAndMeas(const std::unordered_set<unsigned int>& addresses);
        void addTransactions(unsigned int address);
        void writeMeas(unsigned int address);
        
        auto calulateResult(const std::unordered_map<std::string, Number>& result) -> DelayResult;

    private:
        std::unique_ptr<SRAMSimulator> _simulator;
    };

}