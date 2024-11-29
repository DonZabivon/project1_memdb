#pragma once

#include <stdexcept>

#include "base.h"
#include "value.h"

namespace memdb
{

    struct Condition
    {
        Value rhs;
        RelOp op;

        Condition(const Value &rhs, RelOp op) : rhs(rhs), op(op) {}

        bool match(const Value &lhs) const
        {
            switch (op)
            {
            case RelOp::EQ:
                return lhs == rhs;
            case RelOp::NE:
                return lhs != rhs;
            case RelOp::LT:
                return lhs < rhs;
            case RelOp::GT:
                return lhs > rhs;
            case RelOp::LE:
                return lhs <= rhs;
            case RelOp::GE:
                return lhs >= rhs;
            default:
                throw std::runtime_error("Invalid operation");
            }
        }
    };

}
