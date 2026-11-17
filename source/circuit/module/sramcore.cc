#include "./sramcore.hh"

#include <circuit/module/prebitcellarray.hh>
#include <circuit/module/datapath.hh>
#include <circuit/module/controllogic.hh>

#include <tech/tech.hh>
#include <circuit/mod.hh>

#include <utility/misc/string.hh>
#include <utility/debug/debug.hh>
#include <string>

namespace xtaro::circuit
{
    std::string SRAMCore::Arguments::toString() const
    {
        return std::format("rss{}_css{}_ww{}", this->rowSelSize, this->colSelSize, this->wordWidth);
    }

    auto SRAMCore::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<SRAMCore>(circuitName, *this);
    }

    SRAMCore::SRAMCore(std::string_view name, const SRAMCore::Arguments& arguments) :
        Module{name},
        _rowSelSize{arguments.rowSelSize},
        _colSelSize{arguments.colSelSize},
        _wordWidth{arguments.wordWidth},

        _arrayColSize{arguments.colSelSize * arguments.wordWidth},
        _bitcellSize{arguments.colSelSize * arguments.rowSelSize * arguments.wordWidth},

        _perpheralBitcellArray{nullptr},
        _datapath{nullptr},
        _controllogic{nullptr}
    {
        debug::checkf(this->_rowSelSize <= SRAMCore::MAX_CORE_ROW, "Row sel size '{}' > {}", this->_rowSelSize, static_cast<int>(SRAMCore::MAX_CORE_ROW));
        debug::checkf(this->_colSelSize <= SRAMCore::MAX_CORE_COL, "Col sel size '{}' > {}", this->_colSelSize, static_cast<int>(SRAMCore::MAX_CORE_COL));
        debug::check(_bitcellSize <= SRAMCore::MAX_CORE_BIT_SIZE, "Too much bitcell size");

        this->createSelf();
    }

    void SRAMCore::createPorts()
    {
        this->addPortNet("clk", PortType::Input);

        this->addPortNet("csb", PortType::Input);
        this->addPortNet("we", PortType::Input);

        for (int i = 0; i < this->_rowSelSize; ++i)
            this->addPortNet(stringf("rsel{}", i), PortType::Input);

        if (this->hasColAddress()) 
        {
            for (int i = 0; i < this->_colSelSize; ++i)
                this->addPortNet(stringf("csel{}", i), PortType::Input);
        }

        for (int i = 0; i < this->_wordWidth; ++i)
            this->addPortNet(stringf("din{}", i), PortType::Input);

        for (int i = 0; i < this->_wordWidth; ++i)
            this->addPortNet(stringf("dout{}", i), PortType::Output);
        
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void SRAMCore::createCircuits()
    {
        PerpheralBitcellArray::Arguments precellArg {this->_rowSelSize, this->_arrayColSize};
        this->_perpheralBitcellArray = this->addCircuit(precellArg);

        DataPath::Arguments datapathArgs {this->_wordWidth, this->_colSelSize};
        this->_datapath = this->addCircuit(datapathArgs);

        ControlLogic::Arguments controlArgs {};
        this->_controllogic = this->addCircuit(controlArgs);
    } 

    void SRAMCore::createInstances() 
    {
        // Nets' name
        std::vector<std::string_view> blNets{};
        std::vector<std::string_view> brNets{};
        std::vector<std::string_view> rselNets{};
        std::vector<std::string_view> cselNets{};

        for (int i = 0; i < this->_arrayColSize; ++i)
        {
            blNets.emplace_back(stringf("bl{}", i));
            brNets.emplace_back(stringf("br{}", i));
        }
        
        for (int i = 0; i < this->_rowSelSize; ++i)
            rselNets.emplace_back(stringf("rsel{}", i));

        if (this->hasColAddress())
        {
            for (int i = 0; i < this->_colSelSize; ++i)
                cselNets.emplace_back(stringf("csel{}", i));
        }

        // PerpheralBitcellArray
        auto prebitcellarrayNets = std::vector<std::string_view>{};
        prebitcellarrayNets.emplace_back("wl_en");
        prebitcellarrayNets.emplace_back("p_en_bar");
        prebitcellarrayNets.emplace_back("we_en");
        for (auto& net : blNets)
            prebitcellarrayNets.emplace_back(net);
        for (auto& net : brNets)
            prebitcellarrayNets.emplace_back(net);
        for (auto& net : rselNets)
            prebitcellarrayNets.emplace_back(net);
        prebitcellarrayNets.emplace_back("rbl");
        prebitcellarrayNets.emplace_back(tech->vdd());
        prebitcellarrayNets.emplace_back(tech->gnd());

        this->addInstance(PerpheralBitcellArray::TYPE, this->_perpheralBitcellArray, prebitcellarrayNets);

        // Data path
        auto datapathNets = std::vector<std::string_view>{};
        datapathNets.emplace_back("sa_en");
        datapathNets.emplace_back("we_en");
        
        for (auto& net : blNets)
            datapathNets.emplace_back(net);
        for (auto& net : brNets)
            datapathNets.emplace_back(net);

        for (auto& net : cselNets)
            datapathNets.emplace_back(net);

        for (int i = 0; i < this->_wordWidth; ++i)
            datapathNets.emplace_back(stringf("din{}", i));

        for (int i = 0; i < this->_wordWidth; ++i)
            datapathNets.emplace_back(stringf("dout{}", i));

        datapathNets.emplace_back(tech->vdd());
        datapathNets.emplace_back(tech->gnd());
        this->addInstance(DataPath::TYPE, this->_datapath, datapathNets);

        // Control logic
        this->addInstance(ControlLogic::TYPE, this->_controllogic, {
            "clk", "csb", "we", "rbl", 
            "wl_en", "p_en_bar", "sa_en", "we_en", 
            tech->vdd(), tech->gnd()
        });
    }

    void SRAMCore::calculateLayoutConstants()
    {
        debug::unimplement("SRAMCore::calculateLayoutConstants");
    }

    void SRAMCore::placeInstances()
    {
        debug::unimplement("SRAMCore::placeInstances");
    }

    void SRAMCore::routeInstances()
    {
        debug::unimplement("SRAMCore::placeInstances");
    }
    
    void SRAMCore::createLayoutPins()
    {
        debug::unimplement("SRAMCore::createLayoutPins");
    }

}