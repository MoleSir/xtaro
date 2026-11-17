#pragma once

#include "./coord.hh"

namespace xtaro::circuit 
{

    struct Blockage
    {
        Coord leftdown;
        Coord rightup;

        auto transform(const Coord& offset, MirrorType mirror, double angle) const -> Blockage;
    };

}