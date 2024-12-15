#pragma once

#include <stdexcept>

#include "base.h"
#include "value.h"

namespace memdb
{

    struct Condition
    {
        Value that;
        RelOp op;

        Condition(const Value &rhs, RelOp op) : that(rhs), op(op) {}

        bool match(const Value &lhs) const
        {
            switch (op)
            {
            case RelOp::EQ:
                return lhs == that;
            case RelOp::NE:
                return lhs != that;
            case RelOp::LT:
                return lhs < that;
            case RelOp::GT:
                return lhs > that;
            case RelOp::LE:
                return lhs <= that;
            case RelOp::GE:
                return lhs >= that;
            default:
                throw std::runtime_error("Invalid operation");
            }
        }
    };

}
