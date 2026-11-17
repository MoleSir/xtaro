#include "sramcoreselector.hh"

#include <circuit/libmodule/inv.hh>
#include <circuit/module/decoder.hh>
#include <circuit/libmodule/or.hh>
#include <circuit/libmodule/and.hh>

#include <circuit/mod.hh>

#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <utility/misc/string.hh>
#include <utility/misc/math.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{

    auto SRAMCoreSelector::Arguments::toString() const -> std::string
    {
        return std::format("aw{}_ww{}", this->addressWidth, this->wordWidth);
    }

    auto SRAMCoreSelector::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<SRAMCoreSelector>(circuitName, *this);
    }

    SRAMCoreSelector::SRAMCoreSelector(std::string_view name, const SRAMCoreSelector::Arguments& arguments) :
        Module{name},
        _addressWidth{arguments.addressWidth},
        _wordWidth{arguments.wordWidth},
        _outputCSBSize{ util::power(2, arguments.addressWidth) },
        _decoder{nullptr},
        _inv{nullptr},
        _and{nullptr},
        _selectOr{nullptr}
    {
        debug::checkf(this->_addressWidth >= 1, "Address width '{}' < 0", this->_addressWidth);
        debug::checkf(this->_wordWidth >= 1, "Word width '{}' < 0", this->_wordWidth);

        this->createSelf();
    }

    void SRAMCoreSelector::createPorts()
    {
        this->addPortNet("csb", PortType::Input);

        for (int i = 0; i < this->_addressWidth; ++i)
            this->addPortNet(stringf("addr{}", i), PortType::Input);

        for (int coreIdx = 0; coreIdx < this->_outputCSBSize; ++coreIdx)
            for (int bit = 0; bit < this->_wordWidth; bit++)
                this->addPortNet(stringf("dout_core{}[{}]", coreIdx, bit), PortType::Input); 

        for (int coreIdx = 0; coreIdx < this->_outputCSBSize; ++coreIdx)
            this->addPortNet(stringf("csb{}", coreIdx), PortType::Output);

        for (int bit = 0; bit < this->_wordWidth; bit++)
            this->addPortNet(stringf("dout{}", bit), PortType::Output);

        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void SRAMCoreSelector::createCircuits()
    {
        this->_decoder = this->addCircuit(Decoder::Arguments {this->_addressWidth});
        this->_inv = this->addCircuit(INV::Arguments {DriveAbility::X1});
        this->_or = this->addCircuit(OR::Arguments {DriveAbility::X1, 2});
        this->_and = this->addCircuit(AND::Arguments {DriveAbility::X1, 2});
        this->_selectOr = this->addCircuit(OR::Arguments {DriveAbility::X1, this->_outputCSBSize});
    } 

    void SRAMCoreSelector::createInstances() 
    {
        std::vector<std::string_view> addrNets{};
        for (int i = 0; i < this->_addressWidth; ++i)
            addrNets.emplace_back(stringf("addr{}", i));
        
        std::vector<std::string_view> csbNets{};
        for (int i = 0; i < this->_outputCSBSize; ++i)
            csbNets.emplace_back(stringf("csb{}", i));

        std::vector<std::string_view> yNets {};
        for (int i = 0; i < this->_outputCSBSize; ++i)
            yNets.emplace_back(stringf("y{}", i));

        std::vector<std::string_view> ybNets{};
        for (int i = 0; i < this->_outputCSBSize; ++i)
            ybNets.emplace_back(stringf("ybar{}", i));

        std::vector<std::string_view> decoderNets {};
        for (const auto& net : addrNets) 
            decoderNets.emplace_back(net);
        for (const auto& net : yNets) 
            decoderNets.emplace_back(net);
        decoderNets.emplace_back(tech->vdd()), decoderNets.emplace_back(tech->gnd());

        this->addInstance("decoer", this->_decoder, decoderNets);

        // Add inv of all cs
        for (int i = 0; i < this->_outputCSBSize; ++i)
        {
            this->addInstance(
                stringf("csb_inv{}", i), 
                this->_inv, 
                { yNets[i], ybNets[i], tech->vdd(), tech->gnd() }
            );
        }

        // Add csb control
        for (int i = 0; i < this->_outputCSBSize; ++i)
        {
            this->addInstance(
                stringf("csb_or{}", i), 
                this->_or, 
                { "csb", ybNets[i], csbNets[i], tech->vdd(), tech->gnd() }
            );
        }

        // For each output bit
        for (int bit = 0; bit < this->_wordWidth; ++bit)
        {
            // Select dout_core0[bit], dout_core0[bit], dout_core0[bit] .. as dout[bit]
            // By y[0], y[1], y[2]... 
            for (int coreIdx = 0; coreIdx < this->_outputCSBSize; ++coreIdx) 
            {
                this->addInstance(
                    stringf("dout_and_{}_{}", coreIdx, bit),
                    this->_and,
                    {
                        stringf("y{}", coreIdx),
                        stringf("dout_core{}[{}]", coreIdx, bit),
                        stringf("y_dout_core{}[{}]", coreIdx, bit),
                        tech->vdd(), tech->gnd()
                    }
                );
            }

            // Add or
            std::vector<std::string_view> orNets;
            for (int coreIdx = 0; coreIdx < this->_outputCSBSize; ++coreIdx) 
                orNets.emplace_back(stringf("y_dout_core{}[{}]", coreIdx, bit));
            orNets.emplace_back(stringf("dout{}", bit));
            orNets.emplace_back(tech->vdd()), orNets.emplace_back(tech->gnd());
            this->addInstance(
                stringf("dout_or{}", bit),
                this->_selectOr,
                orNets
            );
        }
    }

    void SRAMCoreSelector::calculateLayoutConstants()
    {
        debug::unimplement("SRAMCoreSelector::calculateLayoutConstants");
    }

    void SRAMCoreSelector::placeInstances()
    {
        debug::unimplement("SRAMCoreSelector::placeInstances");
    }

    void SRAMCoreSelector::routeInstances()
    {
        debug::unimplement("SRAMCoreSelector::placeInstances");
    }

    void SRAMCoreSelector::createLayoutPins()
    {
        debug::unimplement("SRAMCoreSelector::createLayoutPins");
    }

}