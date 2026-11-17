#include "decoder.hh"
#include "circuit/base/instance.hh"
#include "circuit/base/module.hh"
#include "circuit/geometry/coord.hh"
#include "global/number/number.hh"
#include "global/tech/layer.hh"

#include <array>
#include <circuit/libmodule/and.hh>
#include <circuit/libmodule/inv.hh>

#include <circuit/mod.hh>

#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <ranges>
#include <string>
#include <utility/misc/string.hh>
#include <utility/misc/math.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{

    const std::array<std::vector<int>, Decoder::COMPONENT_SIZE> 
    Decoder::subDecodersInputSizes
    {
        std::vector<int>{2, 3}, 
        std::vector<int>{3, 3}, 
        std::vector<int>{3, 4}, 
        std::vector<int>{4, 4},
        std::vector<int>{3, 3, 3},
        std::vector<int>{3, 3, 4},
        std::vector<int>{3, 4, 4},
        std::vector<int>{4, 4, 4},
    };

    std::string Decoder::Arguments::toString() const
    {
        return std::format("is{}", this->inputSize);
    }

    auto Decoder::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<Decoder>(circuitName, *this);
    }

    Decoder::Decoder(std::string_view name, const Decoder::Arguments& arguments) :
        Module{name},
        _inputSize{arguments.inputSize},
        _outputSize{ util::power(2, this->_inputSize) },
        _decoderType{ Decoder::getDecoderType(arguments.inputSize) },
        _and{nullptr},
        _inv{nullptr}
    {
        debug::checkf(this->_inputSize >= Decoder::MIN_INPUT_SIZE, "Input size '{}' < {}", this->_inputSize, static_cast<int>(Decoder::MIN_INPUT_SIZE));
        debug::checkf(this->_inputSize <= Decoder::MAX_INPUT_SIZE, "Input size '{}' > {}", this->_inputSize, static_cast<int>(Decoder::MAX_INPUT_SIZE));

        this->createSelf();
    }

    void Decoder::createPorts()
    {
        // Input Ports
        for (int i = 0; i < this->_inputSize; ++i)
            this->addPortNet(this->ADDRESS_NAME(i), PortType::Input);
        
        // Output Ports
        int outputSize{ util::power(2, this->_inputSize) };
        for (int i = 0; i < outputSize; ++i)
            this->addPortNet(this->OUTPUT_NAME(i), PortType::Output);

        // vdd & gnd
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void Decoder::createCircuits()
    {   
        // Create inv
        this->_inv = this->addCircuit(INV::Arguments {DriveAbility::X1});

        // AND Gate 
        switch (this->_decoderType)
        {
            case DecoderType::OneAddr: break;
            case DecoderType::Simple: {
                AND::Arguments andArguments{DriveAbility::X1, this->_inputSize};
                this->_and = this->addCircuit(andArguments);
                break;
            }
            case DecoderType::Componet: {
                int andGateInputSize = this->subDecodersInputSize().size();
                AND::Arguments andArguments{DriveAbility::X1, andGateInputSize};
                this->_and = this->addCircuit(andArguments);

                this->createSubDecoders();
                break;
            }
        }
    }

    void Decoder::createInstances() 
    {
        switch (this->_decoderType)
        {
            case DecoderType::OneAddr:  this->createOneAddrInstance(); break;
            case DecoderType::Simple:   this->createSimpleInstances(); break;
            case DecoderType::Componet: this->createComponetInstances(); break;
        }
    }

    void Decoder::createSubDecoders()
    {
        const std::vector<int>& subDecodersInputSize{this->subDecodersInputSize()};

        for (std::size_t i = 0; i < subDecodersInputSize.size(); ++i)
        {
            Decoder::Arguments arguments {subDecodersInputSize.at(i)};
            auto decoder = dynamic_cast<Decoder*>(this->addCircuit(arguments));
            this->_subDecoders.emplace_back(decoder);
        }
    }

    void Decoder::createOneAddrInstance()
    {
        // One need a 'INV'
        this->_inv0Inst = this->addInstance("inv0", this->_inv, {
            this->ADDRESS_NAME(0), this->OUTPUT_NAME(0), 
            tech->vdd(), tech->gnd()});
        this->_inv1Inst = this->addInstance("inv1", this->_inv, {
            this->OUTPUT_NAME(0), this->OUTPUT_NAME(1), 
            tech->vdd(), tech->gnd()});
    }

    void Decoder::createSimpleInstances()
    {
        std::vector<std::string_view> inputPorts{};
        std::vector<std::string_view> inputPortsInt{};
        for (int i = 0; i < this->_inputSize; ++i)
        {
            inputPorts.emplace_back( this->ADDRESS_NAME(i) );
            inputPortsInt.emplace_back( stringf("A{}_bar", i) );
        }

        // Create INV for input 
        for (int i = 0; i < this->_inputSize; ++i)
        {
            auto inst = this->addInstance(stringf("inv{}", i), this->_inv, {inputPorts.at(i), inputPortsInt.at(i), tech->vdd(), tech->gnd()});
            this->_invInsts.emplace_back(inst);
        }

        // Create AND
        for (int i = 0; i < this->_outputSize; ++i)
        {
            // Well, the literal 'and' is a C++ keyword ...
            // Create nets list to connect with 'andInstance'
            std::vector<std::string_view> nets{};
            for (int j = 0; j < this->_inputSize; ++j)
            {
                // 'i' is the AND gate's index. Each AND gate's inputs are [A0/A0_int, A1/A1_int ... An/An_int]
                // There are '_inputSize' inputs, and 'j' is the input port' index.
                // No.'j' bit in 'i' decides the port for Aj is inverted or not.
                // For example, i == 000, the inputs are [A0_int, A1_int, A2_int].
                //              i == 010, the inputs are [A0_int, A1,     A2_int].
                // ...
                bool bitOne{ ((i >> j) & 0x1) != 0 };
                nets.emplace_back( bitOne ? inputPorts[j] : inputPortsInt[j] );
            }

            nets.emplace_back( this->OUTPUT_NAME(i) );
            nets.emplace_back(tech->vdd());
            nets.emplace_back(tech->gnd());

            auto inst = this->addInstance(stringf("and{}", i), this->_and, nets);
            this->_andInsts.emplace_back(inst);
        }
    }

    void Decoder::createComponetInstances()
    {
        // Create sub decoder
        const std::vector<int>& subDecodersInputSize{this->subDecodersInputSize()};
        int inputPortIndex{0};

        // Loop for each sub decoder
        for (std::size_t decoderIdx = 0; decoderIdx < subDecodersInputSize.size(); ++decoderIdx)
        {
            // Get sub decoder's input size
            int subDecoderInputSize{subDecodersInputSize[decoderIdx]};
            
            // Create decoder instance            
            // Connect nets
            // Create the nets' name
            std::vector<std::string_view> nets{};
            // Input ports
            for (int i = 0; i < subDecoderInputSize; ++i)
                nets.emplace_back( this->ADDRESS_NAME(inputPortIndex++) );
            // Output ports
            for (int outputIdx = 0; outputIdx < util::power(2, subDecoderInputSize); ++outputIdx)
                nets.emplace_back( stringf("Y_{}_{}", decoderIdx, outputIdx) );
            // vdd & gnd
            nets.emplace_back(tech->vdd());
            nets.emplace_back(tech->gnd());
            
            auto inst = this->addInstance(stringf("decoder{}", decoderIdx), this->_subDecoders[decoderIdx], nets);
            this->_subDecoderInsts.emplace_back(inst);
        }

        // Loop for each Output AND gate
        for (int andGateIdx = 0; andGateIdx < this->_outputSize; ++andGateIdx)
        {
            // Create AND gate instance        
            // Connect nets...
            std::vector<std::string_view> nets{};
            
            // For each decoder, get an output line as AND gate's input
            // emmm... this algo is hard to explain, so TODO!!!
            for (std::size_t decoderIdx = 0; decoderIdx < subDecodersInputSize.size(); ++decoderIdx)
            {
                int prefixSum{0};
                for (std::size_t idx = 0; idx < decoderIdx; ++idx)
                    prefixSum += subDecodersInputSize[idx];
                
                int mask{1};
                for (int i = 1; i < subDecodersInputSize[decoderIdx]; ++i)
                    mask = (mask << 1) + 1;

                int decoderOutputIdx{ (andGateIdx >> prefixSum) & mask };
                nets.emplace_back(stringf("Y_{}_{}", decoderIdx, decoderOutputIdx));
            }

            // Output & vdd & gnd
            nets.emplace_back(this->OUTPUT_NAME(andGateIdx));
            nets.emplace_back(tech->vdd());
            nets.emplace_back(tech->gnd());

            auto inst = this->addInstance(stringf("and{}", andGateIdx), this->_and, nets);
            this->_andInsts.emplace_back(inst);
        }
    }

    void Decoder::placeInstances()
    {
        switch (this->_decoderType)
        {
            case DecoderType::OneAddr:  this->placeOneAddrInstances(); break;
            case DecoderType::Simple:   this->placeSimpleInstances(); break;
            case DecoderType::Componet: this->placeComponetInstances(); break;
        }
    }

    void Decoder::placeOneAddrInstances() 
    {

    }

    void Decoder::placeSimpleInstances() 
    {
        Module::placeInstArrayInColumn(this->_invInsts, this->_invXOff, 0._u);
        Module::placeInstArrayInColumn(this->_andInsts, this->_andXOff, 0._u);
    }

    void Decoder::placeComponetInstances() 
    {
        // Place sub decoder
        auto offset = Coord {this->_subDecoderXOff, 0._u};
        for (auto inst : this->_subDecoderInsts) 
        {
            inst->place(offset);
            offset.y += inst->circuit()->height() + this->_subDecoderSpacing;
        }

        Module::placeInstArrayInColumn(this->_andInsts, this->_andXOff, 0._u);
    }

    void Decoder::routeInstances()
    {
        switch (this->_decoderType)
        {
            case DecoderType::OneAddr:  this->routeOneAddrInstances(); break;
            case DecoderType::Simple:   this->routeSimpleInstances(); break;
            case DecoderType::Componet: this->routeComponetInstances(); break;
        }

        // Route VDD/GND
        for (auto i : std::ranges::views::iota(0, this->_outputSize)
                         | std::ranges::views::filter([] (auto a) { return a % 2 == 0; })) 
        {
            auto inst = this->_andInsts.at(i);
            auto vddPin = inst->getPin(tech->vdd());
            this->addRectangleSegmentCenter(
                this->_horLayer,
                Coord {0._u, vddPin.cenY()},
                Coord {this->width(), vddPin.cenY()}
            );
            this->copyInstancePin(inst, tech->vdd());

            auto gndPin = inst->getPin(tech->gnd());
            this->addRectangleSegmentCenter(
                this->_horLayer,
                Coord {0._u, gndPin.cenY()},
                Coord {this->width(), gndPin.cenY()}
            );
            this->copyInstancePin(inst, tech->gnd());
        }

        auto lastInst = this->_andInsts.back();
        auto gndPin = lastInst->getPin(tech->gnd());
        this->addRectangleSegmentCenter(
            this->_horLayer,
            Coord {0._u, gndPin.cenY()},
            Coord {this->width(), gndPin.cenY()}
        );
        this->copyInstancePin(lastInst, tech->gnd());
    }

    void Decoder::routeOneAddrInstances() 
    {

    }

    void Decoder::routeSimpleInstances()
    {
        this->routeColBus(this->_busLayer, this->_busWidth, this->height(), 0._u, this->_invInputPinXOff);
        this->routeColBus(this->_busLayer, this->_busWidth, this->height(), 0._u, this->_andInputBarPinXOff);
        this->routeColBus(this->_busLayer, this->_busWidth, this->height(), 0._u, this->_andInputPinXOff);

        // Route input bus with inv inst
        for (int i = 0; i < this->_inputSize; ++i) 
        {
            auto inst = this->_invInsts.at(i);
            auto xOff = this->_invInputPinXOff.at(i);

            auto pin = inst->getPin(INV::INPUT_NAME);
            auto inputPinCoord = Coord {xOff + this->_busWidth / 2, pin.cenY()};
            this->routePinsCenter(inputPinCoord, this->_busLayer, pin.center(), pin.layer(), this->_horLayer);

            // MARK: hor layer?
            this->addPinCenter(
                this->_horLayer, 
                this->ADDRESS_NAME(i), 
                inputPinCoord);
        }

        // Route and bus to AND bus
        for (int i = 0; i < this->_outputSize; ++i)
        {
            auto inst = this->_andInsts.at(i);

            for (int j = 0; j < this->_inputSize; ++j)
            {
                auto bitOne = ((i >> j) & 0x1) != 0;
                auto xOff = bitOne ? this->_andInputPinXOff.at(j) : this->_andInputBarPinXOff.at(j);
    
                auto pin = inst->getPin(this->ADDRESS_NAME(j));
                auto inputPinCoord = Coord {xOff, pin.minY()};

                this->addRectangle(
                    this->_horLayer, 
                    inputPinCoord,
                    pin.maxX() - inputPinCoord.x,
                    pin.height()
                );

                this->addViaCenter(this->_viaLayer, pin.center());
                this->addViaCenter(this->_viaLayer, Coord{ xOff + this->_busWidth / 2., pin.cenY() });
            }
        }

        // Route inv bus & and bus
        // For origin input
        for (int i = 0; i < this->_inputSize; ++i)
        {
            auto invXOff = this->_invInputPinXOff.at(i);
            auto andXOff = this->_andInputPinXOff.at(i);
            
            auto pin = this->_andInsts.back()->getPin(this->ADDRESS_NAME(i));
            
            auto inputPinCoord = Coord { invXOff, pin.minY() };
            this->addRectangle(
                this->_horLayer, 
                inputPinCoord,
                andXOff - invXOff,
                pin.height()
            );
            
            this->addViaCenter(this->_viaLayer, Coord{ invXOff + this->_busWidth / 2., pin.cenY() });
        }

        // For bar input
        auto invInstMaxX = this->_invXOff + this->_inv->width();
        for (int i = 0; i < this->_inputSize; ++i) 
        {
            auto invInstMaxY = (i + 1) * this->_inv->height();
            auto andXOff = this->_andInputBarPinXOff.at(i); 
            auto invInst = this->_invInsts.at(i);

            auto zPin = invInst->getPin(INV::OUTPUT_NAME);
            auto andYOff = (i+1) * this->_and->height() - 3 * this->_horPitch;
            
            auto viaCenter = Coord{andXOff + this->_horWidth / 2., invInstMaxY - this->_horPitch - this->_horWidth / 2.0};

            // MARK: this->_horWidth
            this->routePathCenter(
                zPin.layer(), 
                std::array{
                    zPin.center(), 
                    Coord{invInstMaxX - this->_horWidth, zPin.cenY()},
                    Coord{ invInstMaxX - this->_horWidth, viaCenter.y},
                    viaCenter
                },
                this->_horWidth
            );

            this->addViaCenter(this->_viaLayer, viaCenter);
        }
    }

    void Decoder::routeComponetInstances()
    {
        this->routeColBus(this->_busLayer, this->_busWidth, this->_subDecoderHeight, 0._u, this->_invInputPinXOff);
        this->routeColBus(this->_busLayer, this->_busWidth, this->height(), 0._u, this->_andInputPinXOff);

        // Route input bus and sub decoder
        auto inputIdx = 0;
        for (auto decoderIdx = 0; decoderIdx < this->_subDecoderInsts.size(); ++decoderIdx)
        {
            auto inst = this->_subDecoderInsts.at(decoderIdx);
            auto decoder = this->_subDecoders.at(decoderIdx);
            for (auto i = 0; i < decoder->inputSize(); ++i) 
            {
                auto pin = inst->getPin(this->ADDRESS_NAME(i));
                auto inputBusXOff = this->_invInputPinXOff.at(inputIdx);

                this->addRectangle(
                    this->_horLayer, 
                    Coord{inputBusXOff, pin.minY()},
                    pin.maxX() - inputBusXOff,
                    this->_horWidth
                );

                auto centerCoord = Coord{ inputBusXOff + this->_busWidth / 2., pin.cenY() };
                this->addViaCenter(this->_viaLayer, centerCoord);
                this->addPinCenter(
                    this->_horLayer, 
                    this->ADDRESS_NAME(inputIdx), 
                    centerCoord);

                inputIdx += 1;
            }
        }

        // Route sub decoder output & and bus
        auto outputIdx = 0;
        for (auto decoderIdx = 0; decoderIdx < this->_subDecoderInsts.size(); ++decoderIdx)
        {
            auto inst = this->_subDecoderInsts.at(decoderIdx);
            auto decoder = this->_subDecoders.at(decoderIdx);
            debug::check(inst->circuit() == decoder, "No math subdecoder with inst");

            auto decoderMaxX = this->_subDecoderXOff + decoder->width();

            for (auto i = 0; i < decoder->outputSize(); ++i)
            {
                auto pin = inst->getPin(this->OUTPUT_NAME(i));
                auto inputBusXOff = this->_andInputPinXOff.at(outputIdx);

                // All sub deocder are simple type!
                auto andInstMaxY = inst->offset().y + (i + 1) * decoder->_and->height();
        
                auto viaCenter = Coord{ inputBusXOff + this->_busWidth / 2., andInstMaxY - this->_horPitch };
                this->routePathCenter(
                    pin.layer(), 
                    std::array{
                        pin.center(),
                        Coord {decoderMaxX, pin.cenY()},
                        Coord{decoderMaxX, viaCenter.y},
                        viaCenter
                    },
                    this->_horWidth
                );

                debug::debugf("viaCenter: {}", viaCenter);
                this->addViaCenter(this->_viaLayer, viaCenter);

                outputIdx += 1;
            }
        }

        // Route and bus & and insts
        const auto& subDecodersInputSize = this->subDecodersInputSize();
        // For each and inst
        for (auto andGateIdx = 0; andGateIdx < this->_outputSize; ++andGateIdx)
        {   
            auto inst = this->_andInsts[andGateIdx];

            // For each deocder(one and will connect one output of each and insts)
            for (auto decoderIdx = 0; decoderIdx < subDecodersInputSize.size(); ++decoderIdx)
            {
                int prefixSum = 0;
                int outputIndexBase = 0;
                for (auto idx = 0; idx < decoderIdx; ++idx)
                {
                    prefixSum += subDecodersInputSize[idx];
                    outputIndexBase = util::power(2, subDecodersInputSize[idx]);
                }

                int mask{1};
                for (int i = 1; i < subDecodersInputSize[decoderIdx]; ++i)
                    mask = (mask << 1) + 1;

                int decoderOutputIdx = (andGateIdx >> prefixSum) & mask ;
                int outputIndex = decoderOutputIdx + outputIndexBase;

                auto pin = inst->getPin(this->ADDRESS_NAME(decoderIdx));
                auto bufXOff = this->_andInputPinXOff.at(outputIndex);

                this->addRectangle(
                    this->_horLayer, 
                    Coord{bufXOff, pin.minY()},
                    pin.maxX() - bufXOff,
                    this->_horWidth
                );

                this->addViaCenter(this->_viaLayer, Coord{ bufXOff + this->_busWidth / 2., pin.cenY() });
                this->addViaCenter(this->_viaLayer, pin.center());
            }
        }
    }

    void Decoder::createLayoutPins() 
    {
        switch (this->_decoderType)
        {
            case DecoderType::OneAddr:  this->createOneAddrLayoutPins(); break;
            case DecoderType::Simple:   this->createSimpleLayoutPins(); break;
            case DecoderType::Componet: this->createComponetLayoutPins(); break;
        }
    }

    void Decoder::createOneAddrLayoutPins()
    {

    }

    void Decoder::createSimpleLayoutPins()
    {
        for (auto i = 0; i < this->_outputSize; ++i)
        {
            auto inst = this->_andInsts.at(i);
            this->copyInstancePin(inst, AND::OUTPUT_NAME, this->OUTPUT_NAME(i));
        }
    }
    
    void Decoder::createComponetLayoutPins()
    {
        for (auto i = 0; i < this->_outputSize; ++i)
        {
            auto inst = this->_andInsts.at(i);
            this->copyInstancePin(inst, AND::OUTPUT_NAME, this->OUTPUT_NAME(i));
        }
    }

    void Decoder::layoutMiscellaneous()
    {
        this->addRectangle(Layer::PRBoundary, Coord{0._n, 0._n}, this->width(), this->height());
    }

    void Decoder::calculateLayoutConstants() 
    {
        // What a mass...

        this->_busLayer = Layer::Metal2;
        this->_horLayer = Layer::Metal1;
        this->_viaLayer = Layer::Via1;
        
        this->_busWidth = tech->layerWidth(this->_busLayer);
        this->_busPitch = tech->layerPitch(this->_busLayer);

        this->_horWidth = tech->layerWidth(this->_horLayer);
        this->_horPitch = tech->layerPitch(this->_horLayer);

        switch (this->_decoderType)
        {
            case DecoderType::OneAddr: {
                
                break;
            }
            case DecoderType::Simple: {
                this->_invXOff = (2 + this->_inputSize) * this->_busPitch;
                this->_andXOff = this->_invXOff + this->_inv->width() + (2 + this->_inputSize * 2) * this->_busPitch;

                auto offset = this->_busPitch;
                for (int i = 0; i < this->_inputSize; ++i)
                {
                    this->_invInputPinXOff.emplace_back(offset);
                    offset += this->_busPitch;
                }

                offset = this->_invXOff + this->_inv->width() + this->_busPitch;
                for (int i = 0; i < this->_inputSize; ++i) {
                    this->_andInputBarPinXOff.emplace_back(offset);
                    offset += this->_busPitch;
                }
                for (int i = 0; i < this->_inputSize; ++i) {
                    this->_andInputPinXOff.emplace_back(offset);
                    offset += this->_busPitch;
                }

                this->_width = this->_andXOff + this->_and->width();
                this->_height = this->_andInsts.size() * this->_and->height();
                break;
            }
            case DecoderType::Componet: {
                this->_subDecoderSpacing = 2. * this->_and->height();
                this->_subDecoderXOff = (2 + this->_inputSize) * this->_busPitch;
                
                this->_subDecoderHeight = (this->_subDecoders.size() - 1) * this->_subDecoderSpacing;
                this->_andBusHeight = this->_andInsts.size() * this->_and->height();

                auto maxSubDecoderWidth = Number{};
                auto outpuZSize = 0;
                for (auto decoder : this->_subDecoders) {
                    outpuZSize += decoder->outputSize();
                    this->_subDecoderHeight += decoder->height();
                    if (maxSubDecoderWidth < decoder->width())
                        maxSubDecoderWidth = decoder->width();
                }

                this->_andBusXOff = this->_subDecoderXOff + maxSubDecoderWidth + this->_busPitch;
                this->_andXOff = this->_andBusXOff + (2 + outpuZSize) * this->_busPitch;

                auto offset = this->_busPitch;
                for (int i = 0; i < this->_inputSize; ++i)
                {
                    this->_invInputPinXOff.emplace_back(offset);
                    offset += this->_busPitch;
                }

                offset = this->_busPitch + this->_andBusXOff;
                for (int i = 0; i < outpuZSize; ++i) {
                    this->_andInputPinXOff.emplace_back(offset);
                    offset += this->_busPitch;
                }

                this->_width = this->_andXOff + this->_and->width();
                this->_height = this->_andInsts.size() * this->_and->height();
                break;
            }
        }
    }

    auto Decoder::getDecoderType(int inputSize) -> DecoderType
    {
        if (inputSize == 1) 
            return DecoderType::OneAddr;
        else if (inputSize <= Decoder::MAX_SIMPLE_INPUT_SIZE)
            return DecoderType::Simple;
        else
            return DecoderType::Componet;
    }
    
    auto Decoder::ADDRESS_NAME(int index) -> std::string_view
    {
        return stringf("A{}", index);
    }

    auto Decoder::OUTPUT_NAME(int index) -> std::string_view
    {
        return stringf("Y{}", index);
    }

}