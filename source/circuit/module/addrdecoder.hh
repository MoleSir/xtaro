#pragma once

#include "../base/module.hh"

namespace xtaro::circuit
{

    /*
        - Ports:
            -- raddr0 raddr1 ...
            -- rsel0 rsel1 ...

            -- caddr0 caddr1 ... 
            -- csel0 csel1 ...
            -- vdd gnd
    */
    class AddressDecoder : public Module
    {
    public:
        static constexpr std::string_view TYPE {"address_decoder"};

        /*
            - Construction: (int rowAddressSize, int colAddressSize);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(int raw, int caw) :
                Circuit::Arguments {AddressDecoder::TYPE},
                rowAddrWidth{raw},
                colAddrWidth{caw}
            {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            int rowAddrWidth;
            int colAddrWidth;
        };

    public:
        AddressDecoder(std::string_view name, const AddressDecoder::Arguments& arguments);
        virtual ~AddressDecoder() noexcept override = default;

    public:
        auto rowSelSize() -> int { return this->_rowSelSize; }
        auto colSelSize() -> int { return this->_colSelSize > 1 ? this->_colSelSize : 0; }

    private:
        virtual void createPorts() override;

        virtual void createCircuits() override;
        virtual void createInstances() override;

        virtual void calculateLayoutConstants() override;
        virtual void placeInstances() override;
        virtual void routeInstances() override;
        virtual void createLayoutPins() override;

    private:
        int _rowAddrWidth;
        int _colAddrWidth;

        int _rowSelSize;
        int _colSelSize;

        Circuit* _rowAddrDecoder;
        Circuit* _colAddrDecoder;
    };

}