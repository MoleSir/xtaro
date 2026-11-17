#include "./random.hh"

#include "characterize/simulator/sramsimulator.hh"
#include "utility/misc/random.hh"
#include <global/number/number.hh>

#include <unordered_set>
#include <utility/misc/string.hh>
#include <utility/misc/math.hh>
#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <utility/debug/debug.hh>
#include <vector>

namespace xtaro::characterize {

    RandomTester::RandomTester(circuit::SRAM* sram, PVT pvt, SlewLoad slewload) :
        FunctionTester(sram, std::move(pvt), std::move(slewload)),

        // MARK: 
        _readAddressSize{ static_cast<int>(0.1 * sram->wordSize()) }
    {
    }

    /*

        1. 生成 N 个地址

        2. 先给 N 个地址写入初始值，并且在写入一个地址后马上插入对应的读操作！

        3. 开始随机产生读/写操作，但读信号地址必须来自这 N 个地址，写信号可以不需要

        在产生至少 2*N 个读操作后结束。

    */
    void RandomTester::generateTransactions()
    {
        // 生成 N 个地址
        debug::debug("Generate Transactions");
        auto addresses = this->generateRandomAddresses();
        
        // 先给 N 个地址写入初始值，并且在写入一个地址后马上插入对应的读操作！
        for (auto address : addresses) {
            this->addWriteTransaction(address, this->_simulator->randomWord());
            this->addReadTransaction(address);
        }

        // 开始随机产生读/写操作，但读信号地址必须来自这 N 个地址，写信号可以不需要
        while (this->_simulator->readTransactionSize() <= 2 * this->_readAddressSize)
        {
            auto oper = this->_simulator->randomOperation();
            if (oper == SRAMOperation::Write)
            {
                auto address = this->_simulator->randomAddress();
                auto word = this->_simulator->randomWord();
                this->addWriteTransaction(address, word);
            }
            else 
            {
                auto addressIndex = util::random<std::size_t>(0, addresses.size() - 1);
                auto address = addresses.at(addressIndex);
                this->addReadTransaction(address);
            }
        }
    }

    auto RandomTester::generateRandomAddresses() -> std::vector<unsigned int>
    {
        auto addressSet = std::unordered_set<unsigned int>{};
        while (addressSet.size() < this->_readAddressSize) {
            auto addr = this->_simulator->randomAddress();
            // debug::debugf("{} : {}", addressSet.size(), this->_readAddressSize);
            addressSet.insert(addr);
        }

        auto addresses = std::vector<unsigned int>{};
        for (auto address : addressSet) 
            addresses.emplace_back(address);

        return addresses;
    }

}