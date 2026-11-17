#include "controllogic.hh"
#include "circuit/geometry/coord.hh"
#include "circuit/geometry/pin.hh"
#include "global/number/number.hh"
#include "global/tech/layer.hh"
#include "global/tech/tech.hh"

#include <array>
#include <circuit/libmodule/inv.hh>
#include <circuit/libmodule/or.hh>
#include <circuit/libmodule/nor.hh>
#include <circuit/libmodule/nand.hh>
#include <circuit/libmodule/and.hh>

#include <circuit/mod.hh>

#include <cmath>
#include <global/tech/driveability.hh>

#include <ranges>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    std::string ControlLogic::Arguments::toString() const
    {
        return "";
    }

    auto ControlLogic::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<ControlLogic>(circuitName, *this);
    }

    ControlLogic::ControlLogic(std::string_view name, const ControlLogic::Arguments& arguments) :
        Module{name}
    {
        this->createSelf();
    }

    void ControlLogic::createPorts()
    {
        this->addPortNet(CLOCK_NAME, PortType::Input);
        this->addPortNet(CHIP_SEL_BAR_NAME, PortType::Input);
        this->addPortNet(WRITE_NAME, PortType::Input);
        this->addPortNet(REPLICA_BL_NAME, PortType::Input);

        this->addPortNet(WORDLINE_ENABLE_NAME, PortType::Output);
        this->addPortNet(PRECHARGE_ENABLE_NAME, PortType::Output);
        this->addPortNet(SENSE_AMP_ENABLE_NAME, PortType::Output);
        this->addPortNet(WRITE_ENABLE_NAME, PortType::Output);

        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void ControlLogic::createCircuits()
    {
        this->_inv = this->addCircuit(INV::Arguments {DriveAbility::X1});
        this->_powerInv = this->addCircuit(INV::Arguments {DriveAbility::X2});
        this->_or2 = this->addCircuit(OR::Arguments {DriveAbility::X1, 2});
        this->_and2 = this->addCircuit(AND::Arguments {DriveAbility::X1, 2});
        this->_and3 = this->addCircuit(AND::Arguments {DriveAbility::X1, 3});
        this->_nand3 = this->addCircuit(NAND::Arguments {DriveAbility::X1, 3});
    }

    void ControlLogic::createInstances() 
    {    
        this->_clkInvInst = this->addInstance("clk_inv", this->_inv, {
            CLOCK_NAME, "clk_bar", tech->vdd(), tech->gnd()});
        this->_weInvInst  = this->addInstance("we_inv",  this->_inv, {
            WRITE_NAME,  "we_bar",  tech->vdd(), tech->gnd()});
        this->_csbInvInst = this->addInstance("csb_inv", this->_inv, {
            CHIP_SEL_BAR_NAME, "csb_bar", tech->vdd(), tech->gnd()});
        this->_rblInvInst = this->addInstance("rbl_inv", this->_inv, {
            REPLICA_BL_NAME, "rbl_bar", tech->vdd(), tech->gnd()});

        this->_pNand3Inst = this->addInstance("p_nand3", this->_nand3, {
            "csb_bar", "we_bar", CLOCK_NAME, PRECHARGE_ENABLE_NAME, tech->vdd(), tech->gnd()});
        
        this->_weAnd2Inst = this->addInstance("we_and2", this->_and2, {
            "csb_bar", WRITE_NAME, WRITE_ENABLE_NAME, tech->vdd(), tech->gnd()});
        
        this->_saAnd2Inst1 = this->addInstance("sa_and2_1", this->_and2, {
            "csb_bar", "we_bar", "sa_net1", tech->vdd(), tech->gnd()});
        this->_saAnd2Inst2 = this->addInstance("sa_and2_2", this->_and2, {
            "clk_bar", "rbl_bar", "sa_net2", tech->vdd(), tech->gnd()});
        this->_saAnd2Inst  = this->addInstance("sa_and2", this->_and2, {
            "sa_net1", "sa_net2", SENSE_AMP_ENABLE_NAME, tech->vdd(), tech->gnd()});

        this->_wlAnd2Inst = this->addInstance("wl_and2", this->_and2, {
            "csb_bar", WRITE_NAME, "wl_net1", tech->vdd(), tech->gnd()});    
        this->_wlAnd3Inst = this->addInstance("wl_and3", this->_and3, {
            "csb_bar", "clk_bar", REPLICA_BL_NAME, "wl_net2", tech->vdd(), tech->gnd()});    
        this->_wlOr2Inst  = this->addInstance("wl_or2", this->_or2, {
            "wl_net1", "wl_net2", WORDLINE_ENABLE_NAME, tech->vdd(), tech->gnd()});
    }

    void ControlLogic::calculateLayoutConstants()
    {
        this->_cellHeight = this->_inv->height();

        this->_busLayer = Layer::Metal2;
        this->_horLayer = Layer::Metal1;
        this->_viaLayer = Layer::Via1;
        
        this->_busWidth = tech->layerWidth(this->_busLayer);
        this->_busPitch = tech->layerPitch(this->_busLayer);

        this->_horWidth = tech->layerWidth(this->_horLayer);
        this->_horPitch = tech->layerPitch(this->_horLayer);

        // X off
        this->_invXOff = 4 * this->_busPitch + this->_busPitch;
        this->_busXOff = this->_invXOff + this->_inv->width() + this->_busPitch;
        this->_outputInstXOff = this->_busXOff + 9. * this->_busPitch;

        // Size
        this->_height = 6. * this->_cellHeight;
        this->_width = this->_outputInstXOff + (2 * this->_and2->width()).max(this->_and2->width() + this->_or2->width());

        // Pos
        for (auto i : std::ranges::views::iota(0, 8)) {
            this->_signalBusXOffs.emplace_back(this->_busXOff + i * this->_busPitch);
        }

        for (auto i : std::ranges::views::iota(0, 4)) {
            this->_inputBusXOffs.emplace_back(i * this->_busPitch);
        }
    }

    void ControlLogic::placeInstances()
    {
        // 1. Place input inv
        this->_clkInvInst->place(Coord {this->_invXOff, 0._u});
        this->_csbInvInst->place(Coord {this->_invXOff, 2. * this->_cellHeight}, MirrorType::MirrorY);
        this->_weInvInst->place(Coord {this->_invXOff, 2. * this->_cellHeight});
        this->_rblInvInst->place(Coord {this->_invXOff, 4. * this->_cellHeight}, MirrorType::MirrorY);

        // 2. Place output insts
        // 2.1 precharge
        this->_pNand3Inst->place(Coord {this->_outputInstXOff, 0._u});

        // 2.2 write 
        this->_weAnd2Inst->place(Coord {this->_outputInstXOff, 2. * this->_cellHeight}, MirrorType::MirrorY);

        // 2.3 senseamp
        this->_saAnd2Inst1->place(Coord {this->_outputInstXOff, 2. * this->_cellHeight});
        this->_saAnd2Inst->place(Coord {this->_outputInstXOff + this->_saAnd2Inst1->circuit()->width(), 2. * this->_cellHeight});
    
        this->_saAnd2Inst2->place(Coord {this->_outputInstXOff, 4. * this->_cellHeight}, MirrorType::MirrorY);

        // 2.4 wrodline
        this->_wlAnd2Inst->place(Coord {this->_outputInstXOff, 4. * this->_cellHeight});
        this->_wlOr2Inst->place(Coord {this->_outputInstXOff + this->_wlAnd2Inst->circuit()->width(), 4. * this->_cellHeight});

        this->_wlAnd3Inst->place(Coord {this->_outputInstXOff, 6. * this->_cellHeight}, MirrorType::MirrorY);

        // 3. bus
        // this->routeRowBus(this->_busLayer, this->_busWidth, this->_height, this->_busXOff, std::span<const Number>);
        this->routeColBus(this->_busLayer, this->_busWidth, *this->_height, 0._u, this->_signalBusXOffs);
        this->routeColBus(this->_busLayer, this->_busWidth, *this->_height, 0._u, this->_inputBusXOffs);
    }

    void ControlLogic::routeInstances()
    {
        this->routeInput();
        this->routeInvInput();
        this->routeInvOutput();

        this->routePrechargeEnable();
        this->routeWordlineEnable();
        this->routeSenseAmpEnable();
        this->routeWriteEnable();
        this->routeVDDGND();
    }

    void ControlLogic::createLayoutPins()
    {
        // MARK Input pin
        this->copyInstancePins(this->_pNand3Inst, NAND::OUTPUT_NAME, ControlLogic::PRECHARGE_ENABLE_NAME);
        this->copyInstancePins(this->_weAnd2Inst, AND::OUTPUT_NAME, ControlLogic::WRITE_ENABLE_NAME);
        this->copyInstancePins(this->_saAnd2Inst, AND::OUTPUT_NAME, ControlLogic::SENSE_AMP_ENABLE_NAME);
        this->copyInstancePins(this->_wlOr2Inst, OR::OUTPUT_NAME, ControlLogic::WORDLINE_ENABLE_NAME);
    
        this->copyInstancePins(this->_pNand3Inst, tech->vdd(), tech->vdd());
        this->copyInstancePins(this->_pNand3Inst, tech->gnd(), tech->gnd());

        this->copyInstancePins(this->_saAnd2Inst, tech->vdd(), tech->vdd());
        this->copyInstancePins(this->_saAnd2Inst, tech->gnd(), tech->gnd());

        this->copyInstancePins(this->_wlOr2Inst, tech->vdd(), tech->vdd());
        this->copyInstancePins(this->_wlOr2Inst, tech->gnd(), tech->gnd());

        this->copyInstancePins(this->_wlAnd3Inst, tech->gnd(), tech->gnd());
    }

    void ControlLogic::routeInput()
    {
        auto yOff = this->height() - this->_horPitch;
        this->routePinsCenter(
            Coord{this->_inputBusXOffs.at(0) + 0.5 * this->_busWidth, yOff}, this->_busLayer,    
            Coord{this->_signalBusXOffs.at(0) + 0.5 * this->_busWidth, yOff}, this->_busLayer,
            this->_horLayer, this->_horWidth
        );

        yOff -= this->_horPitch;
        this->routePinsCenter(
            Coord{this->_inputBusXOffs.at(1) + 0.5 * this->_busWidth, yOff}, this->_busLayer,    
            Coord{this->_signalBusXOffs.at(2) + 0.5 * this->_busWidth, yOff}, this->_busLayer,
            this->_horLayer, this->_horWidth
        );

        yOff = 4 * this->_cellHeight + this->_horPitch;
        this->routePinsCenter(
            Coord{this->_inputBusXOffs.at(2) + 0.5 * this->_busWidth, yOff}, this->_busLayer,    
            Coord{this->_signalBusXOffs.at(4) + 0.5 * this->_busWidth, yOff}, this->_busLayer,
            this->_horLayer, this->_horWidth
        );

        yOff += this->_horPitch;
        this->routePinsCenter(
            Coord{this->_inputBusXOffs.at(3) + 0.5 * this->_busWidth, yOff}, this->_busLayer,    
            Coord{this->_signalBusXOffs.at(6) + 0.5 * this->_busWidth, yOff}, this->_busLayer,
            this->_horLayer, this->_horWidth
        );
    }

    void ControlLogic::routeInvInput() 
    {
        auto clkInvPin = this->_clkInvInst->getPin(INV::INPUT_NAME);
        this->routePinToBus(clkInvPin, this->inputClockXOff());
        this->addLabelPinCenter(Layer::Metal1, ControlLogic::CLOCK_NAME, Coord {this->inputClockXOff(), clkInvPin.cenY()});
        
        auto csbInvPin = this->_csbInvInst->getPin(INV::INPUT_NAME);
        this->routePinToBus(csbInvPin, this->inputChipSelBarXOff());
        this->addLabelPinCenter(Layer::Metal1, ControlLogic::CHIP_SEL_BAR_NAME, Coord {this->inputChipSelBarXOff(), csbInvPin.cenY()});

        auto weInvPin = this->_weInvInst->getPin(INV::INPUT_NAME);
        this->routePinToBus(weInvPin, this->inputWriteXOff());
        this->addLabelPinCenter(Layer::Metal1, ControlLogic::WRITE_NAME, Coord {this->inputWriteXOff(), weInvPin.cenY()});

        auto rblInvPin = this->_rblInvInst->getPin(INV::INPUT_NAME);
        this->routePinToBus(rblInvPin, this->inputRBLXOff());
        this->addLabelPinCenter(Layer::Metal1, ControlLogic::REPLICA_BL_NAME, Coord {this->inputRBLXOff(), rblInvPin.cenY()});
    }

    void ControlLogic::routeInvOutput()
    {
        auto clkInvPin = this->_clkInvInst->getPin(INV::OUTPUT_NAME);
        auto corner1 = Coord {this->_invXOff + this->_inv->width(), clkInvPin.cenY()};
        auto corner2 = Coord {this->_invXOff + this->_inv->width(), this->_cellHeight - 2 * this->_horPitch};
        auto clkInputPos = Coord {this->clockBarXOff(), this->_cellHeight - 2 * this->_horPitch};
        this->routePathCenter(this->_horLayer, std::array{clkInvPin.center(), corner1, corner2, clkInputPos});
        this->addViaCenter(this->_viaLayer, clkInputPos);

        auto csbInvPin = this->_csbInvInst->getPin(INV::OUTPUT_NAME);
        corner1 = Coord {this->_invXOff + this->_inv->width(), csbInvPin.cenY()};
        corner2 = Coord {this->_invXOff + this->_inv->width(), 2 * this->_cellHeight - 2 * this->_horPitch};
        auto csbInputPos = Coord {this->chipSelXOff(), 2 * this->_cellHeight - 2 * this->_horPitch};
        this->routePathCenter(this->_horLayer, std::array{csbInvPin.center(), corner1, corner2, csbInputPos});
        this->addViaCenter(this->_viaLayer, csbInputPos);
    
        auto weInvPin = this->_weInvInst->getPin(INV::OUTPUT_NAME);
        corner1 = Coord {this->_invXOff + this->_inv->width(), weInvPin.cenY()};
        corner2 = Coord {this->_invXOff + this->_inv->width(), 3 * this->_cellHeight - 2 * this->_horPitch};
        auto weInputPos = Coord {this->writeBarXOff(), 3 * this->_cellHeight - 2 * this->_horPitch};
        this->routePathCenter(this->_horLayer, std::array{weInvPin.center(), corner1, corner2, weInputPos});
        this->addViaCenter(this->_viaLayer, weInputPos);

        auto rblInvPin = this->_rblInvInst->getPin(INV::OUTPUT_NAME);
        corner1 = Coord {this->_invXOff + this->_inv->width(), rblInvPin.cenY()};
        corner2 = Coord {this->_invXOff + this->_inv->width(), 4 * this->_cellHeight - 2 * this->_horPitch};
        auto rblInputPos = Coord {this->rblBarXOff(), 4 * this->_cellHeight - 2 * this->_horPitch};
        this->routePathCenter(this->_horLayer, std::array{rblInvPin.center(), corner1, corner2, rblInputPos});
        this->addViaCenter(this->_viaLayer, rblInputPos);
    }

    void ControlLogic::routeWordlineEnable()
    {
        /*
            assign wl_net1 = csb_bar & we;
            assign wl_net2 = csb_bar & clk_bar & rbl;
            assign wl_en = wl_net1 | wl_net2;
        */
        this->routePinToBus(
            this->_wlAnd2Inst->getPin(AND::INPUT_NAME(0)), this->chipSelXOff());
        this->routePinToBus(
            this->_wlAnd2Inst->getPin(AND::INPUT_NAME(1)), this->writeXOff());

        this->routePinToBus(
            this->_wlAnd3Inst->getPin(AND::INPUT_NAME(0)), this->writeBarXOff());
        this->routePinToBus(
            this->_wlAnd3Inst->getPin(AND::INPUT_NAME(1)), this->clockBarXOff());
        this->routePinToBus(
            this->_wlAnd3Inst->getPin(AND::INPUT_NAME(2)), this->rblXOff());

        auto wlNet1Pin = this->_wlAnd2Inst->getPin(AND::OUTPUT_NAME);
        auto wlNet2Pin = this->_wlAnd3Inst->getPin(AND::OUTPUT_NAME);

        auto input1Pin = this->_wlOr2Inst->getPin(OR::INPUT_NAME(0));
        auto input2Pin = this->_wlOr2Inst->getPin(OR::INPUT_NAME(1));

        auto corner = Coord {input2Pin.cenX(), wlNet2Pin.cenY()};

        // this->routePathCenter(this->_horLayer, std::array{wlNet1Pin.center(), input1Pin.center()});
        // this->routePathCenter(this->_busLayer, std::array{wlNet2Pin.center(), corner, input2Pin.center()});
        this->routePinsByPathCenter(
            wlNet1Pin.layer(), input1Pin.layer(),
            this->_horLayer, std::array{wlNet1Pin.center(), input1Pin.center()});
        this->routePinsByPathCenter(
            wlNet2Pin.layer(), input2Pin.layer(),
            this->_busLayer, std::array{wlNet2Pin.center(), corner, input2Pin.center()});
    }

    void ControlLogic::routePrechargeEnable()
    {
        // assign p_en_bar = ~(csb_bar & we_bar & clk); 
        this->routePinToBus(
            this->_pNand3Inst->getPin(NAND::INPUT_NAME(0)), this->chipSelXOff());
        
        this->routePinToBus(
            this->_pNand3Inst->getPin(NAND::INPUT_NAME(1)), this->writeBarXOff());

        this->routePinToBus(
            this->_pNand3Inst->getPin(NAND::INPUT_NAME(2)), this->clockXOff());
    }

    void ControlLogic::routeSenseAmpEnable()
    {
        /*
            assign sa_net1 = csb_bar & we_bar;
            assign sa_net2 = rbl_bar & clk_bar;
            assign sa_en = sa_net1 & sa_net2;
        */
        this->routePinToBus(
            this->_saAnd2Inst1->getPin(AND::INPUT_NAME(0)), this->chipSelXOff());
        this->routePinToBus(
            this->_saAnd2Inst1->getPin(AND::INPUT_NAME(1)), this->writeBarXOff());

        this->routePinToBus(
            this->_saAnd2Inst2->getPin(AND::INPUT_NAME(0)), this->rblBarXOff());
        this->routePinToBus(
            this->_saAnd2Inst2->getPin(AND::INPUT_NAME(1)), this->clockBarXOff());

        auto saNet1Pin = this->_saAnd2Inst1->getPin(AND::OUTPUT_NAME);
        auto saNet2Pin = this->_saAnd2Inst2->getPin(AND::OUTPUT_NAME);

        auto input1Pin = this->_saAnd2Inst->getPin(AND::INPUT_NAME(0));
        auto input2Pin = this->_saAnd2Inst->getPin(AND::INPUT_NAME(1));

        auto corner = Coord {input2Pin.cenX(), saNet2Pin.cenY()};

        // this->routePathCenter(this->_horLayer, std::array{saNet1Pin.center(), input1Pin.center()});
        // this->routePathCenter(this->_busLayer, std::array{saNet2Pin.center(), corner, input2Pin.center()});
        this->routePinsByPathCenter(
            saNet1Pin.layer(), input1Pin.layer(),
            this->_horLayer, std::array{saNet1Pin.center(), input1Pin.center()});
        this->routePinsByPathCenter(
            saNet2Pin.layer(), input2Pin.layer(),
            this->_busLayer, std::array{saNet2Pin.center(), corner, input2Pin.center()});
    }

    void ControlLogic::routeWriteEnable()
    {
        // csb_bar & we
        this->routePinToBus(
            this->_weAnd2Inst->getPin(AND::INPUT_NAME(0)), this->chipSelXOff());
        
        this->routePinToBus(
            this->_weAnd2Inst->getPin(AND::INPUT_NAME(1)), this->chipSelXOff());
    }

    void ControlLogic::routeVDDGND() 
    {
        auto routeSource = [this] (const Number& y) {
            this->addRectangleSegmentCenter(
                this->_horLayer,
                Coord { 0._u, y },
                Coord { this->width() , y }
            );
        };

        auto minY = this->_pNand3Inst->getPin(tech->vdd()).cenY();
        auto digitHeight = this->_inv->height();

        for (auto i : std::ranges::views::iota(0, 6)
                         | std::ranges::views::filter([](auto i){ return i % 2 == 0; }))
        {
            routeSource(i * minY);
            routeSource(minY + i * digitHeight);
        }
        routeSource(minY + 5 * digitHeight);
    }

    void ControlLogic::routePinToBus(const Pin& pin, const Number& xOff)
    {
        this->routePinsCenter(
            Coord {xOff, pin.cenY()}, this->_busLayer, 
            pin.center(), pin.layer(),
            this->_horLayer, this->_horWidth
        );   
    }

    auto ControlLogic::inputClockXOff() -> Number 
    { return this->_inputBusXOffs.at(ControlLogic::CLK_INDEX) + 0.5 * this->_busWidth; }

    auto ControlLogic::inputChipSelBarXOff() -> Number
    { return this->_inputBusXOffs.at(ControlLogic::CSB_INDEX) + 0.5 * this->_busWidth; }

    auto ControlLogic::inputWriteXOff() -> Number
    { return this->_inputBusXOffs.at(ControlLogic::WE_INDEX) + 0.5 * this->_busWidth; }

    auto ControlLogic::inputRBLXOff() -> Number
    { return this->_inputBusXOffs.at(ControlLogic::RBL_INDEX) + 0.5 * this->_busWidth; }
    
    auto ControlLogic::clockXOff() -> Number 
    { return this->_signalBusXOffs.at(2 * ControlLogic::CLK_INDEX) + 0.5 * this->_busWidth; }

    auto ControlLogic::chipSelBarXOff() -> Number
    { return this->_signalBusXOffs.at(2 * ControlLogic::CSB_INDEX) + 0.5 * this->_busWidth; }

    auto ControlLogic::writeXOff() -> Number
    { return this->_signalBusXOffs.at(2 * ControlLogic::WE_INDEX) + 0.5 * this->_busWidth; }

    auto ControlLogic::rblXOff() -> Number
    { return this->_signalBusXOffs.at(2 * ControlLogic::RBL_INDEX) + 0.5 * this->_busWidth; }

    auto ControlLogic::clockBarXOff() -> Number 
    { return this->_signalBusXOffs.at(2 * ControlLogic::CLK_INDEX + 1) + 0.5 * this->_busWidth; }

    auto ControlLogic::chipSelXOff() -> Number
    { return this->_signalBusXOffs.at(2 * ControlLogic::CSB_INDEX + 1) + 0.5 * this->_busWidth; }

    auto ControlLogic::writeBarXOff() -> Number 
    { return this->_signalBusXOffs.at(2 * ControlLogic::WE_INDEX + 1) + 0.5 * this->_busWidth; }

    auto ControlLogic::rblBarXOff() -> Number 
    { return this->_signalBusXOffs.at(2 * ControlLogic::RBL_INDEX + 1) + 0.5 * this->_busWidth; }

}