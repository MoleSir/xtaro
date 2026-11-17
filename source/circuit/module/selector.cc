#include "selector.hh"

#include <circuit/module/decoder.hh>
#include <circuit/libmodule/and.hh>
#include <circuit/libmodule/or.hh>
#include <circuit/mod.hh>
#include <tech/tech.hh>

#include <utility/misc/string.hh>
#include <utility/misc/math.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    std::string Selector::Arguments::toString() const
    {
        return std::format("ss{}", this->selectionSize);
    }

    auto Selector::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<Selector>(circuitName, *this);
    }

    Selector::Selector(std::string_view name, const Selector::Arguments& arguments) :
        Module{name},
        _selectionSize{arguments.selectionSize},
        _inputSize{util::power(2, arguments.selectionSize)},
        _decoder{nullptr},
        _and{nullptr},
        _or{nullptr}
    {
        debug::checkf(this->_selectionSize >= 1, "Selection size '{}' < 1", this->_selectionSize);
        debug::checkf(this->_selectionSize <= Selector::MAX_SELECTION_SIZE, "Selection size '{}' > {}", this->_selectionSize, static_cast<int>(Selector::MAX_SELECTION_SIZE));

        this->createSelf();
    }

    void Selector::createPorts()
    {
        for (int i = 0; i < this->_selectionSize; ++i)
            this->addPortNet(stringf("S{}", i), PortType::Input);

        for (int i = 0; i < this->_inputSize; ++i)
            this->addPortNet(stringf("in{}", i), PortType::Input);

        this->addPortNet("out", PortType::InOut);
    
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void Selector::createCircuits()
    {
        // Create INV
        AND::Arguments invArguments {DriveAbility::X0, 2};
        this->_and = this->addCircuit(invArguments);

        // Create Decoder
        Decoder::Arguments decoderArgument {this->_selectionSize};
        this->_decoder = this->addCircuit(decoderArgument);

        // Create or
        OR::Arguments orArguments {DriveAbility::X0, this->_inputSize};
        this->_or = this->addCircuit(orArguments);
    }

    void Selector::createInstances() 
    {
        std::vector<std::string_view> selectNets {};
        std::vector<std::string_view> inputNets {};
        std::vector<std::string_view> decoderOutNets {};
        std::vector<std::string_view> outputNets {};

        for (int i = 0; i < this->_selectionSize; ++i)
            selectNets.emplace_back(stringf("S{}", i));
        for (int i = 0; i < this->_inputSize; ++i)
        {
            inputNets.emplace_back(stringf("in{}", i));
            decoderOutNets.emplace_back(stringf("Y{}", i));
            outputNets.emplace_back(stringf("out{}", i));
        }

        // Create decoder
        std::vector<std::string_view> decoderNets {};
        for (int i = 0; i < this->_selectionSize; ++i)
            decoderNets.emplace_back(selectNets[i]);
        for (int i = 0; i < this->_inputSize; ++i)
            decoderNets.emplace_back(decoderOutNets[i]);
        decoderNets.emplace_back(tech->vdd());
        decoderNets.emplace_back(tech->gnd());

        this->addInstance("decoder", this->_decoder, decoderNets);

        // Create and gates
        for (int i = 0; i < this->_inputSize; ++i)
        {
            this->addInstance(
                stringf("and{}", i),
                this->_and,
                {inputNets[i], decoderOutNets[i], outputNets[i], tech->vdd(), tech->gnd()}
            );
        }

        // Create or gates
        outputNets.emplace_back("out");
        outputNets.emplace_back(tech->vdd());
        outputNets.emplace_back(tech->gnd());
        this->addInstance("or", this->_or, outputNets);
    }

    void Selector::calculateLayoutConstants()
    {
        debug::unimplement("Selector::calculateLayoutConstants");
    }

    void Selector::placeInstances()
    {
        debug::unimplement("Selector::placeInstances");
    }

    void Selector::routeInstances()
    {
        debug::unimplement("Selector::placeInstances");
    }
    
    void Selector::createLayoutPins()
    {
        debug::unimplement("Selector::createLayoutPins");
    }

}