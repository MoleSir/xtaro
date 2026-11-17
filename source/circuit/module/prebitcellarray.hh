#pragma once

#include "../base/module.hh"

namespace xtaro::circuit
{

    /*
        - Ports: 
            -- wl_en p_en_bar we_en
            
            -- bl0 ... bln 
            -- br0 ... brn

            -- rsel0 rsel1 ... 
        
            -- rbl
            -- vdd gnd
    */
    class PerpheralBitcellArray : public Module
    {
    public:
        static constexpr std::string_view TYPE = "perpheral_bitcell_array";

        /*
            - Construction: (int addressWidth, int wordWidth);
        */
        struct Arguments : public Circuit::Arguments 
        {
            Arguments(int rs, int cs) noexcept : 
                Circuit::Arguments {PerpheralBitcellArray::TYPE},
                rowSize {rs},
                columnSize {cs}
            {
            }

            virtual ~Arguments() noexcept = default;
            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            int rowSize;
            int columnSize;
        };
        
    public:
        PerpheralBitcellArray(std::string_view name, const PerpheralBitcellArray::Arguments& arguments);
        virtual ~PerpheralBitcellArray() noexcept override = default;

    private:
        virtual void createPorts() override;

        virtual void createCircuits() override;
        virtual void createInstances() override;

        virtual void calculateLayoutConstants() override;
        virtual void placeInstances() override;
        virtual void routeInstances() override;
        virtual void createLayoutPins() override;
        
    private:        
        int _rowSize;
        int _columnSize;

        Circuit* _prechargeArray;
        Circuit* _bitcellArray;
        Circuit* _replicaBank;
        Circuit* _andArray;
    };

}