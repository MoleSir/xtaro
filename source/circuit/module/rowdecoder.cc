#include "rowdecoder.hh"
#include "circuit/geometry/coord.hh"
#include "circuit/module/andarray.hh"
#include "circuit/module/wordlinederiverarr.hh"
#include "global/tech/layer.hh"
#include "global/tech/tech.hh"

#include <circuit/libmodule/and.hh>
#include <circuit/module/decoder.hh>
#include <circuit/module/fanoutbuff.hh>

#include <circuit/mod.hh>

#include <ranges>
#include <utility/misc/string.hh>
#include <utility/misc/math.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    std::string RowDecoder::Arguments::toString() const
    {
        return std::format("as{}", this->addressSize);
    }

    auto RowDecoder::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<RowDecoder>(circuitName, *this);
    }

    RowDecoder::RowDecoder(std::string_view name, const RowDecoder::Arguments& arguments) :
        Module{name},
        _addressSize{arguments.addressSize},
        _wlSize{util::power(2, this->_addressSize)}
    {
        debug::checkf(this->_addressSize >= 2, "Address size '{}' < 2", this->_addressSize);
        this->createSelf();
    }

    void RowDecoder::createPorts()
    {
        this->addPortNet(RowDecoder::WORDLINE_ENABLE_NAME, PortType::Input);

        // Input Ports
        for (int i = 0; i < this->_addressSize; ++i)
            this->addPortNet(RowDecoder::ADDRESS_NAME(i), PortType::Input);
        
        // Output Ports
        for (int i = 0; i < this->_wlSize; ++i)
            this->addPortNet(RowDecoder::WORDLINE_NAME(i), PortType::Output);

        // vdd & gnd
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void RowDecoder::createCircuits()
    {
        auto arrayArgument = ANDArray::Arguments {this->_wlSize};
        this->_andArray = this->addCircuit(arrayArgument);

        Decoder::Arguments decoderArgument{this->_addressSize};
        this->_decoder = this->addCircuit(decoderArgument);

        auto wdarratArgument = WordlineDriverArray::Arguments {16, this->_wlSize};
        this->_wordlineDriverArray = this->addCircuit(wdarratArgument);
    } 

    void RowDecoder::createInstances() 
    {
        auto yNets = std::vector<std::string_view>{};
        for (auto i : std::ranges::views::iota(0, this->_wlSize)) 
            yNets.emplace_back(stringf("Y{}", i));

        auto wlinNets = std::vector<std::string_view>{};
        for (auto i : std::ranges::views::iota(0, this->_wlSize)) 
            wlinNets.emplace_back(stringf("Y{}", i));

        // Create decoder instance
        auto decoderPorts = std::vector<std::string_view>{};

        for (auto i : std::ranges::views::iota(0, this->_addressSize))
            decoderPorts.emplace_back(RowDecoder::ADDRESS_NAME(i));

        for (auto i : std::ranges::views::iota(0, this->_wlSize)) 
            decoderPorts.emplace_back(yNets.at(i));
        
        decoderPorts.emplace_back(tech->vdd());
        decoderPorts.emplace_back(tech->gnd());

        this->_decoderInst = this->addInstance(Decoder::TYPE, this->_decoder, decoderPorts);

        // Create AND array
        auto andArrayNets = std::vector<std::string_view>{};

        for (auto i : std::ranges::views::iota(0, this->_wlSize))
             andArrayNets.emplace_back(yNets.at(i));

        andArrayNets.emplace_back(RowDecoder::WORDLINE_ENABLE_NAME);
        
        for (auto i : std::ranges::views::iota(0, this->_wlSize))
             andArrayNets.emplace_back(wlinNets.at(i));

        andArrayNets.emplace_back(tech->vdd());
        andArrayNets.emplace_back(tech->gnd());

        this->_andArrayInst = this->addInstance(ANDArray::TYPE, this->_andArray, andArrayNets);
    
        // Create wordline wriver
        auto wdArrayNets = std::vector<std::string_view>{};

        for (auto i : std::ranges::views::iota(0, this->_wlSize))
             wdArrayNets.emplace_back(wlinNets.at(i));

        for (auto i : std::ranges::views::iota(0, this->_wlSize))
             wdArrayNets.emplace_back(RowDecoder::WORDLINE_NAME(i));

        wdArrayNets.emplace_back(tech->vdd());
        wdArrayNets.emplace_back(tech->gnd());

        this->_wordlineDriverArrayInst = this->addInstance(WordlineDriverArray::TYPE, this->_wordlineDriverArray, wdArrayNets);
    }

    void RowDecoder::calculateLayoutConstants()
    {
        this->_routeLayer = Layer::Metal2;
        this->_instGap = 3 * tech->layerPitch(this->_routeLayer);

        debug::check(
            this->_decoder->height() == this->_andArray->height(), 
            "Decoder height should equal to ANDArray height");

        this->_width = 
            this->_decoder->width() + 
            this->_andArray->width() + 
            this->_wordlineDriverArray->width() + 
            2 *  this->_instGap;
        this->_height = this->_decoder->height();
    }

    void RowDecoder::placeInstances()
    {
        this->_decoderInst->place(
            Coord{0._u, 0._u});

        this->_andArrayInst->place(
            Coord{this->_decoder->width() + this->_instGap, 0._u});

        this->_wordlineDriverArrayInst->place(
            Coord{this->_decoder->width() + this->_andArray->width() + 2 * this->_instGap, 0._u});

        // VDD: Check layer
        for (auto vddPin : this->_decoderInst->getPins(tech->vdd())) {
            this->addRectangleSegmentCenter(
                vddPin.layer(),
                Coord {0._u, vddPin.cenY()},
                Coord {this->width(), vddPin.cenY()}
            );
        }

        for (auto gndPin : this->_decoderInst->getPins(tech->gnd())) {
            this->addRectangleSegmentCenter(
                gndPin.layer(),
                Coord {0._u, gndPin.cenY()},
                Coord {this->width(), gndPin.cenY()}
            );
        }
    }

    void RowDecoder::routeInstances()
    {
        for (auto i = 0; i < this->_wlSize; ++i)
        {
            auto wlInputPin = this->_decoderInst->getPin(Decoder::OUTPUT_NAME(i));
            auto wlOutputPin = this->_andArrayInst->getPin(ANDArray::INPUT_NAME(i));
            this->routePinsCenter(wlInputPin, wlOutputPin, this->_routeLayer);

            wlInputPin = this->_andArrayInst->getPin(ANDArray::OUTPUT_NAME(i));
            wlOutputPin = this->_wordlineDriverArrayInst->getPin(WordlineDriverArray::WORDLINE_INPUT_NAME(i));
            this->routePinsCenter(wlInputPin, wlOutputPin, this->_routeLayer);
        }
    }
    
    void RowDecoder::createLayoutPins()
    {
        for (auto i = 0; i < this->_addressSize; ++i)
            this->copyInstancePins(this->_decoderInst, Decoder::ADDRESS_NAME(i), RowDecoder::ADDRESS_NAME(i));

        for (auto i = 0; i < this->_wlSize; ++i) 
            this->copyInstancePin(this->_wordlineDriverArrayInst, WordlineDriverArray::WORDLINE_OUTPUT_NAME(i), RowDecoder::WORDLINE_NAME(i));
    
        this->copyInstancePin(this->_andArrayInst, ANDArray::ENABLE_NAME, RowDecoder::WORDLINE_ENABLE_NAME);
        this->copyInstancePins(this->_decoderInst, tech->vdd(), tech->vdd());
        this->copyInstancePins(this->_decoderInst, tech->gnd(), tech->gnd());
    }

    auto RowDecoder::ADDRESS_NAME(int index) -> std::string_view
    {
        return stringf("A{}", index);
    }

    auto RowDecoder::WORDLINE_NAME(int index) -> std::string_view
    {
        return stringf("wl{}", index);
    }

}