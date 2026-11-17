#pragma once

#include "./circuitsimulator.hh"

#include <circuit/module/sram.hh>

#include <cstddef>
#include <global/number/number.hh>
#include <global/env/pvt.hh>
#include <global/env/slewload.hh>
#include <global/env/slews.hh>

#include <filesystem>
#include <random>
#include <unordered_map>

namespace xtaro::characterize
{
    enum class SRAMOperation
    {
        Write,
        Read,
    };

    using Bits = std::vector<bool>;

    struct SRAMTransaction
    {
        SRAMTransaction(SRAMOperation op, Bits a, Bits w) : 
            operation{op}, 
            address{std::move(a)}, 
            word{std::move(w)} {}

        SRAMOperation operation;
        Bits address;
        Bits word;
    };

    class SRAMSimulator : public CircuitSimulator
    {
        using AddrGenerator = std::uniform_int_distribution<unsigned int>;
        using WordGenerator = std::uniform_int_distribution<unsigned int>;
        using OperGenerator = std::uniform_int_distribution<unsigned int>;

    public:
        SRAMSimulator(
            std::filesystem::path simulationFilename,
            circuit::SRAM* sram, 
            PVT pvt,
            SlewLoad slewload
        );

    public:
        bool addWriteTransaction(unsigned int address, unsigned int word);
        bool addReadTransaction(unsigned int address);
        void writeTransactions();

        auto memory(unsigned int address) const -> const Bits&;
        bool isWrittenMemory(unsigned int address) const;

        auto readMoment() const -> Number;
        auto halfPeriodMoment() const -> Number;

        auto randomAddress() -> unsigned int;
        auto randomWord() -> unsigned int;
        auto randomOperation() -> SRAMOperation;

        auto hightestAddress() -> unsigned int;
        auto fullWord() -> unsigned int;

        auto writeTransactionSize() const -> std::size_t;
        auto readTransactionSize() const -> std::size_t;
        
    private:
        auto resetAddress(unsigned int address) const -> unsigned int;
        auto resetWord(unsigned int word) const -> unsigned int;

        Bits addressBits(unsigned int address) const;
        Bits wordBits(unsigned int word) const;

        auto writeTransactionComment(unsigned int address, unsigned int word) const -> std::string;
        auto readTransactionComment(unsigned int address) const -> std::string;

        void writeStimulation(std::string_view portName, const std::vector<Number>& voltages);

    protected:
        static Bits intToBits(unsigned int value, int size);

    public:
        circuit::SRAM* sram() const noexcept
        { return this->_sram; }

        auto memoryState() const noexcept -> const std::unordered_map<unsigned int, Bits>&
        { return this->_memoryState; }

        auto period() const noexcept -> Number 
        { return this->_period; }

    protected:
        circuit::SRAM* _sram;
        
        Number _period;
        Number _slew;
        Number _load;

        std::vector<SRAMTransaction> _transactions;
        std::size_t _readTransactionSize;
        std::unordered_map<unsigned int, Bits> _memoryState;

        std::mt19937 _engine;
        AddrGenerator _addrGenerator;
        WordGenerator _wordGenerator;
        OperGenerator _operGenerator;

        const unsigned int _addrMask;
        const unsigned int _wordMask;
    };

}