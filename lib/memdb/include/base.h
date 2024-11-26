#pragma once

#include <cstdint>

namespace memdb
{

    enum class Type
    {
        INT,
        BOOL,
        STRING,
        BYTES,
        NONE
    };

    enum class LogicOp
    {
        EQ,
        NEQ,
        AND,
        OR,
        XOR
    };

    enum class CondOp
    {
        EQ,
        NE,
        LT,
        GT,
        LE,
        GE
    };

}
