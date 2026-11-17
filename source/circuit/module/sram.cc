#include "sram.hh"

#include <circuit/module/addrdecoder.hh>
#include <circuit/module/inputdffs.hh>
#include <circuit/module/mbistselector.hh>
#include <circuit/module/sramcoreselector.hh>
#include <circuit/module/sramcore.hh>

#include <circuit/mod.hh>
#include <tech/tech.hh>

#include <utility/misc/string.hh>
#include <utility/misc/math.hh>
#include <utility/debug/debug.hh>

#include <algorithm>
#include <string_view>
#include <vector>

namespace xtaro::circuit
{


    std::string SRAM::Arguments::toString() const
    {
        return std::format("aw{}_ww{}_{}mbist", this->addressWidth, this->wordWidth, this->mbist ? "" : "no");
    }

    auto SRAM::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<SRAM>(circuitName, *this);
    }

    SRAM::SRAM(std::string_view name, const SRAM::Arguments& arguments) :
        Module{name},
        _addressWidth{arguments.addressWidth},
        _wordWidth{arguments.wordWidth},
        _mbist{arguments.mbist},

        // Not init yet
        _coreAddressWidth{-1},
        _rowAddressWidth{-1},
        _colAddressWidth{-1},

        _coreSize{-1},
        _coreRowSize{-1},
        _coreColSize{-1},

        _multipleCore{false},
        
        _inputDffs{nullptr},
        _mbistSelector{nullptr},
        _sramCoreSelector{nullptr},
        _sramCore{nullptr},
        _addrDecoder{nullptr}
    {
        // `SRAM::addressDistribution` function will check SRAM size!
        auto [coreAddrWdith, rowAddrWidth, colAddrWidth] 
            = SRAM::addressDistribution(this->_addressWidth, this->_wordWidth);
    
        this->_coreAddressWidth = coreAddrWdith;
        this->_rowAddressWidth = rowAddrWidth;
        this->_colAddressWidth = colAddrWidth;
        
        this->_coreSize = util::power(2,  this->_coreAddressWidth);
        this->_coreRowSize = util::power(2,  this->_rowAddressWidth);
        this->_coreColSize = util::power(2,  this->_colAddressWidth) * this->_wordWidth;

        this->_rowSelSize = util::power(2, this->_rowAddressWidth);
        this->_colSelSize = util::power(2, this->_colAddressWidth);

        this->_multipleCore = this->_coreSize > 1;

        this->createSelf();
    }

    void SRAM::createPorts()
    {
        this->addPortNet("clk", PortType::Input);

        if (this->_mbist)
            this->addPortNet("mbist", PortType::Input);

        this->addPortNet("csb", PortType::Input);
        this->addPortNet("we", PortType::Input);

        for (int i = 0; i < this->_addressWidth; ++i)
            this->addPortNet(stringf("addr{}", i), PortType::Input);

        for (int i = 0; i < this->_wordWidth; ++i)
            this->addPortNet(stringf("din{}", i), PortType::Input);

        if (this->_mbist) 
        {
            this->addPortNet("mbist_csb", PortType::Input);
            this->addPortNet("mbist_we", PortType::Input);

            for (int i = 0; i < this->_addressWidth; ++i)
                this->addPortNet(stringf("mbist_addr{}", i), PortType::Input);

            for (int i = 0; i < this->_wordWidth; ++i)
                this->addPortNet(stringf("mbist_din{}", i), PortType::Input);
        }

        for (int i = 0; i < this->_wordWidth; ++i)
            this->addPortNet(stringf("dout{}", i), PortType::Output);
        
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void SRAM::createCircuits()
    {
        InputDFFs::Arguments inputArgs {this->_addressWidth, this->_wordWidth};
        this->_inputDffs = this->addCircuit(inputArgs);

        if (this->_mbist)
        {
            MBISTSelector::Arguments selArgs {this->_addressWidth, this->_wordWidth};
            this->_mbistSelector = this->addCircuit(selArgs);
        }

        if (this->_multipleCore)
        {
            SRAMCoreSelector::Arguments cselArgs {this->_coreAddressWidth, this->_wordWidth};
            this->_sramCoreSelector = this->addCircuit(cselArgs);
        }

        SRAMCore::Arguments coreArgs {this->_rowSelSize, this->_colSelSize, this->_wordWidth};
        this->_sramCore = this->addCircuit(coreArgs);

        AddressDecoder::Arguments decoderArgs {this->_rowAddressWidth, this->_colAddressWidth};
        this->_addrDecoder = this->addCircuit(decoderArgs);

    }

    void SRAM::createInstances() 
    {
        // Ports
        std::vector<std::string_view> addrRegNets{};
        std::vector<std::string_view> addrInputNets{};
        std::vector<std::string_view> addrMBISTNets{};
        std::vector<std::string_view> addrNets{};

        std::vector<std::string_view> wordRegNets{};
        std::vector<std::string_view> wordInputNets{};
        std::vector<std::string_view> wordMBISTNets{};
        std::vector<std::string_view> wordNets{};

        for (int i = 0; i < this->_addressWidth; ++i)
        {
            addrRegNets.emplace_back(stringf("addr{}_r", i));
            addrInputNets.emplace_back(stringf("addr{}", i));
            addrMBISTNets.emplace_back(stringf("mbist_addr{}", i));
            addrNets.emplace_back(stringf("sram_addr{}", i));
        }

        for (int i = 0; i < this->_wordWidth; ++i)
        {
            wordRegNets.emplace_back(stringf("din{}_r", i));
            wordInputNets.emplace_back(stringf("din{}", i));
            wordMBISTNets.emplace_back(stringf("mbist_din{}", i));
            wordNets.emplace_back(stringf("sram_din{}", i));
        }

        // Create selectors
        if (this->_mbist)
        {
            std::vector<std::string_view> mbistSelNets {"mbist"};

            mbistSelNets.emplace_back("csb");
            mbistSelNets.emplace_back("we");

            for (const auto& net : addrInputNets)
                mbistSelNets.emplace_back(net);

            for (const auto& net : wordInputNets)
                mbistSelNets.emplace_back(net);

            mbistSelNets.emplace_back("mbist_csb");
            mbistSelNets.emplace_back("mbist_we");

            for (const auto& net : addrMBISTNets)
                mbistSelNets.emplace_back(net);

            for (const auto& net : wordMBISTNets)
                mbistSelNets.emplace_back(net);

            mbistSelNets.emplace_back("sram_csb");
            mbistSelNets.emplace_back("sram_we");

            for (const auto& net : addrNets)
                mbistSelNets.emplace_back(net);

            for (const auto& net : wordNets)
                mbistSelNets.emplace_back(net);

            mbistSelNets.emplace_back(tech->vdd());
            mbistSelNets.emplace_back(tech->gnd());

            this->addInstance("mbist_selector", this->_mbistSelector, mbistSelNets);
        }

        // For each input, use an DFF to save
        std::vector<std::string_view> inputDffsNets {"clk"};
        if (this->_mbist)
        {
            inputDffsNets.emplace_back("sram_csb");
            inputDffsNets.emplace_back("sram_we");

            for (const auto& net : addrNets)
                inputDffsNets.emplace_back(net);

            for (const auto& net : wordNets)
                inputDffsNets.emplace_back(net);
        }
        else
        {
            inputDffsNets.emplace_back("csb");
            inputDffsNets.emplace_back("we");

            for (const auto& net : addrInputNets)
                inputDffsNets.emplace_back(net);

            for (const auto& net : wordInputNets)
                inputDffsNets.emplace_back(net);
        }

        inputDffsNets.emplace_back("csb_r");
        inputDffsNets.emplace_back("we_r");

        for (const auto& net : addrRegNets)
            inputDffsNets.emplace_back(net);

        for (const auto& net : wordRegNets)
            inputDffsNets.emplace_back(net);

        inputDffsNets.emplace_back(tech->vdd());
        inputDffsNets.emplace_back(tech->gnd());

        this->addInstance("input_dffs", this->_inputDffs, inputDffsNets);

        // SRAM Core
        std::vector<std::string_view> coreCSBNets {};
        for (int coreIdx = 0; coreIdx < this->_coreSize; ++coreIdx)
            coreCSBNets.emplace_back(stringf("csb{}", coreIdx));

        std::vector<std::vector<std::string_view>> coreDoutNets {};
        for (int coreIdx = 0; coreIdx < this->_coreSize; ++coreIdx)
        {
            std::vector<std::string_view> nets {}; 
            for (int bit = 0; bit < this->_wordWidth; ++bit)
                nets.emplace_back(stringf("dout{}[{}]", coreIdx, bit));
            coreDoutNets.emplace_back(std::move(nets));
        }

        // SRAM Cores
        if (this->_multipleCore)
        {
            for (int coreIdx = 0; coreIdx < this->_coreSize; ++coreIdx)
            {
                std::vector<std::string_view> nets {"clk"};
                nets.emplace_back(coreCSBNets[coreIdx]);
                nets.emplace_back("we_r");

                for (int i = 0; i < this->_rowSelSize; ++i)
                    nets.emplace_back(stringf("rsel{}", i));

                if (this->_colAddressWidth > 0) 
                {
                    for (int i = 0; i < this->_colSelSize; ++i)
                        nets.emplace_back(stringf("csel{}", i));
                }

                for (const auto& net : wordRegNets)
                    nets.emplace_back(net);

                for (const auto& net : coreDoutNets[coreIdx])
                    nets.emplace_back(net);

                nets.emplace_back(tech->vdd()), nets.emplace_back(tech->gnd());
            
                this->addInstance(
                    stringf("core{}", coreIdx),
                    this->_sramCore,
                    nets
                );
            }

            // SRAM Core selector
            std::vector<std::string_view> coreSelNets {"csb_r"};
            for (int i = (this->_rowAddressWidth + this->_colAddressWidth); i < this->_addressWidth; ++i)
                coreSelNets.emplace_back(addrRegNets[i]);
            for (const auto& coreNets : coreDoutNets)
                for (const auto& net : coreNets)
                    coreSelNets.emplace_back(net);
            for (const auto& net : coreCSBNets)
                coreSelNets.emplace_back(net);
            for (int bit = 0; bit < this->_wordWidth; ++bit)
                coreSelNets.emplace_back(stringf("dout{}", bit));
            coreSelNets.emplace_back(tech->vdd()), coreSelNets.emplace_back(tech->gnd());
            
            this->addInstance("sram_core_selector", this->_sramCoreSelector, coreSelNets);
        }
        else 
        {
            // Only a Core!
            std::vector<std::string_view> nets {"clk"};
            nets.emplace_back("csb_r");
            nets.emplace_back("we_r");

            for (int i = 0; i < this->_rowSelSize; ++i)
                nets.emplace_back(stringf("rsel{}", i));

            if (this->_colAddressWidth > 0) 
            {
                for (int i = 0; i < this->_colSelSize; ++i)
                    nets.emplace_back(stringf("csel{}", i));
            }

            for (auto& net : wordRegNets)
                nets.emplace_back(net);

            for (int bit = 0; bit < this->_wordWidth; ++bit)
                nets.emplace_back(stringf("dout{}", bit));

            nets.emplace_back(tech->vdd()), nets.emplace_back(tech->gnd());
        
            this->addInstance(
                "core",
                this->_sramCore,
                nets
            );
        }

        // Decoder
        std::vector<std::string_view> decoderNets {};
        for (int i = 0; i < this->_rowAddressWidth; ++i)
            decoderNets.emplace_back(addrRegNets[i + this->_colAddressWidth]);
        for (int i = 0; i < this->_coreRowSize; ++i)
            decoderNets.emplace_back(stringf("rsel{}", i));

        if (this->_colAddressWidth > 0) 
        {
            for (int i = 0; i < this->_colAddressWidth; ++i)
                decoderNets.emplace_back(addrRegNets[i]);
            for (int i = 0; i < this->_colSelSize; ++i)
                decoderNets.emplace_back(stringf("csel{}", i));
        }

        decoderNets.emplace_back(tech->vdd()), decoderNets.emplace_back(tech->gnd());
        this->addInstance(AddressDecoder::TYPE, this->_addrDecoder, decoderNets);
    }

    auto SRAM::asOneCore(int addrWdith, int wordWidth) -> std::optional<int> 
    {
        auto arrayConfig = std::vector<std::tuple<int, int, int, int>>();
        auto maxColAddress = SRAM::MAX_COL_ADDRESS <= addrWdith - 1 ? SRAM::MAX_COL_ADDRESS : addrWdith - 1;

        // Generate all possible configs
        for (int colAddrWidth = 0; colAddrWidth <= maxColAddress; ++colAddrWidth)
        {
            auto row = util::power(2, addrWdith - colAddrWidth);
            auto col = wordWidth * util::power(2, colAddrWidth);
            auto delta = std::abs(row - col);
            arrayConfig.emplace_back(row, col, colAddrWidth, delta);
        }
        
        // Sort by delta 
        std::sort(
            arrayConfig.begin(), arrayConfig.end(),
            [] (const std::tuple<int, int, int, int>& left, const std::tuple<int, int, int, int>& right) -> bool {
                return std::get<3>(left) < std::get<3>(right);
            }
        );

        // Find first config with satisfy constraints
        for (const auto& [row, col, colAddrWidth, _] : arrayConfig) 
        {
            if (row <= SRAMCore::MAX_CORE_ROW && col <= SRAMCore::MAX_CORE_COL)
                return std::optional<int>{ colAddrWidth };
        }

        // This (A, W) can't build as one core!
        return std::nullopt;
    }

    // Retun (core address width, col address width)
    auto SRAM::addressDistribution(int addrWdith, int wordWidth) -> std::tuple<int, int, int>
    {
        // Maybe can use constexpr?
        static const int MAX_SRAM_BIT_SIZE = 
            util::power(2, SRAM::MAX_CORE_ADDRESS) * SRAMCore::MAX_CORE_BIT_SIZE; 

        // 1. Calculate total bit 
        auto totalBit = util::power(2, addrWdith) * wordWidth;

        // 2. Out of range 
        debug::checkf(
            totalBit <= MAX_SRAM_BIT_SIZE, 
            "Bit-cell size '{}' out of range '{}'", totalBit, MAX_SRAM_BIT_SIZE
        );

        // 3. Try Core address from 0 => max
        for (int coreAddrWidth = 0; coreAddrWidth <= SRAM::MAX_CORE_ADDRESS; ++coreAddrWidth)
        {
            // Try as one core
            auto result = asOneCore(addrWdith - coreAddrWidth, wordWidth);
            if (!result.has_value()) 
                continue;
            auto colAddrWdith = result.value();
            auto rowAddrWidth = addrWdith - colAddrWdith - coreAddrWidth;
            return { coreAddrWidth,  rowAddrWidth, colAddrWdith };
        }
    
        // Fail
        debug::exceptionf(
            "Can't find valid address distribution for the option of (address width: {}, word width: {})",
            addrWdith,
            wordWidth
        );
    }

    void SRAM::calculateLayoutConstants()
    {
        debug::unimplement("SRAM::calculateLayoutConstants");
    }

    void SRAM::placeInstances()
    {
        debug::unimplement("SRAM::placeInstances");
    }

    void SRAM::routeInstances()
    {
        debug::unimplement("SRAM::placeInstances");
    }

    void SRAM::createLayoutPins()
    {
        debug::unimplement("SRAM::createLayoutPins");
    }

}