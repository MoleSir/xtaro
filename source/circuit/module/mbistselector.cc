#include "mbistselector.hh"

#include <circuit/module/selector.hh>
#include <circuit/module/fanoutbuff.hh>

#include <circuit/mod.hh>
#include <tech/tech.hh>

#include <utility/misc/string.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    std::string MBISTSelector::Arguments::toString() const
    {
        return std::format("aw{}_ww{}", this->addressWidth, this->wordWidth);
    }

    auto MBISTSelector::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<MBISTSelector>(circuitName, *this);
    }

    MBISTSelector::MBISTSelector(std::string_view name, const MBISTSelector::Arguments& arguments) :
        Module{name},
        _addressWidth{arguments.addressWidth},
        _wordWidth{arguments.wordWidth},
        _fanoutBufSize{2 + this->_addressWidth + this->_wordWidth},
        _selector{nullptr},
        _fanoutbuf{nullptr}
    {
        debug::checkf(this->_addressWidth >= 1, "Address width '{}' < 1", this->_addressWidth);
        debug::checkf(this->_wordWidth >= 1, "Word width '{}' < 1", this->_wordWidth);

        this->createSelf();
    }

    void MBISTSelector::createPorts()
    {
        this->addPortNet("mbist", PortType::Input);

        this->addPortNet("csb", PortType::Input);
        this->addPortNet("we", PortType::Input);

        for (int i = 0; i < this->_addressWidth; ++i)
            this->addPortNet(stringf("addr{}", i), PortType::Input);

        for (int i = 0; i < this->_wordWidth; ++i)
            this->addPortNet(stringf("din{}", i), PortType::Input);

        this->addPortNet("bist_csb", PortType::Input);
        this->addPortNet("bist_we", PortType::Input);

        for (int i = 0; i < this->_addressWidth; ++i)
            this->addPortNet(stringf("bist_addr{}", i), PortType::Input);

        for (int i = 0; i < this->_wordWidth; ++i)
            this->addPortNet(stringf("bist_din{}", i), PortType::Input);

        this->addPortNet("sram_csb", PortType::Output);
        this->addPortNet("sram_we", PortType::Output);

        for (int i = 0; i < this->_addressWidth; ++i)
            this->addPortNet(stringf("sram_addr{}", i), PortType::Output);

        for (int i = 0; i < this->_wordWidth; ++i)
            this->addPortNet(stringf("sram_din{}", i), PortType::Output);  

        this->addPortNet(tech->vdd(), PortType::InOut); 
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void MBISTSelector::createCircuits()
    {
        this->_selector = this->addCircuit(Selector::Arguments {1});
        this->_fanoutbuf = this->addCircuit(FanoutBuffer::Arguments {this->_fanoutBufSize});
    } 

    void MBISTSelector::createInstances() 
    {
        // FanoutBuffer
        std::vector<std::string_view> fanoutNets {};
        fanoutNets.emplace_back("mbist");
        for (int i = 0; i < this->_fanoutBufSize; ++i)
            fanoutNets.emplace_back(stringf("mbist_buf_{}", i));
        fanoutNets.emplace_back(tech->vdd());
        fanoutNets.emplace_back(tech->gnd());

        this->addInstance(FanoutBuffer::TYPE, this->_fanoutbuf, fanoutNets);
        auto mbistBuf {fanoutNets.cbegin()};

        std::vector<std::string_view> addrInputNets{};
        std::vector<std::string_view> addrMBISTNets{};
        std::vector<std::string_view> addrNets{};

        std::vector<std::string_view> wordInputNets{};
        std::vector<std::string_view> wordMBISTNets{};
        std::vector<std::string_view> wordNets{};

        // Build input signals nets.
        // 'addrInputNets[n]' and 'addrMBISTNets[n]' will input into 2-1 Mux, selected by 'mbist'.
        // Give out 'addrNets[n]'.
        /*
                            +---+---+
                            |       |
           inputNets[n] --->+   M   |
                            |   U   +---> nets[n]
           mbistNets[n] --->+   X   |
                            |       |
                            +---+---+
                                ^
                                |
                        mbist --+
        */

        for (int i = 0; i < this->_addressWidth; ++i)
        {
            addrInputNets.emplace_back(stringf("addr{}", i));
            addrMBISTNets.emplace_back(stringf("bist_addr{}", i));
            addrNets.emplace_back(stringf("sram_addr{}", i));
        }

        for (int i = 0; i < this->_wordWidth; ++i)
        {
            wordInputNets.emplace_back(stringf("din{}", i));
            wordMBISTNets.emplace_back(stringf("bist_din{}", i));
            wordNets.emplace_back(stringf("sram_din{}", i));
        }

        for (int i = 0; i < this->_addressWidth; ++i)
        {
            this->addInstance(
                stringf("addr_sel_{}", i),
                this->_selector,
                {*(++mbistBuf), addrInputNets[i], addrMBISTNets[i], addrNets[i], tech->vdd(), tech->gnd()}
            );
        }

        for (int i = 0; i < this->_wordWidth; ++i)
        {
            this->addInstance(
                stringf("word_sel_{}", i),
                this->_selector,
                {*(++mbistBuf), wordInputNets[i], wordMBISTNets[i], wordNets[i], tech->vdd(), tech->gnd()}
            );
        }

        this->addInstance(
            "csb_sel",
            this->_selector,
            {*(++mbistBuf), "csb", "bist_csb", "sram_csb", tech->vdd(), tech->gnd()}
        ); 

        this->addInstance(
            "we_sel",
            this->_selector,
            {*(++mbistBuf), "we", "bist_we", "sram_we", tech->vdd(), tech->gnd()}
        ); 
    }

    void MBISTSelector::calculateLayoutConstants()
    {
        debug::unimplement("MBISTSelector::calculateLayoutConstants");
    }

    void MBISTSelector::placeInstances()
    {
        debug::unimplement("MBISTSelector::placeInstances");
    }

    void MBISTSelector::routeInstances()
    {
        debug::unimplement("MBISTSelector::placeInstances");
    }

    void MBISTSelector::createLayoutPins()
    {
        debug::unimplement("MBISTSelector::createLayoutPins");
    }
    
}