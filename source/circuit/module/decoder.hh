#pragma once

#include "../base/module.hh"
#include "circuit/base/instance.hh"
#include "global/number/number.hh"

#include <string_view>
#include <vector>
#include <array>

namespace xtaro::circuit
{

    /*
        - Ports: A0 A1 ... An-1 Y0 Y1 ... Y2^n-1 vdd gnd
    */
    class Decoder : public Module
    {
    public:
        static constexpr std::string_view TYPE = "decoder";
        static auto ADDRESS_NAME(int index) -> std::string_view; 
        static auto OUTPUT_NAME(int index) -> std::string_view;

        /*
            - Construction: (int inputSize);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(int is) : 
                Circuit::Arguments {Decoder::TYPE},
                inputSize{is} {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            int inputSize;
        };

    public:
        enum 
        {
            MAX_SIMPLE_INPUT_SIZE = 4,
            MIN_INPUT_SIZE = 1,
            MAX_INPUT_SIZE = 12,
            COMPONENT_SIZE = MAX_INPUT_SIZE - MAX_SIMPLE_INPUT_SIZE,
        };

        static const std::array<std::vector<int>, COMPONENT_SIZE> subDecodersInputSizes; 

    public:
        Decoder(std::string_view name, const Decoder::Arguments& arguments);
        virtual ~Decoder() noexcept override = default;

    private:
        enum class DecoderType {
            OneAddr,
            Simple,
            Componet,
        };

        static auto getDecoderType(int inputSize) -> DecoderType; 

    private:
        virtual void createPorts() override;

        virtual void createCircuits() override;
        virtual void createInstances() override;

        virtual void calculateLayoutConstants() override;
        virtual void placeInstances() override;
        virtual void routeInstances() override;
        virtual void createLayoutPins() override;
        virtual void layoutMiscellaneous() override;

    private:
        void placeOneAddrInstances();
        void placeSimpleInstances();
        void placeComponetInstances();

        void routeOneAddrInstances();
        void routeSimpleInstances();
        void routeComponetInstances();

        void createOneAddrLayoutPins(); 
        void createSimpleLayoutPins();
        void createComponetLayoutPins();

    private:
        int subDecodersIndex() const noexcept
        { return this->_inputSize - 1 - MAX_SIMPLE_INPUT_SIZE; }

        const std::vector<int>& subDecodersInputSize() const noexcept
        { return Decoder::subDecodersInputSizes[this->subDecodersIndex()]; }

    private:
        void createSubDecoders();

        void createOneAddrInstance();
        void createSimpleInstances();
        void createComponetInstances();

    public:
        auto inputSize() const -> int { return this->_inputSize; }
        auto outputSize() const -> int { return this->_outputSize; }

    private:
        int _inputSize;
        int _outputSize;

        DecoderType _decoderType;

        Circuit* _and;
        Circuit* _inv;

        std::vector<Decoder*> _subDecoders;

        Instance* _inv0Inst {nullptr};
        Instance* _inv1Inst {nullptr};

        std::vector<Instance*> _invInsts {};
        std::vector<Instance*> _andInsts {};

        std::vector<Instance*> _subDecoderInsts {};

        std::vector<Number> _invInputPinXOff {};
        std::vector<Number> _andInputPinXOff {};
        std::vector<Number> _andInputBarPinXOff {};

        //////////////////// 
        Number _invXOff {};
        Number _andXOff {};
        
        Layer  _busLayer {};
        Layer  _horLayer {};
        Layer  _viaLayer {};

        Number _busWidth {};
        Number _busPitch {};

        Number _horWidth {};
        Number _horPitch {};

        // comp
        Number _subDecoderSpacing {};        
        Number _subDecoderXOff {};
        Number _andBusXOff {};
        Number _andBusHeight {};
        Number _subDecoderHeight {};
    };

}