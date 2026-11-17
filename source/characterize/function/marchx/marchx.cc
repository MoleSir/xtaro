#include "./marchx.hh"

#include "characterize/simulator/sramsimulator.hh"
#include <global/number/number.hh>

#include <utility/misc/string.hh>
#include <utility/misc/math.hh>
#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <utility/debug/debug.hh>

namespace xtaro::characterize {

    MarchXTester::MarchXTester(circuit::SRAM* sram, PVT pvt, SlewLoad slewload) :
        FunctionTester(sram, std::move(pvt), std::move(slewload))
    {
    }

    void MarchXTester::generateTransactions()
    {        
        auto hightestAddress = this->_simulator->hightestAddress();
        auto fullWord = this->_simulator->fullWord();

        // 从地址 “低到高” 写 0
        for (unsigned int address = 0; address <= this->_simulator->hightestAddress(); address++)
            this->addWriteTransaction(address, 0);
        
        // 从地址 “低到高” 读 0 写 1
        for (unsigned int address = 0; address <= this->_simulator->hightestAddress(); address++) 
        {
            this->addReadTransaction(address);
            this->addWriteTransaction(address, fullWord);
        }

        // 从地址 “高到低” 读 1 写 0
        for (unsigned int address = hightestAddress; ; address--) 
        {
            this->addReadTransaction(address);
            this->addWriteTransaction(address, 0);

            if (address == 0) break;
        }

        // 从地址 “高到低” 读 0
        for (unsigned int address = hightestAddress; ; address--) 
        {
            this->addReadTransaction(address);
            if (address == 0)
                break;
        }
    }

}