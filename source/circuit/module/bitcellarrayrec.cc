#include "bitcellarrayrec.hh"
#include "../base/module.hh"
#include "circuit/base/instance.hh"
#include "circuit/geometry/coord.hh"

#include <circuit/libmodule/bitcell.hh>

#include <circuit/mod.hh>

#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <utility/misc/string.hh>
#include <utility/debug/debug.hh>

#include <cmath>
#include <memory>
#include <cstdint>
#include <vector>

namespace xtaro::circuit
{
    /*

        Note:

        Divide bitcellarray to subarray and some bitcell. For 
        - row size => rowSize = rowSubSize * rowSubCount + rowRemainder
        - col size => colSize = colSubSize * colSubCount + colRemainder

        So, there are four kinds of subarray:
        - 1. rowSubSize * colSubSize (rowSubCount * colSubCount)
        - 2. rowRemainder * colSubSize (colSubSize) 
        - 3. rowSubSize * colRemainder (rowSubSize)
        - 4. rowRemainder * colRemainder (1)

        +-----------+-----------+-----------+---------------+-----+
        |           |           |           |               |     |
        |     1     |     1     |     1     |    .......    |  3  |
        |           |           |           |               |     |
        +-----------+-----------+-----------+---------------+-----+
        |           |           |           |               |     |
        |     1     |     1     |     1     |    .......    |  3  |
        |           |           |           |               |     |
        +-----------+-----------+-----------+---------------+-----+
        |           |           |           |               |     |
        |     1     |     1     |     1     |    .......    |  3  |
        |           |           |           |               |     |
        +-----------+-----------+-----------+---------------+-----+
        |           |           |           |   .           |     |
        |     .     |     .     |     .     |     .         |  .  |
        |     .     |     .     |     .     |       .       |  .  |
        |     .     |     .     |     .     |         .     |  .  |
        |           |           |           |           .   |     |
        +-----------+-----------+-----------+---------------+-----+
        |     2     |     2     |     2     |    .......    |  4  |
        +-----------+-----------+-----------+---------------+-----+

    */
    
    auto BitcellArrayRecursive::Arguments::toString() const -> std::string
    {
        return std::format("r{}_c{}", this->rowSize, this->columnSize);
    }

    auto BitcellArrayRecursive::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<BitcellArrayRecursive>(circuitName, *this);
    }

    BitcellArrayRecursive::BitcellArrayRecursive(std::string_view name, const BitcellArrayRecursive::Arguments& arguments) :
        Module{name},
        _rowSize{arguments.rowSize},
        _columnSize{arguments.columnSize},

        _useSubArray {false},
        _rowSubArrInfo { -1, -1, -1 },
        _colSubArrInfo { -1, -1, -1 },

        _rowcolSubArray {nullptr},
        _rowSubArray {nullptr},
        _colSubArray {nullptr},
        _subArray {nullptr},

        _bitcell{nullptr}
    {
        debug::checkf(this->_rowSize >= 1, "Row size '{}' < 1", this->_rowSize);
        debug::checkf(this->_columnSize >= 1, "Column size '{}' < 1", this->_columnSize);

        if (this->_rowSize >= 4 || this->_columnSize >= 4)
        {
            this->_useSubArray = true;
            this->_rowSubArrInfo = BitcellArrayRecursive::subArrayInfo(this->_rowSize);
            this->_colSubArrInfo = BitcellArrayRecursive::subArrayInfo(this->_columnSize);
        }

        this->createSelf();
    }

    void BitcellArrayRecursive::createPorts()
    {
        // 'bl' and 'br'
        std::vector<std::string_view> blbrPortsName;
        for (std::int32_t i = 0; i < this->_columnSize; ++i)
            blbrPortsName.emplace_back( BitcellArrayRecursive::BITLINE_NAME(i) );
        for (std::int32_t i = 0; i < this->_columnSize; ++i)
            blbrPortsName.emplace_back( BitcellArrayRecursive::BITLINE_BAR_NAME(i) );

        this->addPortNets(blbrPortsName, PortType::Output);

        // 'WL' ports
        std::vector<std::string_view> wlPortsName;
        for (std::int32_t i = 0; i < this->_rowSize; ++i)
            wlPortsName.emplace_back( BitcellArrayRecursive::WORDLINE_NAME(i) );
        
        this->addPortNets(wlPortsName, PortType::Input);

        // 'vdd' and 'gnd'
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void BitcellArrayRecursive::createCircuits()
    {
        if (this->_useSubArray)
            this->createSubArrayCircuits();
        else
            this->_bitcell = this->addCircuit(Bitcell::Arguments {});
    }

    void BitcellArrayRecursive::createInstances()
    {
        if (this->_useSubArray)
            this->createSubArrayInstances();
        else
            this->createBitcellInstances();
    }

    void BitcellArrayRecursive::calculateLayoutConstants()
    {
    }

    void BitcellArrayRecursive::placeInstances() 
    {
        if (this->_useSubArray) 
            this->placeSubArrayInstances();
        else 
            this->placeBitcellInstances();
    }

    void BitcellArrayRecursive::routeInstances()
    {
        if (this->_useSubArray)
        {
            auto blbrIndex = 0;
            auto wlIndex = 0;

            for (auto c = 0; c < this->_colSubArrInfo.count; ++c)
            {               
                auto inst = this->_rowcolSubArrayInsts[0][c]; 
                for (auto i = 0; i < this->_colSubArrInfo.size; ++i) 
                {
                    auto blPin = inst->getPin(BitcellArrayRecursive::BITLINE_NAME(i));
                    this->addLabelPin(
                        blPin.layer(), 
                        BitcellArrayRecursive::BITLINE_NAME(blbrIndex),
                        blPin.leftdown().scale(1., 0.),
                        blPin.width(),
                        this->_height);

                    auto brPin = inst->getPin(BitcellArrayRecursive::BITLINE_BAR_NAME(i));
                    this->addLabelPin(
                        brPin.layer(), 
                        BitcellArrayRecursive::BITLINE_BAR_NAME(blbrIndex), 
                        brPin.leftdown().scale(1., 0.), 
                        brPin.width(), 
                        this->_height); 

                    blbrIndex += 1;
                }
            }

            if (this->_colSubArray != nullptr) 
            {
                auto inst = this->_colSubArrayInsts[0]; 
                for (auto i = 0; i < this->_colSubArrInfo.remainder; ++i) 
                {
                    auto blPin = inst->getPin(BitcellArrayRecursive::BITLINE_NAME(i));
                    this->addLabelPin(
                        blPin.layer(), 
                        BitcellArrayRecursive::BITLINE_NAME(blbrIndex),
                        blPin.leftdown().scale(1., 0.),
                        blPin.width(),
                        this->_height);

                    auto brPin = inst->getPin(BitcellArrayRecursive::BITLINE_BAR_NAME(i));
                    this->addLabelPin(
                        brPin.layer(), 
                        BitcellArrayRecursive::BITLINE_BAR_NAME(blbrIndex), 
                        brPin.leftdown().scale(1., 0.), 
                        brPin.width(), 
                        this->_height); 

                    blbrIndex += 1;
                }
            }

            for (auto r = 0; r < this->_rowSubArrInfo.count; ++r)
            {               
                auto inst = this->_rowcolSubArrayInsts[r][0]; 
                for (auto i = 0; i < this->_rowSubArrInfo.size; ++i) 
                {
                    auto wlPin = inst->getPin(BitcellArrayRecursive::WORDLINE_NAME(i));
                    this->addLabelPin(
                        wlPin.layer(), 
                        BitcellArrayRecursive::WORDLINE_NAME(wlIndex), 
                        wlPin.leftdown().scale(0., 1.), 
                        this->_width, 
                        wlPin.height()); 
                    wlIndex += 1;
                }
            }

            if (this->_rowSubArray != nullptr)
            {
                auto inst = this->_rowSubArrayInsts[0];
                for (auto i = 0; i < this->_rowSubArrInfo.remainder; ++i) 
                {
                    auto wlPin = inst->getPin(BitcellArrayRecursive::WORDLINE_NAME(i));
                    this->addLabelPin(
                        wlPin.layer(), 
                        BitcellArrayRecursive::WORDLINE_NAME(wlIndex), 
                        wlPin.leftdown().scale(0., 1.), 
                        this->_width, 
                        wlPin.height()); 
                    wlIndex += 1;
                }
            }
        }
        else 
        {
            for (int col = 0; col < this->_columnSize; ++col) 
            {
                auto inst = this->_bitcellInsts[0][col];
                
                auto blPin = inst->getPin("bl");
                this->addLabelPin(
                    blPin.layer(), 
                    BitcellArrayRecursive::BITLINE_NAME(col), 
                    blPin.leftdown().scale(1., 0.), 
                    blPin.width(), 
                    this->_height);

                auto brPin = inst->getPin("br");
                this->addLabelPin(
                    brPin.layer(), 
                    BitcellArrayRecursive::BITLINE_BAR_NAME(col), 
                    brPin.leftdown().scale(1., 0.), 
                    brPin.width(), 
                    this->_height); 
            }

            for (int row = 0; row < this->_rowSize; ++row) 
            {
                auto inst = this->_bitcellInsts[row][0];
                auto wlPin = inst->getPin("wl");
                this->addLabelPin(
                    wlPin.layer(), 
                    BitcellArrayRecursive::WORDLINE_NAME(row), 
                    wlPin.leftdown().scale(0., 1.), 
                    this->_width, 
                    wlPin.height()); 
            } 
        }
    }

    void BitcellArrayRecursive::createBitcellInstances()
    {
        for (std::int32_t r = 0; r < this->_rowSize; ++r)
        {
            auto insts = std::vector<Instance*>{};
            for (std::int32_t c = 0; c < this->_columnSize; ++c)
            {                
                auto inst = this->addInstance(stringf("bitcell_{}_{}", r, c), this->_bitcell, {
                    BitcellArrayRecursive::BITLINE_NAME(c),
                    BitcellArrayRecursive::BITLINE_BAR_NAME(c),
                    BitcellArrayRecursive::WORDLINE_NAME(r),
                    tech->vdd(),
                    tech->gnd()
                });
                insts.emplace_back(inst);
            }
            this->_bitcellInsts.emplace_back(std::move(insts));
        }
    }

    void BitcellArrayRecursive::createSubArrayCircuits()
    {
        // Row & Col sub array
        this->_rowcolSubArray = this->addCircuit(BitcellArrayRecursive::Arguments {
            this->_rowSubArrInfo.size, this->_colSubArrInfo.size
        });

        // Row sub array
        if (this->_rowSubArrInfo.remainder >= 1)
            this->_rowSubArray = this->addCircuit(BitcellArrayRecursive::Arguments {
                this->_rowSubArrInfo.remainder, this->_colSubArrInfo.size
            });

        // Col sub array
        if (this->_colSubArrInfo.remainder >= 1)
            this->_colSubArray = this->addCircuit(BitcellArrayRecursive::Arguments {
                this->_rowSubArrInfo.size, this->_colSubArrInfo.remainder
            });

        // Sub array
        if (this->_colSubArrInfo.remainder >= 1 && this->_rowSubArrInfo.remainder >= 1)
            this->_subArray = this->addCircuit(BitcellArrayRecursive::Arguments {
                this->_rowSubArrInfo.remainder, this->_colSubArrInfo.remainder
            });
    }

    void BitcellArrayRecursive::createSubArrayInstances()
    {
        this->createRowColSubArrayInstances();
        this->createRowSubArrayInstances();
        this->createColSubArrayInstances();
        this->createSingleSubArrayInstance();
    }

    void BitcellArrayRecursive::createRowColSubArrayInstances()
    {
        for (int row = 0; row < this->_rowSubArrInfo.count; ++row)
        {
            auto insts = std::vector<Instance*>{};
            for (int col = 0; col < this->_colSubArrInfo.count; ++col)
            {
                int baseRowIdx {row * this->_rowSubArrInfo.size};
                int baseColIdx {col * this->_colSubArrInfo.size};
                std::vector<std::string_view> nets {};

                for (int idx = 0; idx < this->_colSubArrInfo.size; ++idx)
                    nets.emplace_back(BitcellArrayRecursive::BITLINE_NAME(baseColIdx + idx));

                for (int idx = 0; idx < this->_colSubArrInfo.size; ++idx)
                    nets.emplace_back(BitcellArrayRecursive::BITLINE_BAR_NAME(baseColIdx + idx));

                for (int idx = 0; idx < this->_rowSubArrInfo.size; ++idx)
                    nets.emplace_back(BitcellArrayRecursive::WORDLINE_NAME(baseRowIdx + idx));

                nets.emplace_back(tech->vdd());
                nets.emplace_back(tech->gnd());
                
                std::string_view insName {stringf("rowcol_subarray_{}_{}", row, col)};
                insts.emplace_back(this->addInstance(insName, this->_rowcolSubArray, nets));
            }
            this->_rowcolSubArrayInsts.emplace_back(std::move(insts));
        }
    }

    void BitcellArrayRecursive::createRowSubArrayInstances()
    {
        // No row array left
        if (this->_rowSubArray == nullptr) return;

        int baseRowIdx {this->_rowSubArrInfo.count * this->_rowSubArrInfo.size};
        for (int col = 0; col < this->_colSubArrInfo.count; ++col)
        {
            int baseColIdx {col * this->_colSubArrInfo.size};
            std::vector<std::string_view> nets {};
            
            for (int idx = 0; idx < this->_colSubArrInfo.size; ++idx)
                nets.emplace_back(BitcellArrayRecursive::BITLINE_NAME(baseColIdx + idx));

            for (int idx = 0; idx < this->_colSubArrInfo.size; ++idx)
                nets.emplace_back(BitcellArrayRecursive::BITLINE_BAR_NAME(baseColIdx + idx));

            for (int idx = 0; idx < this->_rowSubArrInfo.remainder; ++idx)
                nets.emplace_back(BitcellArrayRecursive::WORDLINE_NAME(baseRowIdx + idx));

            nets.emplace_back(tech->vdd());
            nets.emplace_back(tech->gnd());
        
            std::string_view insName {stringf("row_subarray_{}", col)};
            auto inst = this->addInstance(insName, this->_rowSubArray, nets);
            this->_rowSubArrayInsts.emplace_back(inst);
        }
    }   
 
    void BitcellArrayRecursive::createColSubArrayInstances()
    {
        // No col array left
        if (this->_colSubArray == nullptr) return;

        int baseColIdx {this->_colSubArrInfo.count * this->_colSubArrInfo.size};
        for (int row = 0; row < this->_rowSubArrInfo.count; ++row)
        {
            int baseRowIdx {row * this->_rowSubArrInfo.size};
            std::vector<std::string_view> nets {};

            for (int idx = 0; idx < this->_colSubArrInfo.remainder; ++idx)
                nets.emplace_back(BitcellArrayRecursive::BITLINE_NAME(baseColIdx + idx));

            for (int idx = 0; idx < this->_colSubArrInfo.remainder; ++idx)
                nets.emplace_back(BitcellArrayRecursive::BITLINE_BAR_NAME(baseColIdx + idx));

            for (int idx = 0; idx < this->_rowSubArrInfo.size; ++idx)
                nets.emplace_back(BitcellArrayRecursive::WORDLINE_NAME(baseRowIdx + idx));

            nets.emplace_back(tech->vdd());
            nets.emplace_back(tech->gnd());

            std::string_view insName {stringf("col_subarray_{}", row)};
            auto inst = this->addInstance(insName, this->_colSubArray, nets);
            this->_colSubArrayInsts.emplace_back(inst);
        }
    }
    
    void BitcellArrayRecursive::createSingleSubArrayInstance()
    {
        // No row & col left
        if (this->_subArray == nullptr) return;

        int baseRowIdx {this->_rowSubArrInfo.count * this->_rowSubArrInfo.size};
        int baseColIdx {this->_colSubArrInfo.count * this->_colSubArrInfo.size};

        std::vector<std::string_view> nets {};
        for (int idx = 0; idx < this->_colSubArrInfo.remainder; ++idx)
            nets.emplace_back(BitcellArrayRecursive::BITLINE_NAME(baseColIdx + idx));

        for (int idx = 0; idx < this->_colSubArrInfo.remainder; ++idx)
            nets.emplace_back(BitcellArrayRecursive::BITLINE_BAR_NAME(baseColIdx + idx));

        for (int idx = 0; idx < this->_rowSubArrInfo.remainder; ++idx)
            nets.emplace_back(BitcellArrayRecursive::WORDLINE_NAME(baseRowIdx + idx));

        nets.emplace_back(tech->vdd());
        nets.emplace_back(tech->gnd());

        std::string_view insName {stringf("subarray")};
        this->_subArrayInst = this->addInstance(insName, this->_subArray, nets);
    }

    void BitcellArrayRecursive::placeSubArrayInstances() 
    {
        this->_width.emplace(
            this->_colSubArrInfo.count * this->_rowcolSubArray->width() + 
            (this->_colSubArray == nullptr ? 0._n : this->_colSubArray->width())
        );
        this->_height.emplace(
            this->_rowSubArrInfo.count * this->_rowcolSubArray->height() + 
            (this->_rowSubArray == nullptr ? 0._n : this->_rowSubArray->height())
        );

        this->placeRowColSubArrayInstances();
        this->placeRowSubArrayInstances();
        this->placeColSubArrayInstances();
        this->placeSingleSubArrayInstance();
    }

    void BitcellArrayRecursive::placeBitcellInstances()
    {
        this->_width.emplace(this->_columnSize * this->_bitcell->width());
        this->_height.emplace(this->_rowSize * this->_bitcell->height());

        for (std::int32_t r = 0; r < this->_rowSize; ++r)
        {
            for (std::int32_t c = 0; c < this->_columnSize; ++c)
            {
                auto inst = this->_bitcellInsts[r][c];
                auto coord = Coord{c * this->_bitcell->width(), r * this->_bitcell->height()};
                inst->place(coord);
            }
        }
    }

    void BitcellArrayRecursive::placeRowColSubArrayInstances()
    {
        for (int row = 0; row < this->_rowSubArrInfo.count; ++row)
        {
            for (int col = 0; col < this->_colSubArrInfo.count; ++col)
            {
                auto inst = this->_rowcolSubArrayInsts[row][col];
                auto coord = Coord{col * this->_rowcolSubArray->width(), row * this->_rowcolSubArray->height()};
                inst->place(coord);
            }
        }
    }

    void BitcellArrayRecursive::placeRowSubArrayInstances()
    {
        // No row array left
        if (this->_rowSubArray == nullptr) return;

        auto baseRowPos = this->_rowcolSubArray->height() * this->_rowSubArrInfo.count;
        for (int col = 0; col < this->_colSubArrInfo.count; ++col)
        {
            auto inst = this->_rowSubArrayInsts[col];
            auto coord = Coord{col * this->_rowSubArray->width(), baseRowPos};
            inst->place(coord);
        }
    }

    void BitcellArrayRecursive::placeColSubArrayInstances()
    {
        // No row array left
        if (this->_colSubArray == nullptr) return;

        auto baseColPos = this->_rowcolSubArray->width() * this->_colSubArrInfo.count;
        for (int row = 0; row < this->_rowSubArrInfo.count; ++row)
        {
            auto inst = this->_colSubArrayInsts[row];
            auto coord = Coord{baseColPos, row * this->_colSubArray->height()};
            inst->place(coord);
        }
    }

    void BitcellArrayRecursive::placeSingleSubArrayInstance()
    {
        // No row & col left
        if (this->_subArray == nullptr) return;

        auto baseRowPos = this->_rowSubArrInfo.count * this->_rowcolSubArray->height();
        auto baseColPos = this->_colSubArrInfo.count * this->_rowcolSubArray->width();

        this->_subArrayInst->place(Coord{baseColPos, baseRowPos});
    }

    auto BitcellArrayRecursive::subArrayInfo(int size) -> SubArrayInfo
    {
        // size = subsize * subcount + remainder
        int multiple = 1;
        int left = size - multiple * multiple;
        while (true) 
        {
            int nextMultiple = multiple + 1;
            int powMul2 = nextMultiple * nextMultiple;
            if (powMul2 > size) break;
            // Update
            multiple = nextMultiple;
            left = size - powMul2;
        }

        // Now size = multiple * multiple + remainder
        // But, left may bigger than multiple
        if (multiple > left)
            return SubArrayInfo { multiple, multiple, left };
        
        int remainder = left % multiple;
        int subsize = multiple + (left / multiple);
        int subcount = multiple;

        return SubArrayInfo { subsize, subcount, remainder };
    }

    void BitcellArrayRecursive::createLayoutPins()
    {
    }

    auto BitcellArrayRecursive::BITLINE_NAME(int index) -> std::string_view
    {
        return stringf("bl{}", index);
    }
    
    auto BitcellArrayRecursive::BITLINE_BAR_NAME(int index) -> std::string_view
    {
        return stringf("br{}", index);
    }
    
    auto BitcellArrayRecursive::WORDLINE_NAME(int index) -> std::string_view
    {
        return stringf("wl{}", index);
    }

}