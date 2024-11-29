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

    enum class RelOp
    {
        EQ,
        NE,
        LT,
        GT,
        LE,
        GE
    };

    enum class Op
    {
        // Arithmetic
        PLS,
        MNS,
        MUL,
        DIV,
        MOD,
        // Relation
        EQ,
        NE,
        LT,
        GT,
        LE,
        GE,
        // Logic
        AND,
        OR,
        XOR,
        NOT
    };

}
