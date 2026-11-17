#include "fanoutbuff.hh"
#include "global/config/config.hh"
#include <tech/tech.hh>

#include <circuit/libmodule/inv.hh>

#include <circuit/mod.hh>

#include <utility/misc/string.hh>
#include <utility/misc/math.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    std::string FanoutBuffer::Arguments::toString() const
    {
        return std::format("fs{}", this->fanoutSize);
    }

    auto FanoutBuffer::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<FanoutBuffer>(circuitName, *this);
    }

    auto FanoutBuffer::calculateFanoutInfo(int outputSize) -> FanoutInfo
    {
        constexpr double SUIT_RATIO {0.6};
        if (outputSize <= MAX_FANOUT_SIZE) return FanoutInfo {1, outputSize, 0};
        int invSize {2};
        while (true)
        {
            bool remainder = outputSize % invSize == 0;
            int fanout = outputSize / invSize + (remainder ? 0 : 1);
            if (fanout > FanoutBuffer::MAX_FANOUT_SIZE)
            {
                invSize += 1;
                continue;
            }

            int delta = fanout * invSize - outputSize;
            return FanoutInfo {invSize, fanout, delta};
        }
        return FanoutInfo {-1, -1, -1};
    }

    FanoutBuffer::FanoutBuffer(std::string_view name, const FanoutBuffer::Arguments& arguments) :
        Module{name},
        _fanoutSize{arguments.fanoutSize},
        _treeLevelFanoutInfos{},
        _inv{nullptr}
    {
        debug::checkf(this->_fanoutSize > 1, "Fanout width '{}' <= 1", this->_fanoutSize);

        auto outputSize = this->_fanoutSize;
        while (true)
        {
            auto [invSize, fanout, delta] = FanoutBuffer::calculateFanoutInfo(outputSize);
            outputSize = invSize;
            this->_treeLevelFanoutInfos.emplace_front(invSize, fanout, delta);
            if (invSize == 1) break;
        }

        this->createSelf();
        if (config->buildBackend) {
            debug::warning("FanoutBuffer not support layout now...");
        }
    }

    void FanoutBuffer::createPorts()
    {
        this->addPortNet("in", PortType::Input);
        for (int outputNetIdx = 0; outputNetIdx < this->_fanoutSize; ++outputNetIdx)
            this->addPortNet(stringf("out{}", outputNetIdx), PortType::Output);
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void FanoutBuffer::createCircuits()
    {   
        this->_inv = this->addCircuit(INV::Arguments {DriveAbility::X2});
    } 

    void FanoutBuffer::createInstances() 
    {
        // The tree always full
        int treeDepth = this->_treeLevelFanoutInfos.size();
        for (int level = 0; level < treeDepth ; ++level)
        {
            int invSize {this->treeLevelInvSize(level)};
            int fanout {this->treeLevelFanout(level)};

            // Now, this level has 'inputSize' input, each of input will generate 'fanout' output
            for (int invIdx = 0; invIdx < invSize; ++invIdx)
            {
                std::string_view inputNet {this->treeLevelInvInputName(level, invIdx)};
                std::string_view outputNet {this->treeLevelInvOutputName(level, invIdx)};
                this->addInstance(
                    this->treeLevelInvName(level, invIdx),
                    this->_inv,
                    {inputNet,  outputNet , tech->vdd(), tech->gnd()}
                );
            } 
        }

        // Add inv to balance logic
        if (treeDepth % 2 != 0)
            this->addInstance("inv", this->_inv, {"in", this->treeLevelInvInputName(0, 0), tech->vdd(), tech->gnd()});
        else
            this->connectNets("in", this->treeLevelInvInputName(0, 0));

        // Link outputs
        for (int outputNetIdx = 0; outputNetIdx < this->_fanoutSize; ++outputNetIdx)
            this->connectNets(
                this->treeLevelInvInputName(treeDepth, outputNetIdx), stringf("out{}", outputNetIdx)
            );
    }

    auto FanoutBuffer::treeLevelInvSize(int level) const -> int
    {
        auto [i, f, d] {this->_treeLevelFanoutInfos[level]};
        return i;
    }

    auto FanoutBuffer::treeLevelFanout(int level) const -> int
    {
        auto [i, f, d] {this->_treeLevelFanoutInfos[level]};
        return f;
    }

    auto FanoutBuffer::treeLevelInvName(int level, int invIdx) const -> std::string_view
    {
        int fanout {this->treeLevelFanout(level)};
        return stringf("inv_{}_{}", level, invIdx);
    }

    auto FanoutBuffer::treeLevelInvInputName(int level, int invIdx) const -> std::string_view
    {
        if (level == 0) return "net_begin";
        int fanout {this->treeLevelFanout(level - 1)};
        return this->treeLevelInvOutputName(level - 1, invIdx / fanout);
    }

    auto FanoutBuffer::treeLevelInvOutputName(int level, int invIdx) const -> std::string_view
    {
        int fanout {this->treeLevelFanout(level)};
        return stringf("net_{}_{}", level, invIdx);
    }

    void FanoutBuffer::calculateLayoutConstants()
    {
        // debug::unimplement("FanoutBuffer::calculateLayoutConstants");
    }

    void FanoutBuffer::placeInstances()
    {
        // debug::unimplement("FanoutBuffer::placeInstances");
    }

    void FanoutBuffer::routeInstances()
    {
        // debug::unimplement("FanoutBuffer::placeInstances");
    }

    void FanoutBuffer::createLayoutPins()
    {
        // debug::unimplement("FanoutBuffer::createLayoutPins");
    }

}