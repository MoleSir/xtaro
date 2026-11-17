#pragma once

#include "../base/module.hh"
#include "circuit/base/instance.hh"
#include <string_view>
#include <vector>

namespace xtaro::circuit
{

    /*
        - Ports: bl0 ... bln br0 ... brn ... wl0 ... wlm vdd gnd;
    */
    class BitcellArrayRecursive : public Module
    {
    public:
        static constexpr std::string_view TYPE = "bitcell_array";
        static auto BITLINE_NAME(int index) -> std::string_view;
        static auto BITLINE_BAR_NAME(int index) -> std::string_view;
        static auto WORDLINE_NAME(int index) -> std::string_view;

        /*
            - Construction: (int rowSize, int columnSize);
        */
        struct Arguments : public Circuit::Arguments 
        {
            Arguments(int rs, int cs) noexcept : 
                Circuit::Arguments {BitcellArrayRecursive::TYPE},
                rowSize{rs}, columnSize{cs} {}

            virtual ~Arguments() noexcept = default;

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            int rowSize;
            int columnSize;
        };

    public:
        BitcellArrayRecursive(std::string_view name, const BitcellArrayRecursive::Arguments& arguments);
        virtual ~BitcellArrayRecursive() noexcept override = default;

    private:
        struct SubArrayInfo 
        {   
            int size;
            int count;
            int remainder;
        };
        static auto subArrayInfo(int size) -> SubArrayInfo;

    private:
        virtual void createPorts() override;
        
        virtual void createCircuits() override;
        virtual void createInstances() override;

        virtual void calculateLayoutConstants() override;
        virtual void placeInstances() override;
        virtual void routeInstances() override;
        virtual void createLayoutPins() override;

    private:
        void createSubArrayCircuits();

        void createSubArrayInstances();
        void createBitcellInstances();
        void createRowColSubArrayInstances();
        void createRowSubArrayInstances();
        void createColSubArrayInstances();
        void createSingleSubArrayInstance();

    private:
        void placeSubArrayInstances();

        void placeBitcellInstances();
        void placeRowColSubArrayInstances();
        void placeRowSubArrayInstances();
        void placeColSubArrayInstances();
        void placeSingleSubArrayInstance();

    private:
        int _rowSize;
        int _columnSize;

        bool _useSubArray;
        SubArrayInfo _rowSubArrInfo;
        SubArrayInfo _colSubArrInfo;

        Circuit* _rowcolSubArray;
        Circuit* _rowSubArray;
        Circuit* _colSubArray;
        Circuit* _subArray;

        Circuit* _bitcell;
    
        std::vector<std::vector<Instance*>> _rowcolSubArrayInsts {};
        std::vector<Instance*> _rowSubArrayInsts {};
        std::vector<Instance*> _colSubArrayInsts {};
        Instance* _subArrayInst;

        std::vector<std::vector<Instance*>> _bitcellInsts {}; 
    };

}