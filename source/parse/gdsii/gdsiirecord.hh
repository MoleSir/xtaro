#pragma once

#include <cstdint>

namespace xtaro::parse
{

    enum class GDSIIRecordType : std::uint16_t
    {
        HEADER = 0x0002,
        BGNLIB = 0x0102,
        LIBNAME = 0x0206,
        REFLIBS = 0x1F06,
        FONTS = 0x2006,
        ATTRTABLE = 0x2306,
        GENERATIONS = 0x2202,
        FORMAT = 0x3602,
        MASK = 0x3706,
        ENDMASKS = 0x3800,
        UINTS = 0x0305,

        ENDLIB = 0x0400,

        BGNSTR = 0x0502,
        STRNAME = 0x0606,

        ENDSTR = 0x0700,
        ENDELE = 0x1100,

        BOUNDARY = 0x0800,
        PATH = 0x0900,
        SREF = 0x0A00,
        AREF = 0x0B00,
        TEXT = 0x0C00,
        NODE = 0x1500,
        BOX = 0x2D00,

        ELFLAGS = 0x2601,
        PLEX = 0x2F03,
        LAYER = 0x0D02,
        DATATYPE = 0x0E02,
        XY = 0x1003,
        PATHTYPE = 0x2102,
        WIDTH = 0x0F03,
        SNAME = 0x1206,
        STRANS = 0x1A01,
        MAG = 0x1B05,
        ANGLE = 0x1C05,
        COLROW = 0x1302,
        TEXTTYPE = 0x1602,
        PRESENTATION = 0x1701,
        STRING = 0x1906,
        NODETYPE = 0x2A02,
        BOXTYPE = 0x2E02,

        BGNEXTN = 0x3003,
        ENDEXTN = 0x3103,
    };

}