#include "./sram1bank.hh"
#include "../base/module.hh"
#include "circuit/base/port.hh"
#include "circuit/geometry/pin.hh"
#include "circuit/module/buffer.hh"
#include "circuit/module/controllogic.hh"
#include "circuit/module/datapath.hh"
#include "circuit/module/decoder.hh"
#include "circuit/module/dffcolumn.hh"
#include "circuit/module/dffrow.hh"
#include "circuit/module/prechargearr.hh"
#include "circuit/module/replicatedbitcellarray.hh"
#include "circuit/module/rowdecoder.hh"
#include <circuit/libmodule/writedriver.hh>
#include "global/number/number.hh"
#include "global/tech/driveability.hh"
#include "global/tech/layer.hh"
#include "utility/misc/math.hh"
#include <array>
#include <cassert>
#include <circuit/geometry/coord.hh>
#include <circuit/mod.hh>
#include <format>
#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <ranges>
#include <string_view>
#include <utility/debug/debug.hh>
#include <vector>

namespace xtaro::circuit
{

    auto SRAM1Bank::Arguments::toString() const -> std::string 
    {
        return std::format("aw{}_caw{}_ww{}", this->addressWidth, this->colAddressWdith, this->wordWidth);
    }
    
    auto SRAM1Bank::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<SRAM1Bank>(circuitName, *this);
    }

    SRAM1Bank::SRAM1Bank(std::string_view name, const SRAM1Bank::Arguments& arguments) :
        Module{name},
        _addressWidth{arguments.addressWidth},
        _colAddressWidth{arguments.colAddressWdith},
        _wordWidth{arguments.wordWidth},
        _rowAddressWidth{arguments.addressWidth - arguments.colAddressWdith}
    {
        debug::checkf(this->_addressWidth >= 2, "Address width '{}' < 2", this->_addressWidth);
        debug::checkf(this->_colAddressWidth > 0, "Negtive/0 column address width '{}'", this->_colAddressWidth);
        debug::checkf(this->_colAddressWidth < this->_addressWidth, "column address width '{}' >= address width '{}'", this->_colAddressWidth, this->_addressWidth);
        
        this->_bitcellRowSize = util::power(2, this->_rowAddressWidth);
        this->_columnSelSize = util::power(2, this->_colAddressWidth);
        this->_bitcellColSize = this->_columnSelSize * this->_wordWidth;

        this->createSelf();
    }

    void SRAM1Bank::createPorts()
    {
        this->addPort(CLOCK_NAME, PortType::Input);
        this->addPort(CHIP_SEL_BAR_NAME, PortType::Input);
        this->addPort(WRITE_ENABLE_NAME, PortType::Input);

        for (auto i : std::ranges::views::iota(0, this->_addressWidth)) 
            this->addPort(ADDRESS_NAME(i), PortType::Input);

        for (auto i : std::ranges::views::iota(0, this->_wordWidth))
            this->addPort(DATA_INPUT_NAME(i), PortType::Input);

        for (auto i : std::ranges::views::iota(0, this->_wordWidth))
            this->addPort(DATA_OUTPUT_NAME(i), PortType::Output);

        this->addPort(tech->vdd(), PortType::InOut);
        this->addPort(tech->gnd(), PortType::InOut);
    }

    void SRAM1Bank::createCircuits()
    {
        this->_replicatedBitcellArray = this->addCircuit(ReplicatedBitcellArray::Arguments{
            this->_bitcellRowSize, this->_bitcellColSize
        });

        this->_prechargeArray = this->addCircuit(PrechargeArray::Arguments {this->_bitcellColSize + 1});

        this->_rowDecoder = this->addCircuit(RowDecoder::Arguments{this->_rowAddressWidth});
        this->_colDecoder = this->addCircuit(Decoder::Arguments{this->_colAddressWidth});
        this->_datapath = this->addCircuit(DataPath::Arguments{
            this->_wordWidth, util::power(2, this->_colAddressWidth)
        });

        this->_controlLogic = this->addCircuit(ControlLogic::Arguments{});
        this->_writeDriver = this->addCircuit(WriteDriver::Arguments{});
        this->_buffer = this->addCircuit(Buffer::Arguments{DriveAbility::X1});
        this->_dinDffs = this->addCircuit(DFlipFlopRow::Arguments{this->_wordWidth});
        this->_addressDffs = this->addCircuit(DFlipFlopColumn::Arguments{this->_addressWidth});
        this->_controlSignalDffs = this->addCircuit(DFlipFlopColumn::Arguments{2});
    }

    void SRAM1Bank::createInstances() 
    {
        // Inner nets
        auto wlNets = std::vector<std::string_view>{};
        for (auto i : std::ranges::views::iota(0, this->_bitcellRowSize))
            wlNets.emplace_back(ReplicatedBitcellArray::WORDLINE_NAME(i));

        auto blNets = std::vector<std::string_view>{};
        auto brNets = std::vector<std::string_view>{};
        for (auto i : std::ranges::views::iota(0, this->_bitcellColSize))
        {
            blNets.emplace_back(ReplicatedBitcellArray::BITLINE_NAME(i));
            brNets.emplace_back(ReplicatedBitcellArray::BITLINE_BAR_NAME(i));
        }
        
        auto poweredRBL = "prbl";
        auto rbl = ReplicatedBitcellArray::REPLICA_BITLINE_NAME;
        auto rbr = ReplicatedBitcellArray::REPLICA_BITLINE_BAR_NAME;

        auto colselNets = std::vector<std::string_view>{};
        for (auto i : std::ranges::views::iota(0, util::power(2, this->_colAddressWidth))) 
            colselNets.emplace_back(stringf("csel{}", i));

        // Bitcell Array: bl0 ... bln br0 ... brn ... wl0 ... wlm rbl rwl vdd gnd
        auto bitcellArrayNets = std::vector<std::string_view>{};
        for (auto i : std::ranges::views::iota(0, this->_bitcellColSize))
            bitcellArrayNets.emplace_back(blNets.at(i));
        for (auto i : std::ranges::views::iota(0, this->_bitcellColSize))
            bitcellArrayNets.emplace_back(brNets.at(i));
        for (auto i : std::ranges::views::iota(0, this->_bitcellRowSize))
            bitcellArrayNets.emplace_back(wlNets.at(i));
        bitcellArrayNets.emplace_back(rbl);
        bitcellArrayNets.emplace_back(rbr);
        bitcellArrayNets.emplace_back(ControlLogic::WORDLINE_ENABLE_NAME);
        bitcellArrayNets.emplace_back(tech->vdd());
        bitcellArrayNets.emplace_back(tech->gnd());

        this->_replicatedBitcellArrayInst = 
            this->addInstance(ReplicatedBitcellArray::TYPE, this->_replicatedBitcellArray, bitcellArrayNets);

        // Precharge Array: bl0 bl1 ... br0 br1 ... p_en_bar vdd gnd
        auto prechargeArrayNets = std::vector<std::string_view>{};
        for (auto i : std::ranges::views::iota(0, this->_bitcellColSize))
            prechargeArrayNets.emplace_back(blNets.at(i));
        prechargeArrayNets.emplace_back(rbl);
        for (auto i : std::ranges::views::iota(0, this->_bitcellColSize))
            prechargeArrayNets.emplace_back(brNets.at(i));
        prechargeArrayNets.emplace_back(rbr);
        prechargeArrayNets.emplace_back(ControlLogic::PRECHARGE_ENABLE_NAME);
        prechargeArrayNets.emplace_back(tech->vdd());
        prechargeArrayNets.emplace_back(tech->gnd());

        this->_prechargeArrayInst =
            this->addInstance(PrechargeArray::TYPE, this->_prechargeArray, prechargeArrayNets);

        // Row decoder: wl_en A0 A1 ... An wl0 wl1 ... wl2^n-1 vdd gnd
        auto rowDecoderNets = std::vector<std::string_view>{};
        rowDecoderNets.emplace_back(ControlLogic::WORDLINE_ENABLE_NAME);
        for (auto i : std::ranges::views::iota(this->_colAddressWidth, this->_addressWidth))
            rowDecoderNets.emplace_back(REG_ADDRESS_NAME(i));
        for (auto i : std::ranges::views::iota(0, this->_bitcellRowSize))
            rowDecoderNets.emplace_back(wlNets.at(i));
        rowDecoderNets.emplace_back(tech->vdd());
        rowDecoderNets.emplace_back(tech->gnd());

        this->_rowDecoderInst =
            this->addInstance(RowDecoder::TYPE, this->_rowDecoder, rowDecoderNets);
        
        // Decoder: A0 A1 ... An-1 Y0 Y1 ... Y2^n-1 vdd gnd
        auto decoderNets = std::vector<std::string_view>{};
        for (auto i : std::ranges::views::iota(0, this->_colAddressWidth)) 
            decoderNets.emplace_back(REG_ADDRESS_NAME(i));
        for (auto& net : colselNets)
            decoderNets.emplace_back(net);
        decoderNets.emplace_back(tech->vdd());
        decoderNets.emplace_back(tech->gnd());

        this->_colDecoderInst = 
            this->addInstance("col_decoder", this->_colDecoder, decoderNets);
        
        // Datapath
        /*
            - Ports: 
                -- sa_en we_en
                -- bl0 bl1 ...
                -- br0 br1 ...
                -- sel0 sel1 ...
                -- din0 din1 ...
                -- dout0 dout1 ...
                -- vdd gnd
        */
        auto datapathNets = std::vector<std::string_view>{};
        datapathNets.emplace_back(ControlLogic::SENSE_AMP_ENABLE_NAME);
        datapathNets.emplace_back(ControlLogic::WRITE_ENABLE_NAME);
        for (auto& net : blNets) 
            datapathNets.emplace_back(net);
        for (auto& net : brNets) 
            datapathNets.emplace_back(net);
        for (auto& net : colselNets) 
            datapathNets.emplace_back(net);
        for (auto i : std::ranges::views::iota(0, this->_wordWidth)) 
            datapathNets.emplace_back(REG_DATA_INPUT_NAME(i));
        for (auto i : std::ranges::views::iota(0, this->_wordWidth)) 
            datapathNets.emplace_back(DATA_OUTPUT_NAME(i));
        datapathNets.emplace_back(tech->vdd());
        datapathNets.emplace_back(tech->gnd());

        this->_datapathInst = 
            this->addInstance(DataPath::TYPE, this->_datapath, datapathNets);

        // Write Driver for rbl: din bl br en vdd gnd
        auto writeDriverNets = std::vector<std::string_view>{
            tech->gnd(), rbl, rbr, ControlLogic::WRITE_ENABLE_NAME, tech->vdd(), tech->gnd()
        };
        this->_writeDriverInst = this->addInstance("rbl_write_driver", this->_writeDriver, writeDriverNets);

        // Buffer: in out vdd gnd
        this->_bufferInst = this->addInstance("rbl_buffer", this->_buffer, {
            rbl, poweredRBL, tech->vdd(), tech->gnd()
        });

        // Control Logic: clk csb we rbl wl_en p_en_bar sa_en we_en vdd gnd
        auto controlLogicNets = std::vector<std::string_view>{};
        controlLogicNets.emplace_back(SRAM1Bank::CLOCK_NAME);
        controlLogicNets.emplace_back(SRAM1Bank::REG_CHIP_SEL_BAR_NAME);
        controlLogicNets.emplace_back(SRAM1Bank::REG_WRITE_ENABLE_NAME);
        controlLogicNets.emplace_back(poweredRBL);
        controlLogicNets.emplace_back(ControlLogic::WORDLINE_ENABLE_NAME);
        controlLogicNets.emplace_back(ControlLogic::PRECHARGE_ENABLE_NAME);
        controlLogicNets.emplace_back(ControlLogic::SENSE_AMP_ENABLE_NAME);
        controlLogicNets.emplace_back(ControlLogic::WRITE_ENABLE_NAME);
        controlLogicNets.emplace_back(tech->vdd());
        controlLogicNets.emplace_back(tech->gnd());

        this->_controlLogicInst = 
            this->addInstance(ControlLogic::TYPE, this->_controlLogic, controlLogicNets);
    
        // Din Dffs
        auto dinDffsNets = std::vector<std::string_view>{};
        for (auto i : std::ranges::views::iota(0, this->_wordWidth)) 
            dinDffsNets.emplace_back(DATA_INPUT_NAME(i));
        for (auto i : std::ranges::views::iota(0, this->_wordWidth)) 
            dinDffsNets.emplace_back(REG_DATA_INPUT_NAME(i));
        dinDffsNets.emplace_back(SRAM1Bank::CLOCK_NAME);
        dinDffsNets.emplace_back(tech->vdd());
        dinDffsNets.emplace_back(tech->gnd());
        this->_dinDffsInst = this->addInstance("din_dffs", this->_dinDffs, dinDffsNets);
    
        // Row Dffs
        auto addressDffsNets = std::vector<std::string_view>{};
        for (auto i : std::ranges::views::iota(0, this->_addressWidth)) 
            addressDffsNets.emplace_back(SRAM1Bank::ADDRESS_NAME(i));
        for (auto i : std::ranges::views::iota(0, this->_addressWidth)) 
            addressDffsNets.emplace_back(SRAM1Bank::REG_ADDRESS_NAME(i));
        addressDffsNets.emplace_back(SRAM1Bank::CLOCK_NAME);
        addressDffsNets.emplace_back(tech->vdd());
        addressDffsNets.emplace_back(tech->gnd());

        this->_addressDffsInst = this->addInstance("row_addr_dffs", this->_addressDffs, addressDffsNets);
    
        // Control
        this->_controlSignalDffsInst = this->addInstance("control_dffs", this->_controlSignalDffs, {
            SRAM1Bank::CHIP_SEL_BAR_NAME, SRAM1Bank::WRITE_ENABLE_NAME,
            SRAM1Bank::REG_CHIP_SEL_BAR_NAME, SRAM1Bank::REG_WRITE_ENABLE_NAME,
            SRAM1Bank::CLOCK_NAME, tech->vdd(), tech->gnd()
        });
    }

    void SRAM1Bank::calculateLayoutConstants()
    {
        this->_vertLayer = Layer::Metal2;
        this->_horiLayer = Layer::Metal1;
        this->_viaLayer = Layer::Via1;

        this->_higherHoriLayer = Layer::Metal3;
        this->_higherVertLayer = Layer::Metal4;
        this->_higherViaLayer = Layer::Via3;
        
        this->_vertWidth = tech->layerWidth(this->_vertLayer);
        this->_vertPitch = tech->layerPitch(this->_vertLayer);

        this->_horiWidth = tech->layerWidth(this->_horiLayer);
        this->_horiPitch = tech->layerPitch(this->_horiLayer); 

        this->_higherHoriWidth = tech->layerWidth(this->_higherHoriLayer);
        this->_higherHoriPitch = tech->layerPitch(this->_higherHoriLayer);
    
        this->_higherVertWidth = tech->layerWidth(this->_higherVertLayer);
        this->_higherVertPitch = tech->layerPitch(this->_higherVertLayer);

        this->_horiInstGap = 3 * this->_horiPitch;
        this->_vertInstGap = 3 * this->_vertPitch;

        ////////////////////////////////////////////////
        this->_rowDecoderPos = Coord{0._u, 0._u};
        this->_replicatedBitcellArrayPos = Coord{
            this->_rowDecoderPos.x + this->_rowDecoder->width() + 4 * this->_vertPitch + 6 * this->_higherVertPitch,
            this->_rowDecoderPos.y
        };

        this->_prechargeArrayPos = Coord {
            this->_replicatedBitcellArrayPos.x,
            this->_replicatedBitcellArrayPos.y - this->_horiInstGap - this->_prechargeArray->height()
        };

        this->_datapathPos = Coord {
            this->_prechargeArrayPos.x,
            this->_prechargeArrayPos.y - this->_horiInstGap - this->_datapath->height()
        };

        this->_writeDriverPos = Coord {
            this->_datapathPos.x + this->_datapath->width(),
            this->_datapathPos.y
        };

        auto columnMuxArray = (dynamic_cast<DataPath*>(this->_datapath))->columnMuxArray();
        this->_bufferPos = Coord {
            this->_writeDriverPos.x - this->_vertPitch,
            this->_datapathPos.y + this->_datapath->height() - columnMuxArray->height() - this->_buffer->height() - 4 * this->_vertPitch
        };

        // VDD/GND Gap
        for (int i = 4; i >= 0; i--) {
            this->_controlBusXOffs[4 - i] = this->_datapathPos.x - (i + 2) * this->_vertPitch - 5 * this->_higherVertPitch;
        }
    }

    void SRAM1Bank::placeInstances()
    {
        this->_replicatedBitcellArrayInst->place(this->_replicatedBitcellArrayPos);
        this->_prechargeArrayInst->place(this->_prechargeArrayPos);
        this->_rowDecoderInst->place(this->_rowDecoderPos);
        this->_datapathInst->place(this->_datapathPos);
        this->_writeDriverInst->place(this->_writeDriverPos);
        this->_bufferInst->place(this->_bufferPos, MirrorType::MirrorX);

        auto maxSelectIndex = this->_columnSelSize - 1;
        auto dataPathPin 
            = this->_datapathInst->getPin(DataPath::SELECT_NAME(maxSelectIndex));
        
        auto colDecoderPin
            = this->_colDecoder->getPin(Decoder::OUTPUT_NAME(maxSelectIndex));
        
        // y + colDecoderPin.y() == dataPathPin.y()
        auto colDeocderInstY = dataPathPin.cenY() - colDecoderPin.cenY();
        this->_colDecoderPos = Coord {
            this->_controlBusXOffs[0] - this->_horiInstGap - this->_colDecoder->width() - (this->_columnSelSize + 2) * this->_vertPitch,
            // this->_prechargeArrayPos.y + this->_prechargeArray->height() - this->_colDecoder->height()
            this->_rowDecoderPos.y - (2 + this->_colAddressWidth) * this->_horiPitch - this->_colDecoder->height()
        };

        this->_controlLogicPos = Coord {
            this->_colDecoderPos.x - this->_horiInstGap - this->_controlLogic->width(),
            this->_colDecoderPos.y + this->_colDecoder->height() - this->_controlLogic->height()
        };

        auto dinDffsY = 
            this->_datapathPos.y - 
            this->_horiInstGap - 
            (this->_wordWidth * this->_higherHoriPitch) - 
            this->_horiInstGap -
            this->_dinDffs->height();
        dinDffsY = dinDffsY.min(this->_colDecoderPos.y);
        this->_dinDffsPos = Coord {
            this->_datapathPos.x, dinDffsY 
        };

        // MARK: Interval with control logic and col decoder()
        auto dffRowX = 
            this->_rowDecoderPos.x.min(this->_controlLogicPos.x) - 
            this->_addressDffs->width() - (2 + this->_colAddressWidth) * this->_vertPitch;
        this->_addressDffsPos  = Coord { dffRowX, this->_rowDecoderPos.y };
        this->_controlSignalDffsPos = Coord { dffRowX, this->_controlLogicPos.y };

        this->_colDecoderInst->place(this->_colDecoderPos);
        this->_controlLogicInst->place(this->_controlLogicPos);
        this->_dinDffsInst->place(this->_dinDffsPos);
        this->_addressDffsInst->place(this->_addressDffsPos);
        this->_controlSignalDffsInst->place(this->_controlSignalDffsPos);

        auto decoderMinY = this->_rowDecoderPos.y;
        auto deocderWLPin = this->_rowDecoderInst->getPin(RowDecoder::WORDLINE_ENABLE_NAME);
        auto centerY = (decoderMinY + decoderMinY - 3 * this->_vertPitch) / 2.;
        auto corner1 = Coord {deocderWLPin.cenX(), centerY};
        auto corner2 = Coord {this->_controlBusXOffs[WL_EN_INDEX] + this->_vertWidth / 2., centerY};
        auto busWlCoord = Coord { this->_controlBusXOffs[WL_EN_INDEX] + this->_vertWidth / 2., decoderMinY - 3 * this->_vertPitch };
        this->routePinsByPathCenter(deocderWLPin.layer(), this->_vertLayer, this->_horiLayer, std::array{
            deocderWLPin.center(), corner1, corner2, busWlCoord
        });
    }

    void SRAM1Bank::routeInstances()
    {
        this->routeColDecoder();
        this->routePrechargeArray();
        this->routeRowDecoder();
        this->routeControlLogic();
        this->routeRBL();
        this->routeDinDFlipFlops();
        this->routeAddrDFlipFlops();
        this->routeControlDFlipFlops();
        this->routeVDDGND();
        this->routeClock();
        this->routeIO();
    }

    void SRAM1Bank::routeColDecoder() 
    {
        auto xoffs = std::vector<Number>{};
        for (auto i : std::ranges::views::iota(0, this->_columnSelSize)) 
            xoffs.push_back(this->_colDecoderPos.x + this->_colDecoder->width() + this->_horiPitch + i * this->_horiPitch); 

        auto height = this->_colDecoder->height().max(this->_prechargeArrayPos.y + this->_prechargeArray->height() - this->_datapathInst->getPin(DataPath::SENSE_AMP_VDD_NAME).cenY());
        this->routeColBus(
            this->_vertLayer, 
            this->_vertWidth, height, 
            this->_colDecoderPos.y + this->_colDecoder->height() - height, 
            xoffs);

        for (auto i : std::ranges::views::iota(0, this->_columnSelSize)) 
        {
            auto decoderPin = this->_colDecoderInst->getPin(Decoder::OUTPUT_NAME(i));
            auto selPin = this->_datapathInst->getPin(DataPath::SELECT_NAME(i));

            auto busXCenter = xoffs.at(i) + 0.5 * this->_vertWidth;

            this->routePinsCenter(
                decoderPin.center(), decoderPin.layer(),
                Coord {busXCenter, decoderPin.cenY()}, this->_vertLayer,
                this->_horiLayer
            );

            this->routePinsCenter(
                selPin.center(), selPin.layer(),
                Coord {busXCenter, selPin.cenY()}, this->_vertLayer,
                this->_horiLayer
            );
        }
    }

    void SRAM1Bank::routePrechargeArray() 
    {
        auto bitcellArrayMinY = this->_replicatedBitcellArrayPos.y;
        auto prechargeArrayMaxY = this->_prechargeArrayPos.y + this->_prechargeArray->height();
        auto bicellAndPrecharGapCenterY = (bitcellArrayMinY + prechargeArrayMaxY) / 2.;

        // With Bitcell Array
        for (auto i : std::ranges::views::iota(0, this->_bitcellColSize)) {
            auto precharBLPin = this->_prechargeArrayInst->getPin(PrechargeArray::BITLINE_NAME(i));
            auto precharBRPin = this->_prechargeArrayInst->getPin(PrechargeArray::BITLINE_BAR_NAME(i));

            auto bitcellArrayBLPin = this->_replicatedBitcellArrayInst->getPin(ReplicatedBitcellArray::BITLINE_NAME(i));
            auto bitcellArrayBRPin = this->_replicatedBitcellArrayInst->getPin(ReplicatedBitcellArray::BITLINE_BAR_NAME(i));

            auto corner1 = Coord {bitcellArrayBLPin.cenX(), bicellAndPrecharGapCenterY};
            auto corner2 = Coord {precharBLPin.cenX(), bicellAndPrecharGapCenterY};
            this->routePathCenter(this->_vertLayer, std::array{bitcellArrayBLPin.center(), corner1, corner2, precharBLPin.center()});
        
            corner1 = Coord {bitcellArrayBRPin.cenX(), bicellAndPrecharGapCenterY};
            corner2 = Coord {precharBRPin.cenX(), bicellAndPrecharGapCenterY};
            this->routePathCenter(this->_vertLayer, std::array{bitcellArrayBRPin.center(), corner1, corner2, precharBRPin.center()});
        }

        auto prechargeArrayMinY = this->_prechargeArrayPos.y;
        auto dataPathMaxY = this->_datapathPos.y + this->_datapath->height();
        auto prechargeAndDataPathCenterY = (prechargeArrayMinY + dataPathMaxY)  / 2.;

        // With Data path
        for (auto i : std::ranges::views::iota(0, this->_bitcellColSize)) {
            auto precharBLPin = this->_prechargeArrayInst->getPin(PrechargeArray::BITLINE_NAME(i));
            auto precharBRPin = this->_prechargeArrayInst->getPin(PrechargeArray::BITLINE_BAR_NAME(i));

            // With data path
            auto dataBLPin = this->_datapathInst->getPin(ReplicatedBitcellArray::BITLINE_NAME(i));
            auto dataBRPin = this->_datapathInst->getPin(ReplicatedBitcellArray::BITLINE_BAR_NAME(i));

            auto corner1 = Coord {dataBLPin.cenX(), prechargeAndDataPathCenterY};
            auto corner2 = Coord {precharBLPin.cenX(), prechargeAndDataPathCenterY};
            this->routePathCenter(this->_vertLayer, std::array{dataBLPin.center(), corner1, corner2, precharBLPin.center()});
        
            corner1 = Coord {dataBRPin.cenX(), prechargeAndDataPathCenterY};
            corner2 = Coord {precharBRPin.cenX(), prechargeAndDataPathCenterY};
            this->routePathCenter(this->_vertLayer, std::array{dataBRPin.center(), corner1, corner2, precharBRPin.center()});
        }

        // Replica
        auto rblPin = this->_replicatedBitcellArrayInst->getPin(ReplicatedBitcellArray::REPLICA_BITLINE_NAME);
        auto rbrPin = this->_replicatedBitcellArrayInst->getPin(ReplicatedBitcellArray::REPLICA_BITLINE_BAR_NAME);
        auto precharBLPin = this->_prechargeArrayInst->getPin(PrechargeArray::BITLINE_NAME(this->_bitcellColSize));
        auto precharBRPin = this->_prechargeArrayInst->getPin(PrechargeArray::BITLINE_BAR_NAME(this->_bitcellColSize));

        auto corner1 = Coord {rblPin.cenX(), bicellAndPrecharGapCenterY};
        auto corner2 = Coord {precharBLPin.cenX(), bicellAndPrecharGapCenterY};
        this->routePathCenter(this->_vertLayer, std::array{rblPin.center(), corner1, corner2, precharBLPin.center()});
        
        corner1 = Coord {rbrPin.cenX(), bicellAndPrecharGapCenterY};
        corner2 = Coord {precharBRPin.cenX(), bicellAndPrecharGapCenterY};
        this->routePathCenter(this->_vertLayer, std::array{rbrPin.center(), corner1, corner2, precharBRPin.center()});
    }
    
    void SRAM1Bank::routeRowDecoder() 
    {
        /*
            Route the wl with bitcell array and row decoder!
        */
        auto decoderMaxX = this->_rowDecoderPos.x + this->_rowDecoder->width();
        auto bitcellArrayMinX = this->_replicatedBitcellArrayPos.x;
        auto centerX = decoderMaxX + 2 * this->_vertPitch;

        for (auto row : std::ranges::views::iota(0, this->_bitcellRowSize)) {
            auto decoderWLPin = this->_rowDecoderInst->getPin(RowDecoder::WORDLINE_NAME(row));
            auto bitcellWLPin = this->_replicatedBitcellArrayInst->getPin(ReplicatedBitcellArray::WORDLINE_NAME(row));

            auto corner1 = Coord { centerX, decoderWLPin.cenY() };
            auto corner2 = Coord { centerX, bitcellWLPin.cenY() };
            this->routePathCenter(this->_horiLayer, std::array{decoderWLPin.center(), corner1, corner2, bitcellWLPin.center()});
        }
    }

    void SRAM1Bank::routeControlLogic() 
    {
        auto height = this->_prechargeArrayPos.y + this->_prechargeArray->height() - this->_datapathPos.y;
        debug::check(height > this->_controlLogic->height(), "Control logic to high!");
        
        this->routeColBus(this->_vertLayer, this->_vertWidth, height, this->_datapathPos.y, this->_controlBusXOffs);

        auto clWLPin = this->_controlLogicInst->getPin(ControlLogic::WORDLINE_ENABLE_NAME);
        auto clPEPin = this->_controlLogicInst->getPin(ControlLogic::PRECHARGE_ENABLE_NAME);
        auto clSAPin = this->_controlLogicInst->getPin(ControlLogic::SENSE_AMP_ENABLE_NAME);
        auto clWEPin = this->_controlLogicInst->getPin(ControlLogic::WRITE_ENABLE_NAME);

        // MARK! Att seq
        auto clCoords = std::array<Pin, 4>{clPEPin, clWLPin, clSAPin, clWEPin};

        for (auto i : std::ranges::views::iota(0, 4)) {
            auto beginPin = clCoords[i];
            auto endCoord = Coord {this->_controlBusXOffs[i] + this->_vertWidth / 2., beginPin.cenY()};
            this->routePinsCenter(
                beginPin.center(), clWLPin.layer(),
                endCoord, this->_vertLayer,
                this->_higherHoriLayer, this->_higherHoriWidth
            );
        }

        auto datapathSAPin = this->_datapathInst->getPin(DataPath::SENSE_AMP_ENABLE_NAME);
        auto busSACoord = Coord { this->_controlBusXOffs[SA_EN_INDEX] + this->_vertWidth / 2., datapathSAPin.cenY() };
        this->routePinsCenter(
            datapathSAPin.center(), datapathSAPin.layer(),
            busSACoord, this->_vertLayer,
            this->_horiLayer, this->_horiWidth 
        );

        auto datapathWEPin = this->_datapathInst->getPin(DataPath::WRITE_DRIVER_ENABLE_NAME);
        auto busWECoord = Coord { this->_controlBusXOffs[WE_EN_INEDX] + this->_vertWidth / 2., datapathWEPin.cenY() };
        this->routePinsCenter(
            datapathWEPin.center(), datapathWEPin.layer(),
            busWECoord, this->_vertLayer,
            this->_horiLayer, this->_horiWidth 
        );

        auto connect = [this] (const Pin& datapathPin, int index) {
            auto coord = Coord { this->_controlBusXOffs[index] + this->_vertWidth / 2., datapathPin.cenY() };
            this->routePinsCenter(
                datapathPin.center(), datapathPin.layer(),
                coord, this->_vertLayer,
                this->_horiLayer, this->_horiWidth 
            );
        };

        connect(this->_datapathInst->getPin(DataPath::WRITE_DRIVER_ENABLE_NAME), WE_EN_INEDX);
        connect(this->_datapathInst->getPin(DataPath::SENSE_AMP_ENABLE_NAME), SA_EN_INDEX);
        connect(this->_prechargeArrayInst->getPin(PrechargeArray::ENABLE_NAME), P_EN_INDEX);
    }

    void SRAM1Bank::routeRBL() 
    {
        auto enPin = this->_writeDriverInst->getPin(WriteDriver::ENABLE_NAME);
        // MARK: Maybe connect?
        this->addRectangleSegmentCenter(
            enPin.layer(), 
            Coord { this->_controlBusXOffs[WE_EN_INEDX] + this->_vertWidth / 2., enPin.cenY() },
            enPin.center()
        );

        // Connect Write Driver RBL 
        auto precharBLPin = this->_prechargeArrayInst->getPin(PrechargeArray::BITLINE_NAME(this->_bitcellColSize));
        auto precharBRPin = this->_prechargeArrayInst->getPin(PrechargeArray::BITLINE_BAR_NAME(this->_bitcellColSize));

        // With data path
        auto writeDriverBLPin = this->_writeDriverInst->getPin(WriteDriver::BITLINE_NAME);
        auto writeDriverBRPin = this->_writeDriverInst->getPin(WriteDriver::BITLINE_BAR_NAME);

        auto cornerY = this->_prechargeArrayPos.y - 5 * this->_horiPitch;

        auto corner1 = Coord {writeDriverBLPin.cenX(), cornerY};
        auto corner2 = Coord {precharBLPin.cenX(), cornerY};
        this->routePathCenter(this->_vertLayer, std::array{writeDriverBLPin.center(), corner1, corner2, precharBLPin.center()});
    
        corner1 = Coord {writeDriverBRPin.cenX(), cornerY};
        corner2 = Coord {precharBRPin.cenX(), cornerY};
        this->routePathCenter(this->_vertLayer, std::array{writeDriverBRPin.center(), corner1, corner2, precharBLPin.center()});
    
        // Connect Buffer
        auto bufferInPin = this->_bufferInst->getPin(Buffer::INPUT_NAME);
        this->routePinsCenter(
            bufferInPin.center(), bufferInPin.layer(), 
            Coord {writeDriverBLPin.cenX(), bufferInPin.cenY()},
            writeDriverBLPin.layer(),
            writeDriverBLPin.layer()
        );

        // Route to control 
        auto bufferOutPin = this->_bufferInst->getPin(Buffer::OUTPUT_NAME);
        auto bufferLeftDownPos = this->_bufferPos - Coord {this->_buffer->width(), 0._u};
        this->routePinsByPathCenter(bufferOutPin.layer(), this->_vertLayer, this->_horiLayer, std::array{
            bufferOutPin.center(),
            Coord {bufferLeftDownPos.x - this->_vertPitch, bufferOutPin.cenY()},
            Coord {bufferLeftDownPos.x - this->_vertPitch, bufferLeftDownPos.y + this->_buffer->height() +  2 * this->_horiPitch},
            Coord {this->_controlBusXOffs[RBL_INDEX] + 0.5 * this->_vertWidth, bufferLeftDownPos.y + this->_buffer->height() +  2 * this->_horiPitch},
        });
    
        auto rblPin = this->_controlLogicInst->getPin(ControlLogic::REPLICA_BL_NAME);
        this->routePinsCenter(
            rblPin.center(), rblPin.layer(), 
            Coord { this->_controlBusXOffs[RBL_INDEX] + 0.5 * this->_vertWidth, rblPin.cenY() }, this->_vertLayer,
            this->_higherHoriLayer
        );

        // Route rwl
        auto rwlXOff = this->_replicatedBitcellArrayPos.x + this->_replicatedBitcellArray->width() + 2 * this->_vertPitch;
        auto highestRWLPin = this->_replicatedBitcellArrayInst->getPin(ReplicatedBitcellArray::REPLICA_WORDLINE_NAME);
        for (auto rwlPin : this->_replicatedBitcellArrayInst->getPins(ReplicatedBitcellArray::REPLICA_WORDLINE_NAME))
        {
            if (highestRWLPin.cenY() < rwlPin.cenY()) 
                highestRWLPin = rwlPin;
            this->addRectangleSegmentCenter(rwlPin.layer(), rwlPin.center(), Coord{rwlXOff, rwlPin.cenY()});
            this->addViaThroughtLayersCenter(rwlPin.layer(), this->_vertLayer, Coord{rwlXOff, rwlPin.cenY()});
        }
        
        this->addRectangleSegmentCenter(
            this->_vertLayer, 
            Coord {rwlXOff, highestRWLPin.cenY()},
            Coord{rwlXOff, this->_datapathPos.y});
        
        auto yOff = bufferLeftDownPos.y + this->_buffer->height() +  3 * this->_horiPitch;
        this->routePinsCenter(
            Coord {rwlXOff, yOff}, this->_vertLayer,
            Coord {this->_controlBusXOffs[WL_EN_INDEX], yOff}, this->_vertLayer,
            this->_horiLayer
        );
    }

    void SRAM1Bank::routeDinDFlipFlops() 
    {
        for (auto i : std::ranges::views::iota(0, this->_wordWidth))
        {
            auto dffDoutPin = this->_dinDffsInst->getPin(DFlipFlopRow::DOUT_NAME(i));
            auto dataPatDinPin = this->_datapathInst->getPin(DataPath::DATA_INPUT_NAME(i));

            auto cornerY = 
                this->_dinDffsPos.y + 
                this->_dinDffs->height() + 
                i * this->_higherHoriPitch +
                this->_horiInstGap;
                
            auto corner1 = Coord {dffDoutPin.cenX(), cornerY};
            auto corner2 = Coord {dataPatDinPin.cenX(), cornerY};

            this->routePinsCenter(
                dffDoutPin.center(), dffDoutPin.layer(),
                corner1, this->_higherVertLayer,
                this->_higherVertLayer
            );

            this->routePinsCenter(
                corner1, this->_higherVertLayer,
                corner2, this->_higherVertLayer,
                this->_higherHoriLayer
            );

            this->routePinsCenter(
                corner2, this->_higherVertLayer,
                dataPatDinPin.center(), dataPatDinPin.layer(),
                this->_higherVertLayer
            );
        }
    }

    void SRAM1Bank::routeAddrDFlipFlops() 
    {
        for (auto i : std::ranges::views::iota(this->_colAddressWidth, this->_addressWidth)) 
        {
            auto addrPin = this->_rowDecoderInst->getPin(RowDecoder::ADDRESS_NAME(i - this->_colAddressWidth));
            auto regPin = this->_addressDffsInst->getPin(DFlipFlopColumn::DOUT_NAME(i));
            // MARK: !!! addrPin != vert layer
            this->routePinsCenter(
                regPin.center(), regPin.layer(),
                Coord {addrPin.cenX(), regPin.cenY()}, this->_vertLayer,
                this->_horiLayer
            );
        }

        // Route Col bus
        auto colAddrBusXOffs = std::vector<Number>{};
        for (auto i : std::ranges::views::iota(0, this->_colAddressWidth))
            colAddrBusXOffs.emplace_back(this->_addressDffsPos.x + this->_addressDffs->width() + (i + 1) * this->_vertPitch);
        auto highestAddrPin = this->_addressDffsInst->getPin(DFlipFlopColumn::DOUT_NAME(this->_colAddressWidth - 1));
        // auto minY = this->_controlLogicPos.y.min(this->_colDecoderPos.y) - (1 + this->_colAddressWidth) * this->_horiPitch;
        auto minY = this->_colDecoderPos.y + this->_colDecoder->height() + this->_horiPitch;
        this->routeColBus(
            this->_vertLayer, 
            this->_vertWidth, highestAddrPin.maxY() - minY, 
            minY, colAddrBusXOffs);

        for (auto i : std::ranges::views::iota(0, this->_colAddressWidth))
        {
            auto addrPin = this->_addressDffsInst->getPin(DFlipFlopColumn::DOUT_NAME(i));
            this->routePinsCenter(
                addrPin.center(), addrPin.layer(),
                Coord { colAddrBusXOffs.at(i) + 0.5 * this->_vertWidth, addrPin.cenY() }, this->_vertLayer,
                this->_horiLayer
            );
        }

        auto colAddrBusYOffs = std::vector<Number>{};
        for (auto i : std::ranges::views::iota(0, this->_colAddressWidth))
            colAddrBusYOffs.emplace_back(minY + i * this->_horiPitch);
        this->routeRowBus(
            this->_horiLayer,
            this->_colDecoderPos.x + this->_colDecoder->width() - colAddrBusXOffs.at(0), this->_horiWidth,
            colAddrBusXOffs.at(0), colAddrBusYOffs
        );

        // Add Via
        for (auto i : std::ranges::views::iota(0, this->_colAddressWidth))
        {
            this->addViaThroughtLayersCenter(
                this->_horiLayer, this->_vertLayer, 
                Coord { colAddrBusXOffs.at(i) + 0.5 * this->_vertWidth, colAddrBusYOffs.at(i) + 0.5 * this->_horiWidth }
            );
        }

        for (auto i : std::ranges::views::iota(0, this->_colAddressWidth))
        {
            auto inputPin = this->_colDecoderInst->getPin(Decoder::ADDRESS_NAME(i));
            this->routePinsCenter(
                inputPin.center(), inputPin.layer(),
                Coord {inputPin.cenX(), colAddrBusYOffs.at(i) + 0.5 * this->_horiWidth}, this->_horiLayer,
                _vertLayer
            );
        }
    }

    void SRAM1Bank::routeControlDFlipFlops()
    {
        auto csbRegPin = this->_controlSignalDffsInst->getPin(DFlipFlopColumn::DOUT_NAME(0));
        auto cbsPin = this->_controlLogicInst->getPin(ControlLogic::CHIP_SEL_BAR_NAME);
        this->routePinsCenter(
            csbRegPin.center(), csbRegPin.layer(),
            Coord {cbsPin.cenX(), csbRegPin.cenY()}, this->_vertLayer,
            this->_horiLayer
        );

        auto weRegPin = this->_controlSignalDffsInst->getPin(DFlipFlopColumn::DOUT_NAME(1));
        auto wePin = this->_controlLogicInst->getPin(ControlLogic::WRITE_NAME);
        this->routePinsCenter(
            weRegPin.center(), weRegPin.layer(),
            Coord {wePin.cenX(), weRegPin.cenY()}, this->_vertLayer,
            this->_horiLayer
        );
    }

    void SRAM1Bank::routeVDDGND()
    {
        // Route vdd/gnd throught line!
        auto lleftVDDX = this->_addressDffsPos.x - 3 * this->_higherVertPitch;
        auto leftVDDX = this->_replicatedBitcellArrayPos.x - 3 * this->_higherVertPitch;
        auto rightVDDX = this->_replicatedBitcellArrayPos.x + this->_replicatedBitcellArray->width() + 3 * this->_higherVertPitch;

        auto lleftGNDX = this->_addressDffsPos.x - 5 * this->_higherVertPitch;
        auto leftGNDX = this->_replicatedBitcellArrayPos.x - 5 * this->_higherVertPitch;
        auto rightGNDX = this->_replicatedBitcellArrayPos.x + this->_replicatedBitcellArray->width() + 5 * this->_higherVertPitch;

        auto lleftVDDCenterX = lleftVDDX + 0.5 * this->_higherVertWidth;
        auto leftVDDCenterX = leftVDDX + 0.5 * this->_higherVertWidth;
        auto rightVDDCenterX = rightVDDX + 0.5 * this->_higherVertWidth;

        auto lleftGNDCenterX = lleftGNDX + 0.5 * this->_higherVertWidth;
        auto leftGNDCenterX = leftGNDX + 0.5 * this->_higherVertWidth;
        auto rightGNDCenterX = rightGNDX + 0.5 * this->_higherVertWidth;

        auto sourceHeight = this->_replicatedBitcellArrayPos.y + this->_replicatedBitcellArray->height() - this->_dinDffsPos.y;

        this->addRectangle(
            this->_higherVertLayer, 
            Coord{leftVDDX, this->_dinDffsPos.y},
            this->_higherVertWidth,
            sourceHeight
        );
        this->addRectangle(
            this->_higherVertLayer, 
            Coord{leftGNDX, this->_dinDffsPos.y},
            this->_higherVertWidth,
            sourceHeight
        );

        this->addRectangle(
            this->_higherVertLayer, 
            Coord{rightVDDX, this->_dinDffsPos.y},
            this->_higherVertWidth,
            sourceHeight
        );
        this->addRectangle(
            this->_higherVertLayer, 
            Coord{rightGNDX, this->_dinDffsPos.y},
            this->_higherVertWidth,
            sourceHeight
        );

        this->addRectangle(
            this->_higherVertLayer, 
            Coord{lleftGNDX, this->_dinDffsPos.y},
            this->_higherVertWidth,
            sourceHeight
        );
        this->addRectangle(
            this->_higherVertLayer, 
            Coord{lleftVDDX, this->_dinDffsPos.y},
            this->_higherVertWidth,
            sourceHeight
        );

        // Instance between with two throught vdd/gnd
        {
            auto routeVDD = [leftVDDCenterX, rightVDDCenterX, this] (const Number& centerY, Layer routeLayer) {
                auto leftCoord = Coord{leftVDDCenterX, centerY};
                auto rightCoord = Coord{rightVDDCenterX, centerY};
                this->addRectangleSegmentCenter(routeLayer, leftCoord, rightCoord);
                this->addViaThroughtLayersCenter(routeLayer, this->_higherVertLayer, leftCoord);
                this->addViaThroughtLayersCenter(routeLayer, this->_higherVertLayer, rightCoord);      
            };

            auto routeGND = [leftGNDCenterX, rightGNDCenterX, this] (const Number& centerY, Layer routeLayer) {
                auto leftCoord = Coord{leftGNDCenterX, centerY};
                auto rightCoord = Coord{rightGNDCenterX, centerY};
                this->addRectangleSegmentCenter(routeLayer, leftCoord, rightCoord);
                this->addViaThroughtLayersCenter(routeLayer, this->_higherVertLayer, leftCoord);
                this->addViaThroughtLayersCenter(routeLayer, this->_higherVertLayer, rightCoord);
            };

            // Bitcell array
            for (auto vddPin : this->_replicatedBitcellArray->getPins(tech->vdd())) 
            {
                assert(vddPin.layer() == this->_horiLayer);
                routeVDD(vddPin.cenY(), this->_horiLayer);
            }

            for (auto gndPin : this->_replicatedBitcellArray->getPins(tech->gnd())) 
            {
                assert(gndPin.layer() == this->_horiLayer);
                routeGND(gndPin.cenY(), this->_horiLayer);
            }

            // Precharge array
            auto preVDDPin = this->_prechargeArrayInst->getPin(tech->vdd());
            routeVDD(preVDDPin.cenY(), this->_horiLayer);

            // Data path
            // MARK: Ensure vdd/gnd in hori layer
            routeGND(this->_datapathInst->getPin(DataPath::COLUMN_MUX_GND_NAME).cenY(), this->_horiLayer);

            routeVDD(this->_datapathInst->getPin(DataPath::SENSE_AMP_VDD_NAME).cenY(), this->_horiLayer);
            routeGND(this->_datapathInst->getPin(DataPath::SENSE_AMP_GND_NAME).cenY(), this->_horiLayer);

            routeVDD(this->_datapathInst->getPin(DataPath::WRITE_DRIVER_VDD_NAME).cenY(), this->_horiLayer);
            routeGND(this->_datapathInst->getPin(DataPath::WRITE_DRIVER_GND_NAME).cenY(), this->_horiLayer);
        
            // Row dffs
            routeVDD(this->_dinDffsInst->getPin(tech->vdd()).cenY(), this->_horiLayer);
            routeGND(this->_dinDffsInst->getPin(tech->gnd()).cenY(), this->_horiLayer);
        }

        // Route Row decoder
        {
            for (auto vddPin : this->_rowDecoderInst->getPins(tech->vdd())) 
            {
                assert(vddPin.layer() == this->_horiLayer);
                this->addRectangleSegmentCenter(
                    vddPin.layer(),
                    vddPin.center(),
                    Coord { leftVDDCenterX, vddPin.cenY() }
                );
            }
            for (auto gndPin : this->_rowDecoderInst->getPins(tech->gnd())) 
            {
                assert(gndPin.layer() == this->_horiLayer);
                this->addRectangleSegmentCenter(
                    gndPin.layer(),
                    gndPin.center(),
                    Coord { leftGNDCenterX, gndPin.cenY() }
                );
            }
        }

        // Route control logic and col decoder
        {
            for (auto vddPin : this->_controlLogicInst->getPins(tech->vdd())) 
            {
                this->addRectangleSegmentCenter(
                    vddPin.layer(), vddPin.center(), Coord {this->_colDecoderPos.x, vddPin.cenY()});
            }

            for (auto gndPin : this->_controlLogicInst->getPins(tech->gnd())) 
            {
                this->addRectangleSegmentCenter(
                    gndPin.layer(), gndPin.center(), Coord {this->_colDecoderPos.x, gndPin.cenY()});
            }

            auto height = this->_colDecoder->height().max(this->_controlLogic->height());
            auto yOff = this->_colDecoderPos.y.min(this->_controlLogicPos.y);
            auto vddXOff = this->_controlLogicPos.x + this->_controlLogic->width() + 2 * this->_vertPitch;
            auto gndXOff = this->_controlLogicPos.x + this->_controlLogic->width() + 3 * this->_vertPitch;

            // Route vdd/gnd line inner
            this->addRectangle(this->_vertLayer, Coord{vddXOff, yOff}, this->_vertWidth, height);
            this->addRectangle(this->_vertLayer, Coord{gndXOff, yOff}, this->_vertWidth, height);
            
            auto higherInst = this->_controlLogicInst;
            if (this->_controlLogic->height() < this->_colDecoder->height()) 
                higherInst = this->_colDecoderInst;
            
            auto minVddPin = higherInst->getPin(tech->vdd());
            for (auto vddPin : higherInst->getPins(tech->vdd()))
            {
                auto coord = Coord { vddXOff + 0.5 * this->_vertWidth, vddPin.cenY() };
                this->addViaThroughtLayersCenter(this->_vertLayer, vddPin.layer(), coord);
                if (vddPin.cenY() < minVddPin.cenY())
                    minVddPin = vddPin;
            }

            auto minGndPin = higherInst->getPin(tech->gnd());
            for (auto gndPin : higherInst->getPins(tech->gnd()))
            {
                auto coord = Coord { gndXOff + 0.5 * this->_vertWidth, gndPin.cenY() };
                this->addViaThroughtLayersCenter(this->_vertLayer, gndPin.layer(), coord);
                if (gndPin.cenY() < minGndPin.cenY())
                    minVddPin = gndPin;
            }
            
            this->routePinsCenter(
                minVddPin.center(), minVddPin.layer(),
                Coord {leftVDDCenterX, minVddPin.cenY()}, this->_higherVertLayer,
                this->_horiLayer
            );
            this->routePinsCenter(
                minGndPin.center(), minGndPin.layer(),
                Coord {leftGNDCenterX, minGndPin.cenY()}, this->_higherVertLayer,
                this->_horiLayer
            );
        }

        // Buffer
        {
            auto vddPin = this->_bufferInst->getPin(tech->vdd());
            this->routePinsCenter(
                vddPin.center(), vddPin.layer(),
                Coord { rightVDDCenterX, vddPin.cenY() }, this->_higherVertLayer,
                vddPin.layer()
            );
    
            auto gndPin = this->_bufferInst->getPin(tech->gnd());
            this->routePinsCenter(
                gndPin.center(), gndPin.layer(),
                Coord { rightGNDCenterX, gndPin.cenY() }, this->_higherVertLayer,
                gndPin.layer()
            );
        }
    
        // Row address
        {
            for (auto pin : this->_controlSignalDffsInst->getPins(tech->vdd())) 
            {
                this->routePinsCenter(
                    pin.center(), pin.layer(),
                    Coord { lleftVDDCenterX, pin.cenY() }, this->_higherVertLayer,
                    this->_horiLayer
                );
            }
    
            for (auto pin : this->_controlSignalDffsInst->getPins(tech->gnd())) 
            {
                this->routePinsCenter(
                    pin.center(), pin.layer(),
                    Coord { lleftGNDCenterX, pin.cenY() }, this->_higherVertLayer,
                    this->_horiLayer
                );
            }
        }

        // Control dffs
        {
            for (auto pin : this->_addressDffsInst->getPins(tech->vdd())) 
            {
                this->routePinsCenter(
                    pin.center(), pin.layer(),
                    Coord { lleftVDDCenterX, pin.cenY() }, this->_higherVertLayer,
                    this->_horiLayer
                );
            }
    
            for (auto pin : this->_addressDffsInst->getPins(tech->gnd())) 
            {
                this->routePinsCenter(
                    pin.center(), pin.layer(),
                    Coord { lleftGNDCenterX, pin.cenY() }, this->_higherVertLayer,
                    this->_horiLayer
                );
            }
        }
    }

    void SRAM1Bank::routeClock()
    {
        auto colClockXOff = this->_addressDffsInst->getPin(DFlipFlopColumn::CLOCK_NAME).cenX();
        auto rowClockYOff = this->_dinDffsInst->getPin(DFlipFlopColumn::CLOCK_NAME).cenY();

        auto corner = Coord { colClockXOff, rowClockYOff };
        this->addRectangleSegmentCenter(
            this->_higherVertLayer,
            corner, Coord {corner.x, this->_addressDffsPos.y + this->_addressDffs->height()}
        );
        this->addRectangleSegmentCenter(
            this->_higherHoriLayer,
            corner, Coord { this->_dinDffsPos.x + this->_dinDffs->width(), corner.y }
        );
        this->addViaThroughtLayersCenter(this->_higherHoriLayer, this->_higherVertLayer, corner);
        
        for (auto clkPin : this->_addressDffsInst->getPins(DFlipFlopColumn::CLOCK_NAME))
            this->addViaThroughtLayersCenter(this->_higherVertLayer, clkPin.layer(), clkPin.center());
        for (auto clkPin : this->_controlSignalDffs->getPins(DFlipFlopColumn::CLOCK_NAME))
            this->addViaThroughtLayersCenter(this->_higherVertLayer, clkPin.layer(), clkPin.center());
    
        for (auto clkPin : this->_dinDffsInst->getPins(DFlipFlopColumn::CLOCK_NAME))
            this->addViaThroughtLayersCenter(this->_higherHoriLayer, clkPin.layer(), clkPin.center());
    }

    void SRAM1Bank::routeIO()
    {
        auto lineWidth = (2. * this->_higherHoriWidth).max(2. * this->_higherVertWidth);

        auto leftdownCoord = Coord {
            this->_addressDffsPos.x - 7 * this->_higherVertPitch - lineWidth,
            this->_dinDffsPos.y - 4 * this->_higherHoriPitch - lineWidth,
        };

        auto rightUpCoord = Coord {
            this->_replicatedBitcellArrayPos.x + this->_replicatedBitcellArray->width() + 6 * this->_higherVertPitch + lineWidth,
            this->_replicatedBitcellArrayPos.y + this->_replicatedBitcellArray->height() + 3 * this->_higherHoriPitch + lineWidth,
        };

        auto vddLeftdownCenterCoord = Coord { leftdownCoord.x + 0.5 * lineWidth, leftdownCoord.y + 0.5 * lineWidth };
        auto vddRightUpCenterCoord = Coord { rightUpCoord.x + 0.5 * lineWidth, rightUpCoord.y + 0.5 * lineWidth };

        auto gndLeftdownCenterCoord = Coord { vddLeftdownCenterCoord.x - 2 * lineWidth, vddLeftdownCenterCoord.y - 2 * lineWidth };
        auto gndRightUpCenterCoord = Coord { vddRightUpCenterCoord.x + 2 * lineWidth, vddRightUpCenterCoord.y + 2 * lineWidth };

        this->routeSourceRingCenter(vddLeftdownCenterCoord, vddRightUpCenterCoord, lineWidth);
        this->routeSourceRingCenter(gndLeftdownCenterCoord, gndRightUpCenterCoord, lineWidth);

        // IO
        {
            auto routeToLeft = [this, gndLeftdownCenterCoord] (const Pin& pin, std::string_view name) {
                auto coord = Coord {gndLeftdownCenterCoord.x, pin.cenY()};
                this->routePinsCenter(
                    pin.center(), pin.layer(),
                    coord, this->_horiLayer,
                    this->_horiLayer
                );
                this->addLabelPinCenter(this->_horiLayer, name, coord);
            };

            for (auto i : std::ranges::views::iota(0, this->_addressWidth))
                routeToLeft(
                    this->_addressDffsInst->getPin(DFlipFlopColumn::DIN_NAME(i)),
                    SRAM1Bank::ADDRESS_NAME(i)
                );
                
            routeToLeft(
                this->_controlSignalDffsInst->getPin(DFlipFlopColumn::DIN_NAME(0)),
                SRAM1Bank::CHIP_SEL_BAR_NAME
            );
            routeToLeft(
                this->_controlSignalDffsInst->getPin(DFlipFlopColumn::DIN_NAME(1)),
                SRAM1Bank::WRITE_ENABLE_NAME
            );

            auto routeToDown = [this, gndLeftdownCenterCoord] (const Pin& pin, std::string_view name) {
                auto coord = Coord {pin.cenX(), gndLeftdownCenterCoord.y};
                this->routePinsCenter(
                    pin.center(), pin.layer(),
                    coord, this->_vertLayer,
                    this->_vertLayer
                );
                this->addLabelPinCenter(this->_vertLayer, name, coord);
            };

            for (auto i : std::ranges::views::iota(0, this->_wordWidth))
                routeToDown(
                    this->_dinDffsInst->getPin(DFlipFlopRow::DIN_NAME(i)),
                    SRAM1Bank::DATA_INPUT_NAME(i)
                );

            auto routeToRight = [this, gndRightUpCenterCoord] (const Pin& pin, std::string_view name) {
                auto coord = Coord {gndRightUpCenterCoord.x, pin.cenY()};
                this->routePinsCenter(
                    pin.center(), pin.layer(),
                    coord, this->_horiLayer,
                    this->_horiLayer
                );
                this->addLabelPinCenter(this->_horiLayer, name, coord);
            };

            for (auto i : std::ranges::views::iota(0, this->_wordWidth))
                routeToRight(
                    this->_datapathInst->getPin(DataPath::DATA_OUTPUT_NAME(i)),
                    SRAM1Bank::DATA_OUTPUT_NAME(i)
                );

            auto clkCoord = Coord {
                this->_controlSignalDffsInst->getPin(DFlipFlopColumn::CLOCK_NAME).cenX(),
                this->_controlSignalDffsPos.y + this->_controlSignalDffs->height() + 3 * this->_horiPitch
            };
            auto clkIOCoord = Coord{gndLeftdownCenterCoord.x, clkCoord.y};
            this->routePinsCenter(
                clkCoord, this->_higherHoriLayer,
                clkIOCoord, this->_horiLayer,
                this->_horiLayer
            );
            this->addLabelPinCenter(this->_horiLayer, SRAM1Bank::CLOCK_NAME, clkIOCoord);
        }

        this->_width.emplace(gndRightUpCenterCoord.x - gndLeftdownCenterCoord.x + lineWidth);
        this->_height.emplace(gndRightUpCenterCoord.y - gndLeftdownCenterCoord.y + lineWidth);

        this->addRectangle(Layer::PRBoundary, 
            Coord {gndLeftdownCenterCoord.x - 0.5 * lineWidth, gndLeftdownCenterCoord.y - 0.5 * lineWidth},
            this->_width, this->_height
        );
    }

    void SRAM1Bank::createLayoutPins()
    {

    }

    void SRAM1Bank::layoutMiscellaneous() 
    {

    }

    void SRAM1Bank::routeSourceRingCenter(const Coord& ld, const Coord& ru, const Number& width)
    {
        this->addRectangleSegmentCenter(this->_higherHoriLayer, ld, Coord{ru.x, ld.y}, width);
        this->addRectangleSegmentCenter(this->_higherHoriLayer, Coord{ld.x, ru.y}, ru, width);
        this->addRectangleSegmentCenter(this->_higherVertLayer, ld, Coord{ld.x, ru.y}, width);
        this->addRectangleSegmentCenter(this->_higherVertLayer, Coord{ru.x, ld.y}, ru, width);
    }

    auto SRAM1Bank::ADDRESS_NAME(int index) -> std::string_view
    {
        return stringf("addr{}", index);
    }

    auto SRAM1Bank::DATA_INPUT_NAME(int index) -> std::string_view
    {
        return stringf("din{}", index);
    }

    auto SRAM1Bank::DATA_OUTPUT_NAME(int index) -> std::string_view
    {
        return stringf("dout{}", index);
    }

    auto SRAM1Bank::REG_ADDRESS_NAME(int index) -> std::string_view 
    {
        return stringf("addr{}_reg", index);
    }
    
    auto SRAM1Bank::REG_DATA_INPUT_NAME(int index) -> std::string_view 
    {
        return stringf("din{}_reg", index);
    }

}