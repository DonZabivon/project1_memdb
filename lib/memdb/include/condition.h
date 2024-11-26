#pragma once

#include <stdexcept>

#include "base.h"
#include "value.h"

namespace memdb
{

    struct Condition
    {
        Value rhs;
        CondOp op;

        Condition(const Value &rhs, CondOp op) : rhs(rhs), op(op) {}

        bool match(const Value &lhs) const
        {
            switch (op)
            {
            case CondOp::EQ:
                return lhs == rhs;
            case CondOp::NE:
                return lhs != rhs;
            case CondOp::LT:
                return lhs < rhs;
            case CondOp::GT:
                return lhs > rhs;
            case CondOp::LE:
                return lhs <= rhs;
            case CondOp::GE:
                return lhs >= rhs;
            default:
                throw std::runtime_error("Invalid operation");
            }
        }
    };

}
