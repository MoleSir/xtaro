#include "./addrdecoder.hh"
#include "circuit/base/port.hh"

#include <circuit/module/decoder.hh>

#include <circuit/mod.hh>
#include <tech/tech.hh>

#include <utility/misc/string.hh>
#include <utility/misc/math.hh>
#include <utility/debug/debug.hh>
#include <string_view>
#include <vector>

namespace xtaro::circuit
{
    std::string AddressDecoder::Arguments::toString() const
    {
        return std::format("raw{}_caw", this->rowAddrWidth, this->colAddrWidth);
    }

    auto AddressDecoder::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<AddressDecoder>(circuitName, *this);
    }

    AddressDecoder::AddressDecoder(std::string_view name, const AddressDecoder::Arguments& arguments) :
        Module{name},
        _rowAddrWidth{arguments.rowAddrWidth},
        _colAddrWidth{arguments.colAddrWidth},

        _rowAddrDecoder{nullptr},
        _colAddrDecoder{nullptr},
        
        _rowSelSize{-1},
        _colSelSize{-1}
    {
        debug::checkf(this->_rowAddrWidth >= 2, "Row address size '{}' < 2", this->_rowAddrWidth);
        debug::checkf(this->_colAddrWidth >= 0, "Column address size '{}' < 0", this->_colAddrWidth);

        this->_rowSelSize = util::power(2, this->_rowAddrWidth);
        this->_colSelSize = util::power(2, this->_colAddrWidth);

        this->createSelf();
    }

    void AddressDecoder::createPorts()
    {
        for (int i = 0; i < this->_rowAddrWidth; ++i)
            this->addPortNet(stringf("raddr{}", i), PortType::Input);

        for (int i = 0; i < this->_rowSelSize; ++i)
            this->addPortNet(stringf("rsel{}", i), PortType::Output);

        if (this->_colAddrWidth > 0) 
        {
            for (int i = 0; i < this->_colAddrWidth; ++i)
                this->addPortNet(stringf("caddr{}", i), PortType::Input);

            for (int i = 0; i < this->_colSelSize; ++i)
                this->addPortNet(stringf("csel{}", i), PortType::Output);
        }
        
        // vdd & gnd
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void AddressDecoder::createCircuits()
    {
        this->_rowAddrDecoder = this->addCircuit(Decoder::Arguments{this->_rowAddrWidth});
        if (this->_colAddrWidth > 0) 
            this->_colAddrDecoder = this->addCircuit(Decoder::Arguments{this->_colAddrWidth});
    } 

    void AddressDecoder::createInstances() 
    {
        auto rowDecoderNets = std::vector<std::string_view>{};
        for (int i = 0; i < this->_rowAddrWidth; ++i)
            rowDecoderNets.emplace_back(stringf("raddr{}", i));
        for (int i = 0; i < this->_rowSelSize; ++i)
            rowDecoderNets.emplace_back(stringf("rsel{}", i));
        rowDecoderNets.emplace_back(tech->vdd()), rowDecoderNets.emplace_back(tech->gnd());
        this->addInstance("row_addr_decoder", this->_rowAddrDecoder, rowDecoderNets);

        if (this->_colAddrWidth > 0)
        {
            auto colDecoderNets = std::vector<std::string_view>{};
            for (int i = 0; i < this->_colAddrWidth; ++i)
                colDecoderNets.emplace_back(stringf("caddr{}", i));
            for (int i = 0; i < this->_colSelSize; ++i)
                colDecoderNets.emplace_back(stringf("csel{}", i));
            colDecoderNets.emplace_back(tech->vdd()), colDecoderNets.emplace_back(tech->gnd());
            this->addInstance("col_addr_decoder", this->_colAddrDecoder, colDecoderNets);
        }
    }

    void AddressDecoder::calculateLayoutConstants()
    {
        debug::unimplement("AddressDecoder::calculateLayoutConstants");
    }

    void AddressDecoder::placeInstances()
    {
        debug::unimplement("AddressDecoder::placeInstances");
    }

    void AddressDecoder::routeInstances()
    {
        debug::unimplement("AddressDecoder::placeInstances");
    }

    void AddressDecoder::createLayoutPins()
    {
        debug::unimplement("AddressDecoder::createLayoutPins");
    }

}