/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2015, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file BinaryConstraintOps.h
 *
 * Defines binary constraint operators for AST & RAM
 *
 ***********************************************************************/

#pragma once

#include "RamTypes.h"
#include <cassert>
#include <iostream>

namespace souffle {

/**
 * Binary Constraint Operators
 */

// TODO (darth_tytus): Some of the constraints are repeated because of float and unsigned.
// This is inelegant solution, but Ram execution demands this distinction.
// Investigate a better way.

enum class BinaryConstraintOp {
    __UNDEFINED__,  // undefined operator
    EQ,             // equivalence of two values
    NE,             // whether two values are different
    LT,             // signed <
    ULT,            // Unsigned <
    FLT,            // Float <
    LE,             // signed ≤
    ULE,            // Unsigned ≤
    FLE,            // Float ≤
    GT,             // signed >
    UGT,            // unsigned >
    FGT,            // float >
    GE,             // signed ≥
    UGE,            // Unsigned ≥
    FGE,            // Float ≥
    MATCH,          // matching string
    CONTAINS,       // whether a sub-string is contained in a string
    NOT_MATCH,      // not matching string
    NOT_CONTAINS    // whether a sub-string is not contained in a string
};

/**
 * Utility function, informing whether constraint is overloaded.
 * Only the signed version's are treated as overloaded (as they are returned by the parser).
 */
inline bool isOverloaded(const BinaryConstraintOp constraintOp) {
    switch (constraintOp) {
        case BinaryConstraintOp::LT:
        case BinaryConstraintOp::LE:
        case BinaryConstraintOp::GT:
        case BinaryConstraintOp::GE:
            return true;
        default:
            break;
    }
    return false;
}

/**
 * Convert Constraint to work with requested type.
 * Example: constraintOp = LT, toType = Float -> FLT (less-than working on floats).
 */
inline BinaryConstraintOp convertOverloadedConstraint(
        const BinaryConstraintOp constraintOp, const RamTypeAttribute toType) {
    switch (constraintOp) {
        case BinaryConstraintOp::LT:
            if (toType == RamTypeAttribute::Unsigned) {
                return BinaryConstraintOp::ULT;
            }
            if (toType == RamTypeAttribute::Float) {
                return BinaryConstraintOp::FLT;
            }
            break;
        case BinaryConstraintOp::LE:
            if (toType == RamTypeAttribute::Unsigned) {
                return BinaryConstraintOp::ULE;
            }
            if (toType == RamTypeAttribute::Float) {
                return BinaryConstraintOp::FLE;
            }
            break;
        case BinaryConstraintOp::GT:
            if (toType == RamTypeAttribute::Unsigned) {
                return BinaryConstraintOp::UGT;
            }
            if (toType == RamTypeAttribute::Float) {
                return BinaryConstraintOp::FGT;
            }
            break;
        case BinaryConstraintOp::GE:
            if (toType == RamTypeAttribute::Unsigned) {
                return BinaryConstraintOp::UGE;
            }
            if (toType == RamTypeAttribute::Float) {
                return BinaryConstraintOp::FGE;
            }
            break;
        default:
            break;
    }
    assert(false && "Invalid constraint conversion");
    return BinaryConstraintOp::__UNDEFINED__;
}

/**
 * Negated Constraint Operator
 * Each operator requires a negated operator which is
 * necessary for the expansion of complex rule bodies with disjunction and negation.
 */
inline BinaryConstraintOp negatedConstraintOp(const BinaryConstraintOp op) {
    switch (op) {
        case BinaryConstraintOp::EQ:
            return BinaryConstraintOp::NE;
        case BinaryConstraintOp::NE:
            return BinaryConstraintOp::EQ;

        case BinaryConstraintOp::LT:
            return BinaryConstraintOp::GE;
        case BinaryConstraintOp::ULT:
            return BinaryConstraintOp::UGE;
        case BinaryConstraintOp::FLT:
            return BinaryConstraintOp::FGE;

        case BinaryConstraintOp::LE:
            return BinaryConstraintOp::GT;
        case BinaryConstraintOp::ULE:
            return BinaryConstraintOp::UGT;
        case BinaryConstraintOp::FLE:
            return BinaryConstraintOp::FGT;

        case BinaryConstraintOp::GE:
            return BinaryConstraintOp::LT;
        case BinaryConstraintOp::UGE:
            return BinaryConstraintOp::ULT;
        case BinaryConstraintOp::FGE:
            return BinaryConstraintOp::FLT;

        case BinaryConstraintOp::GT:
            return BinaryConstraintOp::LE;
        case BinaryConstraintOp::UGT:
            return BinaryConstraintOp::ULE;
        case BinaryConstraintOp::FGT:
            return BinaryConstraintOp::FLE;

        case BinaryConstraintOp::MATCH:
            return BinaryConstraintOp::NOT_MATCH;
        case BinaryConstraintOp::NOT_MATCH:
            return BinaryConstraintOp::MATCH;
        case BinaryConstraintOp::CONTAINS:
            return BinaryConstraintOp::NOT_CONTAINS;
        case BinaryConstraintOp::NOT_CONTAINS:
            return BinaryConstraintOp::CONTAINS;

        case BinaryConstraintOp::__UNDEFINED__:
            break;
    }
    assert(false && "Unsupported Operator!");
    return op;
}

/**
 * Converts operator to its symbolic representation
 */
inline std::string toBinaryConstraintSymbol(const BinaryConstraintOp op) {
    switch (op) {
        case BinaryConstraintOp::EQ:
            return "=";
        case BinaryConstraintOp::NE:
            return "!=";
        case BinaryConstraintOp::ULT:
        case BinaryConstraintOp::FLT:
        case BinaryConstraintOp::LT:
            return "<";
        case BinaryConstraintOp::ULE:
        case BinaryConstraintOp::FLE:
        case BinaryConstraintOp::LE:
            return "<=";
        case BinaryConstraintOp::UGT:
        case BinaryConstraintOp::FGT:
        case BinaryConstraintOp::GT:
            return ">";
        case BinaryConstraintOp::UGE:
        case BinaryConstraintOp::FGE:
        case BinaryConstraintOp::GE:
            return ">=";
        case BinaryConstraintOp::MATCH:
            return "match";
        case BinaryConstraintOp::CONTAINS:
            return "contains";
        case BinaryConstraintOp::NOT_MATCH:
            return "not_match";
        case BinaryConstraintOp::NOT_CONTAINS:
            return "not_contains";
        case BinaryConstraintOp::__UNDEFINED__:
            break;
    }
    assert(false && "Unsupported Operator!");
    return "?";
}

/**
 * Converts symbolic representation of an operator to the operator
 */
inline BinaryConstraintOp toBinaryConstraintOp(const std::string& symbol) {
    if (symbol == "=") return BinaryConstraintOp::EQ;
    if (symbol == "!=") return BinaryConstraintOp::NE;
    if (symbol == "<") return BinaryConstraintOp::LT;
    if (symbol == "<=") return BinaryConstraintOp::LE;
    if (symbol == ">=") return BinaryConstraintOp::GE;
    if (symbol == ">") return BinaryConstraintOp::GT;
    if (symbol == "match") return BinaryConstraintOp::MATCH;
    if (symbol == "contains") return BinaryConstraintOp::CONTAINS;
    if (symbol == "not_match") return BinaryConstraintOp::NOT_MATCH;
    if (symbol == "not_contains") return BinaryConstraintOp::NOT_CONTAINS;
    std::cout << "Unrecognised operator: " << symbol << "\n";
    assert(false && "Unsupported Operator!");
    return BinaryConstraintOp::__UNDEFINED__;
}

/**
 * Determines whether arguments of constraint are numeric
 */
inline bool isNumericBinaryConstraintOp(const BinaryConstraintOp op) {
    switch (op) {
        case BinaryConstraintOp::EQ:
        case BinaryConstraintOp::NE:
        case BinaryConstraintOp::LT:
        case BinaryConstraintOp::ULT:
        case BinaryConstraintOp::FLT:
        case BinaryConstraintOp::LE:
        case BinaryConstraintOp::ULE:
        case BinaryConstraintOp::FLE:
        case BinaryConstraintOp::GE:
        case BinaryConstraintOp::UGE:
        case BinaryConstraintOp::FGE:
        case BinaryConstraintOp::GT:
        case BinaryConstraintOp::UGT:
        case BinaryConstraintOp::FGT:
            return true;

        case BinaryConstraintOp::MATCH:
        case BinaryConstraintOp::NOT_MATCH:
        case BinaryConstraintOp::CONTAINS:
        case BinaryConstraintOp::NOT_CONTAINS:
            return false;

        case BinaryConstraintOp::__UNDEFINED__:
            break;
    }
    assert(false && "Uncovered case!");
    return false;
}

/**
 * Determines whether arguments of constraint are numeric
 */
inline bool isSymbolicBinaryConstraintOp(const BinaryConstraintOp op) {
    return !isNumericBinaryConstraintOp(op);
}

}  // end of namespace souffle
