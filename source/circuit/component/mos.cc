#include "./mos.hh"
#include "../base/circuit.hh"
#include "../base/visitor.hh"

#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <utility/misc/string.hh>
#include <utility/debug/debug.hh>
#include <circuit/mod.hh>

namespace xtaro::circuit
{
    std::string MOS::Arguments::toString() const
    {
        return std::format("t{}_w{:.2f}_l{:.2f}", 
                            static_cast<int>(this->type), 
                            this->width, 
                            this->length);
    }

    auto MOS::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<MOS>(circuitName, *this);
    }

    MOS::MOS(std::string_view name, const MOS::Arguments& arguments):
        Circuit{name},
        _mosType{arguments.type},
        _mosWidth{arguments.width},
        _mosLength{arguments.length}
    {
        // // TODO: Check arguments
        // if (this->_mosType == MOSType::NMOS)
        //     this->_name = factory::string(std::string{tech->nmosName()});
        // else
        //     this->_name = factory::string(std::string{tech->pmosName()});

        this->createPorts();

        // this->setupLayoutConstants();
        // this->addActive();
        // this->addWellImplant();
        // this->addPoly();
        // this->addActiveContacts();
    }

    void MOS::createPorts()
    {
        this->addPort("S", PortType::InOut);
        this->addPort("G", PortType::Input);
        this->addPort("D", PortType::InOut);
        this->addPort("B", PortType::InOut);
    }

    auto MOS::createNetlist() -> void
    {

    }

    auto MOS::createLayout()  -> void
    {
        debug::unimplement("`createLayout` for MOS");
    }

    // auto MOS::setupLayoutConstants() -> void
    // {
    //     this->_contactCount = this->calculateContactCount();
    //     this->_activeContact = &this->addContact(tech->layer.activeStack, {1, this->_contactCount});

    //     this->_minPolyPitch = tech->layerWidth("poly") + tech->layerSpacing("poly");
    //     // this->_contactedPolyPitch = this->_minPolyPitch;
    //     this->_contactPitch = 2 * tech->layerSpacing("poly", "contact") + this->_activeContact->width();

    //     this->_polyPitch = this->_minPolyPitch.max(this->_contactPitch);
    //     this->_endToContact = 0.5 * this->_activeContact->width();

    //     this->_activeWidth = 2.0 * this->_endToContact + this->_activeContact->width()
    //                        + 2 * tech->layerSpacing("poly", "contact") 
    //                        + tech->layerWidth("poly") 
    //                        + (this->_mults - 1) * this->_polyPitch; 

    //     this->_activeHeight = this->_width;
    //     this->_polyHeight = this->_activeHeight + 2.0 * tech->drc.extent("poly", "active");
    //     // MARK
    //     this->_activeOffset = Coord {tech->drc.enclose("nwell", "active"), tech->drc.enclose("nwell", "active") };

    //     auto wellName = this->_mosType == MOSType::NMOS ? "nwell" : "pwell";
    //     auto wellWidth = tech->layerWidth(wellName);
    //     this->_wellWidth = wellWidth.max(this->_activeWidth + 2.0 * tech->drc.enclose(wellName, "active"));
    //     this->_wellHeight = wellWidth.max(this->_activeHeight + 2.0 * tech->drc.enclose(wellName, "active"));
    
    //     // TODO 

    //     this->_contactOffset = this->_activeOffset + Coord {
    //         0.5 * this->_activeContact->width(),
    //         0.5 * this->_activeHeight
    //     };
    // }

    // auto MOS::addActive() -> void
    // {
    //     this->_active = &this->addRectangle("active", this->_activeOffset, this->_activeWidth, this->_activeHeight);

    //     auto implantName = this->_mosType == MOSType::NMOS ? "nimplant" : "pimplant";
    //     auto enclose = tech->drc.enclose(implantName, "active");
    //     this->_implant = &this->addRectangle(
    //         implantName,
    //         this->_activeOffset - Coord {enclose, enclose},
    //         this->_activeWidth + 2 * enclose,
    //         this->_activeHeight + 2 * enclose    
    //     );
    // }

    // auto MOS::addWellImplant() -> void
    // {
    //     auto wellName = this->_mosType == MOSType::NMOS ? "nwell" : "pwell";

    //     auto centerCoord = this->_activeOffset + Coord {0.5 * this->_activeWidth, 0.5 * this->_activeHeight};
    //     auto wellOffset = centerCoord - Coord {
    //         0.5 * this->_wellWidth,
    //         0.5 * this->_wellHeight  
    //     };
        
    //     this->_well = &this->addRectangle(
    //         wellName,
    //         wellOffset,
    //         this->_wellWidth,
    //         this->_wellHeight
    //     );
    // }

    // auto MOS::addPoly() -> void
    // {
    //     auto polyOffset = this->_contactOffset;
    //     polyOffset.x += 0.5 * this->_activeContact->width() + 0.5 * tech->layerWidth("poly") + tech->layerSpacing("poly", "active");

    //     for (int i = 0; i < this->_mults; ++i) 
    //     {
    //         this->addRectangle(
    //             "poly",
    //             polyOffset,
    //             this->_polyHeight,
    //             tech->layerWidth("poly")
    //         );

    //         this->addPinCenter("poly", "G", polyOffset, tech->layerWidth("poly"), this->_polyHeight);

    //         polyOffset.x += this->_polyPitch;
    //     }
    // }

    // auto MOS::addActiveContacts() -> void
    // {
    //     // auto pos = this->_contactOffset;
    // }
    
    auto MOS::accept(CircuitVisitor& visitor) -> void
    {
        return visitor.visitMOS(this);
    }

    // auto MOS::calculateContactCount() -> int
    // {
    //     return 1;
    // }

}