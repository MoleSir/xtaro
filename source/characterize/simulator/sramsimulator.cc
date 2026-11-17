#include "./sramsimulator.hh"
#include <global/number/number.hh>

#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <utility/misc/string.hh>
#include <utility/misc/math.hh>
#include <utility/debug/debug.hh>

namespace xtaro::characterize
{

    SRAMSimulator::SRAMSimulator(
        std::filesystem::path simulationFilename, 
        circuit::SRAM* sram, 
        PVT pvt, 
        SlewLoad slewload
    ) :
        CircuitSimulator {
            std::move(simulationFilename),
            sram,
            std::move(pvt)
        },
        
        _sram{sram},

        _period{tech->normalPeriod()},
        _slew{slewload.slew},
        _load{slewload.load},
    
        _transactions{},
        _readTransactionSize{0},
        _memoryState{},

        _addrMask{util::fullBitsNumber(sram->addressWidth())},
        _wordMask{util::fullBitsNumber(sram->wordWidth())},

        _engine{std::random_device{}()},
        _addrGenerator{0, this->hightestAddress()},
        _wordGenerator{0, this->fullWord()},
        _operGenerator{0, 1}
    {    
        this->writeLoads(this->_load);
        this->writeClock(this->_period, this->_slew);

        if (this->_sram->mbist())
        {
            this->writeDCStimulation("mbist", Number::zero());

            this->writeDCStimulation("bist_csb", this->_pvt.voltage);
            this->writeDCStimulation("bist_we", Number::zero());

            for (int i = 0; i < this->_sram->addressWidth(); ++i)
                this->writeDCStimulation(std::format("bist_addr{}", i), Number::zero());

            for (int i = 0; i < this->_sram->wordWidth(); ++i)
                this->writeDCStimulation(std::format("bist_din{}", i), Number::zero());
        }
    }

    bool SRAMSimulator::addWriteTransaction(unsigned int address, unsigned int word)
    {
        address = this->resetAddress(address);
        word = this->resetWord(word);

        // Add transcation
        this->_transactions.emplace_back(
            SRAMOperation::Write,
            this->addressBits(address),
            this->wordBits(word)
        );

        // Record memory state
        this->_memoryState[address] = this->wordBits(word);

        return true;
    }

    bool SRAMSimulator::addReadTransaction(unsigned int address)
    {
        address = this->resetAddress(address);

        if (!this->_memoryState.contains(address))
        {   
            debug::warningf("Try to read an unset address 0x{0:x}, this transaction will be ignored.", address);
            return false;
        }

        // Add transcation
        this->_transactions.emplace_back(
            SRAMOperation::Read,
            this->addressBits(address),
            Bits {}
        );

        this->_readTransactionSize += 1;

        return true;
    }

    auto SRAMSimulator::readMoment() const -> Number
    {
        return (this->_transactions.size() + 0.5) * this->_period;
    }

    auto SRAMSimulator::halfPeriodMoment() const -> Number
    {
        return (this->_transactions.size() + 0.5) * this->_period;
    }

    void SRAMSimulator::writeTransactions()
    {
        this->writeContent('\n');

        Number period {this->_period};
        Number vdd {this->_pvt.voltage};

        // First period is idle
        auto we = std::vector<Number>{};
        auto address = std::vector<std::vector<Number>>{static_cast<std::size_t>(this->_sram->addressWidth())};
        auto word = std::vector<std::vector<Number>>{static_cast<std::size_t>(this->_sram->wordWidth())};
        
        // Generate each PWL
        for (const SRAMTransaction& transaction : this->_transactions)
        {
            bool isWrite = transaction.operation == SRAMOperation::Write;

            // Write or Read
            we.emplace_back( isWrite ? vdd : Number::zero());

            // Address
            for (std::size_t i = 0; i < transaction.address.size(); ++i)
                address[i].emplace_back( transaction.address[i] ? vdd : Number::zero() );
            
            // Word
            if (isWrite)
            {
                for (std::size_t i = 0; i < transaction.word.size(); ++i)
                    word[i].emplace_back( transaction.word[i] ? vdd : Number::zero() );
            }
            else
            {
                for (std::vector<Number>& wordBit : word)
                    wordBit.emplace_back(Number::zero());
            }
        }

        // Write PWL
        this->writeStimulation("csb", {Number::zero()});
        this->writeStimulation("we", we);
        this->writeContent('\n');
        
        for (int i = 0; i < this->_sram->addressWidth(); ++i)
            this->writeStimulation(std::format("addr{}", i), address[i]);
        this->writeContent('\n');

        for (int i = 0; i < this->_sram->wordWidth(); ++i)
            this->writeStimulation(std::format("din{}", i), word[i]);
        this->writeContent('\n');
    
        // Write .TRAN
        this->writeTrans(10._p, Number::zero(), period * static_cast<double>(this->_transactions.size() + 2) );
        this->writeContent('\n');
    }

    void SRAMSimulator::writeStimulation(
        std::string_view portName, 
        const std::vector<Number>& voltages)
    {
        this->writePeriodStimulation(portName, voltages, this->_period, this->_slew, 0.0_n);
    }

    const Bits& SRAMSimulator::memory(unsigned int address) const
    {
        return this->_memoryState.find(address)->second;
    }

    bool SRAMSimulator::isWrittenMemory(unsigned int address) const
    {
        return this->_memoryState.find(address) != this->_memoryState.end();
    }

    unsigned int SRAMSimulator::resetAddress(unsigned int address) const
    {
        unsigned int newAddr {this->_addrMask & address};
        if (newAddr != address)
            debug::warningf("The address 0x{0:x} out of range(0x{0:x})", address, this->_addrMask);
        return newAddr;
    }
    
    unsigned int SRAMSimulator::resetWord(unsigned int word) const
    {
        unsigned int newWord {this->_wordMask & word};
        if (newWord != word)
            debug::warningf("The word 0x{0:x} out of range(0x{0:x})", word, this->_wordMask);
        return newWord;
    }

    std::string SRAMSimulator::
    writeTransactionComment(unsigned int address, unsigned int word) const
    {
        int cycle {static_cast<int>(this->_transactions.size())};
        return std::format(
            "Writing {:#x} to address {:#x} during cycle {}",
            word, address,
            cycle
        );
    }

    std::string SRAMSimulator::
    readTransactionComment(unsigned int address) const
    {
        int cycle {static_cast<int>(this->_transactions.size())};
        return std::format(
            "Reading address {:#x} during cycle {}",
            address,
            cycle
        );
    }

    auto SRAMSimulator::writeTransactionSize() const -> std::size_t
    {
        return this->_transactions.size() - this->_readTransactionSize;
    }

    auto SRAMSimulator::readTransactionSize() const -> std::size_t
    {
        return this->_readTransactionSize;
    }

    Bits SRAMSimulator::addressBits(unsigned int address) const
    {
        return SRAMSimulator::intToBits(address, this->_sram->addressWidth());
    }
    
    Bits SRAMSimulator::wordBits(unsigned int word) const
    {
        return SRAMSimulator::intToBits(word, this->_sram->wordWidth());
    }

    Bits SRAMSimulator::intToBits(unsigned int value, int size)
    {
        // Can't use {} to init... emm...
        Bits bits(size, false);

        for (int i = 0; i < size; ++i)
        {
            bool bit {0 != (value & 0x00000001U)};
            bits[i] = bit;
            value >>= 1;
        }

        return bits;
    }

    unsigned int SRAMSimulator::randomAddress()
    {
        return this->_addrGenerator(this->_engine);
    }
    
    unsigned int SRAMSimulator::randomWord()
    {
        return this->_wordGenerator(this->_engine);
    }

    SRAMOperation SRAMSimulator::randomOperation()
    {
        unsigned int oper {this->_wordGenerator(this->_engine)};
        return oper == 1 ? SRAMOperation::Write : SRAMOperation::Read;
    }

    auto SRAMSimulator::hightestAddress() -> unsigned int
    {
        return util::fullBitsNumber(this->_sram->addressWidth());
    }

    auto SRAMSimulator::fullWord() -> unsigned int
    {
        return util::fullBitsNumber(this->_sram->wordWidth());
    }

}