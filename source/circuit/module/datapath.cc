#include "./datapath.hh"
#include "circuit/geometry/coord.hh"
#include "circuit/module/columnmuxarr.hh"
#include "global/tech/layer.hh"

#include <circuit/module/bitcellarray.hh>
#include <circuit/module/rowdecoder.hh>
#include <circuit/module/replicabank.hh>
#include <circuit/module/prechargearr.hh>
#include <circuit/module/writedriverarr.hh>
#include <circuit/module/senseamparr.hh>

#include <circuit/mod.hh>

#include <ranges>
#include <utility/misc/string.hh>
#include <utility/misc/math.hh>
#include <utility/debug/debug.hh>
#include <global/tech/tech.hh>

namespace stdv = std::ranges::views;

namespace xtaro::circuit
{

    /*
    
         |  | |  | |  | |  | |  | |  | |  | |  | |  | |  | |  | 
        +----+----+----+----+----+----+----+----+----+----+----+
        |    |    |    |   ...   |    |    |    |    |    |    |
        | PR | PR | PR |   ...   | PR | PR | PR | PR | PR | PR |
        |    |    |    |   ...   |    |    |    |    |    |    |
        +----+----+----+----+----+----+----+----+----+----+----+
         |  | |  | |  | |  | |  | |  | |  | |  | |  | |  | |  | 
         |  | |  | |  | |  | |  | |  | |  | |  | |  | |  | |  | 
        +----+----+----+----+----+----+----+----+----+----+----+
        |    |    |    |   ...   |    |    |    |    |    |    |
        | CL | CL | CL |   ...   | CL | CL | CL | CL | CL | CL |
        |    |    |    |   ...   |    |    |    |    |    |    |
        +----+----+----+----+----+----+----+----+----+----+----+ 
         |  | |  | |  | |  | |  | |  | |  | |  | |  | |  | |  | 
        --------------------------------------------------------
        --------------------------------------------------------
        --------------------------------------------------------
        --------------------------------------------------------
         |  |      |  |      |  |      |  |      |  |      |  |
        +----+    +----+    +----+    +----+    +----+    +----+
        |    |    |    |    |    |    |    |    |    |    |    |
        |    |    |    |    |    |    |    |    |    |    |    |
        | SA |    | SA |    | SA |    | SA |    | SA |    | SA |
        |    |    |    |    |    |    |    |    |    |    |    |
        |    |    |    |    |    |    |    |    |    |    |    |
        +----+    +----+    +----+    +----+    +----+    +----+
         |  |      |  |      |  |      |  |      |  |      |  |
         |  +------------------------------------------------------
         |  |      |  +--------------------------------------------
         |  |      |  |      |  +----------------------------------
         |  |      |  |      |  |      |  +------------------------
         |  |      |  |      |  |      |  |      |  +--------------
         |  |      |  |      |  |      |  |      |  |      |  +----
         |  |      |  |      |  |      |  |      |  |      |  |
        +----+    +----+    +----+    +----+    +----+    +----+
        |    |    |    |    |    |    |    |    |    |    |    |
        |    |    |    |    |    |    |    |    |    |    |    |
        | WD |    | WD |    | WD |    | WD |    | WD |    | WD |
        |    |    |    |    |    |    |    |    |    |    |    |
        |    |    |    |    |    |    |    |    |    |    |    |
        +----+    +----+    +----+    +----+    +----+    +----+

    */

    std::string DataPath::Arguments::toString() const
    {
        return std::format("ww{}_caw{}", this->wordWidth, this->columnSelSize);
    }

    auto DataPath::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<DataPath>(circuitName, *this);
    }

    DataPath::DataPath(std::string_view name, const DataPath::Arguments& arguments) :
        Module{name},
        _wordWidth{arguments.wordWidth},
        _columnSelSize{arguments.columnSelSize},

        _columnSize{arguments.columnSelSize * arguments.wordWidth},
        _useMux{this->_columnSelSize > 1}
    {
        debug::checkf(this->_wordWidth >= 1, "Word width '{}' < 1", this->_wordWidth, this->_wordWidth);
        debug::checkf(this->_columnSelSize >= 1, "Column sel size '{}' < 1", this->_wordWidth, this->_wordWidth);

        this->createSelf();
    }

    void DataPath::createPorts()
    {
        this->addPortNet(this->SENSE_AMP_ENABLE_NAME, PortType::Input);
        this->addPortNet(this->WRITE_DRIVER_ENABLE_NAME, PortType::Input);
        
        for (int i = 0; i < this->_columnSize; ++i)
            this->addPortNet( DataPath::BITLINE_NAME(i), PortType::Input);

        for (int i = 0; i < this->_columnSize; ++i)
            this->addPortNet( DataPath::BITLINE_BAR_NAME(i), PortType::Input);

        if (this->_useMux) 
        {
            for (int i = 0; i < this->_columnSelSize; ++i)
                this->addPortNet( DataPath::SELECT_NAME(i), PortType::Input);
        }

        for (int i = 0; i < this->_wordWidth; ++i)
            this->addPortNet( DataPath::DATA_INPUT_NAME(i), PortType::Input);

        for (int i = 0; i < this->_wordWidth; ++i)
            this->addPortNet( DataPath::DATA_OUTPUT_NAME(i), PortType::Input);

        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void DataPath::createCircuits()
    {
        if (this->_useMux) 
        {
            ColumnMuxArray::Arguments args {this->_columnSelSize, this->_wordWidth};
            this->_columnMuxArray = this->addCircuit(args);
        }

        this->_writedriverArray = 
            this->addCircuit(WriteDriverArray::Arguments {this->_wordWidth, this->_columnSelSize - 1});

        this->_senseampArray = 
            this->addCircuit(SenseAmplifierArray::Arguments {this->_wordWidth, this->_columnSelSize - 1});
    }

    void DataPath::createInstances() 
    {
        auto outBlNets = std::vector<std::string_view>{};
        auto outBrNets = std::vector<std::string_view>{};

        if (this->_useMux) 
        {
            for (int i = 0; i < this->_wordWidth; ++i)
            {
                outBlNets.emplace_back(stringf("out_bl{}", i));
                outBrNets.emplace_back(stringf("out_br{}", i));
            }

            std::vector<std::string_view> blNets{};
            std::vector<std::string_view> brNets{};
            std::vector<std::string_view> coladdrNets{};

            for (int i = 0; i < this->_columnSelSize; ++i)
                coladdrNets.emplace_back(DataPath::SELECT_NAME(i));

            for (int i = 0; i < this->_columnSize; ++i)
            {
                blNets.emplace_back(DataPath::BITLINE_NAME(i));
                brNets.emplace_back(DataPath::BITLINE_BAR_NAME(i));
            }

            // Create and mux array
            /*
                - Ports: 
                    -- sel0 sel1 ...
                    
                    -- bl0_0 bl0_1 ... 
                    -- br0_0 br0_1 ...
                    -- bl1_0 bl1_1 ...
                    -- br1_0 br1_1 ...
                    -- ...
                    -- blm-1_0 blm-1_1 ...
                    -- brm-1_0 brm-1_1 ...

                    -- bl0 br0 bl1 br1 ...

                    -- vdd gnd
            */
            auto muxarrayNets = std::vector<std::string_view>{};

            for (auto& coladdrNet : coladdrNets)
                muxarrayNets.emplace_back(coladdrNet);

            for (int mux = 0; mux < this->_wordWidth; ++mux) 
            {
                for (int i = 0; i < this->_columnSelSize; ++i) 
                    muxarrayNets.emplace_back(blNets[mux*this->_columnSelSize+i]);
                for (int i = 0; i < this->_columnSelSize; ++i) 
                    muxarrayNets.emplace_back(brNets[mux*this->_columnSelSize+i]);
            }

            // For each mux, it has one bl and br output
            for (int muxIdx = 0; muxIdx < this->_wordWidth; ++muxIdx)
            {
                muxarrayNets.emplace_back(outBlNets[muxIdx]);
                muxarrayNets.emplace_back(outBrNets[muxIdx]);
            }

            muxarrayNets.emplace_back(tech->vdd());
            muxarrayNets.emplace_back(tech->gnd());
            this->_columnMuxArrayInst = 
                this->addInstance(ColumnMuxArray::TYPE, this->_columnMuxArray, muxarrayNets);
        }
        else 
        {
            for (int i = 0; i < this->_wordWidth; ++i)
            {
                outBlNets.emplace_back(DataPath::BITLINE_NAME(i));
                outBrNets.emplace_back(DataPath::BITLINE_BAR_NAME(i));
            }
        }

        // Create and Connect sensa amp array
        // bl0 bl1 ... br0 br1 ... dout0 dout1 ... sa_en vdd gnd
        std::vector<std::string_view> senseampPorts{};
        for (int i = 0; i < this->_wordWidth; ++i)
            senseampPorts.emplace_back(outBlNets[i]);
        for (int i = 0; i < this->_wordWidth; ++i)
            senseampPorts.emplace_back(outBrNets[i]);
        for (int i = 0; i < this->_wordWidth; ++i)
            senseampPorts.emplace_back(DataPath::DATA_OUTPUT_NAME(i));
        senseampPorts.emplace_back(this->SENSE_AMP_ENABLE_NAME);
        senseampPorts.emplace_back(tech->vdd());
        senseampPorts.emplace_back(tech->gnd());
        this->_senseampArrayInst = 
            this->addInstance(SenseAmplifierArray::TYPE, this->_senseampArray, senseampPorts);

        // Create and Connect write driver array
        // din0 din1 ... bl0 bl1 ... br0 br1 ... we_en vdd gnd
        std::vector<std::string_view> writedriverPorts{};
        for (int i = 0; i < this->_wordWidth; ++i)
            writedriverPorts.emplace_back(DataPath::DATA_INPUT_NAME(i));
        for (int i = 0; i < this->_wordWidth; ++i)
            writedriverPorts.emplace_back(outBlNets[i]);
        for (int i = 0; i < this->_wordWidth; ++i)
            writedriverPorts.emplace_back(outBrNets[i]);
        writedriverPorts.emplace_back(this->WRITE_DRIVER_ENABLE_NAME);
        writedriverPorts.emplace_back(tech->vdd());
        writedriverPorts.emplace_back(tech->gnd());
        this->_writedriverArrayInst = 
            this->addInstance(WriteDriverArray::TYPE, this->_writedriverArray, writedriverPorts);
    }

    void DataPath::calculateLayoutConstants()
    {
        this->_bitlineLayer = Layer::Metal2;
        this->_bitlinePitch = tech->layerPitch(this->_bitlineLayer);
        this->_bitlineWidth = tech->layerWidth(this->_bitlineLayer);
    
        this->_horiLayer = Layer::Metal1;
        this->_horiPitch = tech->layerPitch(this->_horiLayer);
        this->_horiWidth = tech->layerWidth(this->_horiLayer);

        this->_instGap = 3 * this->_bitlinePitch;

        this->_writedriverArrayInstYOff = 0._u;
        
        auto baseY = 
            this->_writedriverArrayInstYOff + 
            this->_writedriverArrayInst->circuit()->height() +
            this->_instGap;

        for (auto i : std::ranges::views::iota(0, this->_wordWidth))
            this->_outputLineYOffs.emplace_back(baseY + i * this->_horiPitch);

        /*
            G A P
            -----
            -----
            -----
            G A P
        */
        this->_senseampArrayInstYOff = this->_outputLineYOffs.back() + this->_instGap;

        if (this->_useMux) 
        {
            this->_columnMuxArrayInstYOff = 
                this->_senseampArrayInstYOff + 
                this->_senseampArray->height() +
                this->_instGap;

            this->_height = 
                this->_columnMuxArrayInstYOff + 
                this->_columnMuxArray->height();

            this->_width = this->_columnMuxArray->width();
        }
        else 
        {
            this->_height = 
                this->_senseampArrayInstYOff + 
                this->_senseampArrayInst->circuit()->height();

            this->_width = this->_senseampArray->width();
        }
    }

    void DataPath::placeInstances()
    {
        this->_writedriverArrayInst->place(Coord{0._u, this->_writedriverArrayInstYOff});
        this->_senseampArrayInst->place(Coord{0._u, this->_senseampArrayInstYOff});
        if (this->_useMux)
            this->_columnMuxArrayInst->place(Coord{0._u, this->_columnMuxArrayInstYOff});
    }

    void DataPath::routeInstances()
    {
        // Route BL & BR
        for (auto i : std::ranges::views::iota(0, this->_wordWidth))
        {
            auto wdblPin = this->_writedriverArrayInst->getPin(WriteDriverArray::BITLINE_NAME(i));
            auto sablPin = this->_senseampArrayInst->getPin(SenseAmplifierArray::BITLINE_NAME(i));

            auto wdbrPin = this->_writedriverArrayInst->getPin(WriteDriverArray::BITLINE_BAR_NAME(i));
            auto sabrPin = this->_senseampArrayInst->getPin(SenseAmplifierArray::BITLINE_BAR_NAME(i));

            this->routePinsCenter(wdblPin, sablPin, this->_bitlineLayer);
            this->routePinsCenter(wdbrPin, sabrPin, this->_bitlineLayer);

            if (this->_useMux)
            {
                auto muxblPin = this->_columnMuxArrayInst->getPin(SenseAmplifierArray::BITLINE_NAME(i));
                auto muxbrPin = this->_columnMuxArrayInst->getPin(SenseAmplifierArray::BITLINE_BAR_NAME(i));
                this->routePinsCenter(muxblPin, sabrPin, this->_bitlineLayer);
                this->routePinsCenter(muxbrPin, sabrPin, this->_bitlineLayer);
            }
        }

        // Route output 
        for (auto i : std::ranges::views::iota(0, this->_wordWidth)) 
        {
            auto saOutputPin = this->_senseampArrayInst->getPin(SenseAmplifierArray::DATA_OUTPUT_NAME(i));
            auto corner = Coord {saOutputPin.cenX(), this->_outputLineYOffs[i]};
            auto outputCoord = Coord {this->width(), this->_outputLineYOffs[i]};
            this->routePinsByPathCenter(saOutputPin.layer(), this->_horiLayer, this->_horiLayer, std::array{
                saOutputPin.center(), corner, outputCoord
            });
            this->addPinCenter(
                this->_horiLayer, 
                DataPath::DATA_OUTPUT_NAME(i), 
                outputCoord
            );
        }
    }
 
    void DataPath::createLayoutPins()
    {
        // 1. Copy control signal
        this->copyInstancePin(this->_writedriverArrayInst, WriteDriverArray::ENABLE_NAME, DataPath::WRITE_DRIVER_ENABLE_NAME);
        this->copyInstancePin(this->_senseampArrayInst, SenseAmplifierArray::ENABLE_NAME, DataPath::SENSE_AMP_ENABLE_NAME);
    
        // 2. Copy bl/br pins or sel(if exits)
        if (this->_useMux) 
        {
            // Is data path with column mux array
            for (auto i : stdv::iota(0, this->_columnSelSize))
                this->copyInstancePin(
                    this->_columnMuxArrayInst, 
                    ColumnMuxArray::SELECTION_NAME(i), 
                    DataPath::SELECT_NAME(i)
                );

            for (auto muxIndex : stdv::iota(0, this->_wordWidth))
            {
                for (auto inputIdx : stdv::iota(0, this->_columnSelSize)) 
                {
                    auto index = muxIndex * this->_columnSelSize + inputIdx;
                    this->copyInstancePin(
                        this->_columnMuxArrayInst,
                        ColumnMuxArray::BITLINE_NAME(muxIndex, inputIdx),
                        DataPath::BITLINE_NAME(index)
                    );

                    this->copyInstancePin(
                        this->_columnMuxArrayInst,
                        ColumnMuxArray::BITLINE_BAR_NAME(muxIndex, inputIdx),
                        DataPath::BITLINE_BAR_NAME(index)
                    );
                }
            }

            // MARK!!! no vdd
            // this->copyInstancePin(this->_columnMuxArrayInst, tech->vdd(), DataPath::COLUMN_MUX_VDD_NAME);
            this->copyInstancePin(this->_columnMuxArrayInst, tech->gnd(), DataPath::COLUMN_MUX_GND_NAME);
        }
        else
        {
            // Is data path no column mux array
            // Use sense amp array's bl/br as input pins
            for (auto index : stdv::iota(0, this->_wordWidth)) 
            {
                this->copyInstancePin(
                    this->_senseampArrayInst,
                    SenseAmplifierArray::BITLINE_NAME(index),
                    DataPath::BITLINE_NAME(index)
                );
            }
        }

        // 3. Din already create in `routeInstances`
        // 4. Copy Din
        for (auto i : std::ranges::views::iota(0, this->_wordWidth))
        {
            this->copyInstancePin(
                this->_writedriverArrayInst,
                WriteDriverArray::DATA_INPUT_NAME(i),
                DataPath::DATA_INPUT_NAME(i)
            );
        }

        this->copyInstancePin(this->_senseampArrayInst, tech->vdd(), DataPath::SENSE_AMP_VDD_NAME);
        this->copyInstancePin(this->_senseampArrayInst, tech->gnd(), DataPath::SENSE_AMP_GND_NAME);
        this->copyInstancePin(this->_writedriverArrayInst, tech->vdd(), DataPath::WRITE_DRIVER_VDD_NAME);
        this->copyInstancePin(this->_writedriverArrayInst, tech->gnd(), DataPath::WRITE_DRIVER_GND_NAME);
    }

    auto DataPath::BITLINE_NAME(int index) -> std::string_view
    {
        return stringf("bl{}", index);
    }

    auto DataPath::BITLINE_BAR_NAME(int index) -> std::string_view 
    {
        return stringf("br{}", index);
    }

    auto DataPath::SELECT_NAME(int index) -> std::string_view
    {
        return stringf("sel{}", index);
    }

    auto DataPath::DATA_INPUT_NAME(int index) -> std::string_view
    {
        return stringf("din{}", index);
    }

    auto DataPath::DATA_OUTPUT_NAME(int index) -> std::string_view
    {
        return stringf("dout{}", index);
    }

}