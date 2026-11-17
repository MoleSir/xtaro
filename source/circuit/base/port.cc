#include "port.hh"

namespace xtaro::circuit
{

    Port::Port(std::string_view name, PortType type):
        _name{name},
        _type{type},
        _net{nullptr}
    {

    }

}