#pragma once

namespace xtaro::circuit {

    class Circuit;
    class Module;
    class Resistance;
    class Capacitance;
    class MOS;
    class Contact;

    struct CircuitVisitor 
    {
        virtual auto visitModule(Module*) -> void = 0;
        virtual auto visitMOS(MOS*) -> void = 0;
        virtual auto visitResistance(Resistance*) -> void = 0;
        virtual auto visitCapacitance(Capacitance*) -> void = 0;
    };

}