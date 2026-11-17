#pragma once

#include "../base/circuit.hh"
#include "circuit/geometry/rectangle.hh"
#include <global/number/number.hh>
#include <string>
#include <string_view>

namespace xtaro::circuit
{

    enum class MOSType
    {
        NMOS, PMOS,
    };

    /*
        - Ports: d/s g s/d b
    */
    class MOS : public Circuit
    {
    public:
        static constexpr std::string_view TYPE = "mos";

        /*
            - Construction: (MOSType type, Number width, Number length);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(MOSType t, Number w, Number l) :
                Circuit::Arguments {MOS::TYPE},
                type{t}, width{w}, length{l} {}
                
            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            MOSType type;
            Number width;
            Number length;
        };

    public:
        MOS(std::string_view name, const MOS::Arguments& arguments);
        virtual ~MOS() noexcept override = default;

    public:
        virtual auto accept(CircuitVisitor& visitor) -> void override;

    private:
        virtual auto createPorts() -> void override;
        virtual auto createNetlist() -> void override;
        virtual auto createLayout()  -> void override;

    public:
        auto mosType() const -> MOSType { return this->_mosType; }
        auto mosWidth() const -> const Number& { return this->_mosWidth; }
        auto mosLength() const -> const Number& { return this->_mosLength; }

    private:
        auto setupLayoutConstants() -> void;
        auto addActive() -> void;
        auto addWellImplant() -> void;
        auto addPoly() -> void;
        auto addActiveContacts() -> void;

        auto calculateContactCount() -> int;

    private:
        // TODO: m=1 w=1.6u l=0.4u pd=4.00u ps=4.00u as=1.60p ad=1.60p
        // other arguments
        MOSType _mosType;
        Number  _mosWidth;
        Number  _mosLength;
        int _mults;

        int _contactCount;
        Number _minPolyPitch;
        Number _contactedPolyPitch;
        Number _contactPitch;
        Number _polyPitch;
        Number _endToContact;
        Number _activeWidth;
        Number _activeHeight;
        Number _polyHeight;
        Coord _activeOffset;
        Coord _contactOffset;

        Number _wellWidth;
        Number _wellHeight;

        Via* _activeContact;
        Rectangle* _active;
        Rectangle* _implant;
        Rectangle* _well;
    };

}

DESERIALIZE_ENUM(xtaro::circuit::MOSType, 
    DE_VALUE_AS(NMOS, "nmos")
    DE_VALUE_AS(PMOS, "pmos")
)