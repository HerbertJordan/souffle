/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2019, The Souffle Developers. All rights reserved.
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file FunctorOps.h
 *
 * Defines intrinsic functor operators for AST and RAM
 *
 ***********************************************************************/

#pragma once

#include "RamTypes.h"

#include <cassert>
#include <cstdlib>
#include <iostream>

namespace souffle {

enum class FunctorOp {
    /** Unary Functor Operators */
    ORD,       // ordinal number of a string
    STRLEN,    // length of a string
    NEG,       // Signed numeric negation
    FNEG,      // Float numeric negation
    BNOT,      // Signed bitwise negation
    UBNOT,     // Unsigned bitwise negation
    LNOT,      // Signed logical negation
    ULNOT,     // Unsigned logical negation
    TONUMBER,  // convert string to number
    TOSTRING,  // convert number to string
    ITOU,      // convert signed number to unsigned
    UTOI,      // convert unsigned number to signed
    ITOF,      // convert signed number to float
    FTOI,      // convert float number to signed number.
    UTOF,      // convert unsigned number to a float.
    FTOU,      // convert float to unsigned number.

    /** Binary Functor Operators */
    ADD,    // addition
    SUB,    // subtraction
    MUL,    // multiplication
    DIV,    // division
    EXP,    // exponent
    MAX,    // max of two numbers
    MIN,    // min of two numbers
    MOD,    // modulus
    BAND,   // bitwise and
    BOR,    // bitwise or
    BXOR,   // bitwise exclusive or
    LAND,   // logical and
    LOR,    // logical or
    UADD,   // addition
    USUB,   // subtraction
    UMUL,   // multiplication
    UDIV,   // division
    UEXP,   // exponent
    UMAX,   // max of two numbers
    UMIN,   // min of two numbers
    UMOD,   // modulus
    UBAND,  // bitwise and
    UBOR,   // bitwise or
    UBXOR,  // bitwise exclusive or
    ULAND,  // logical and
    ULOR,   // logical or
    FADD,   // addition
    FSUB,   // subtraction
    FMUL,   // multiplication
    FDIV,   // division
    FEXP,   // exponent
    FMAX,   // max of two floats
    FMIN,   // min of two floats

    CAT,  // string concatenation

    /** Ternary Functor Operators */
    SUBSTR,  // substring

    /** Undefined */
    __UNDEFINED__,  // undefined operator
};

/**
 * Checks whether a functor operation can have a given argument count.
 */
inline bool isValidFunctorOpArity(const FunctorOp op, const size_t arity) {
    switch (op) {
        /** Unary Functor Operators */
        case FunctorOp::ORD:
        case FunctorOp::STRLEN:
        case FunctorOp::NEG:
        case FunctorOp::FNEG:
        case FunctorOp::BNOT:
        case FunctorOp::UBNOT:
        case FunctorOp::LNOT:
        case FunctorOp::ULNOT:
        case FunctorOp::TONUMBER:
        case FunctorOp::TOSTRING:
        case FunctorOp::ITOU:
        case FunctorOp::UTOI:
        case FunctorOp::ITOF:
        case FunctorOp::FTOI:
        case FunctorOp::UTOF:
        case FunctorOp::FTOU:
            return arity == 1;

        /** Binary Functor Operators */
        case FunctorOp::ADD:
        case FunctorOp::SUB:
        case FunctorOp::MUL:
        case FunctorOp::DIV:
        case FunctorOp::EXP:
        case FunctorOp::MOD:
        case FunctorOp::BAND:
        case FunctorOp::BOR:
        case FunctorOp::BXOR:
        case FunctorOp::LAND:
        case FunctorOp::LOR:
        case FunctorOp::UADD:
        case FunctorOp::USUB:
        case FunctorOp::UMUL:
        case FunctorOp::UDIV:
        case FunctorOp::UEXP:
        case FunctorOp::UMOD:
        case FunctorOp::UBAND:
        case FunctorOp::UBOR:
        case FunctorOp::UBXOR:
        case FunctorOp::ULAND:
        case FunctorOp::ULOR:
        case FunctorOp::FADD:
        case FunctorOp::FSUB:
        case FunctorOp::FMUL:
        case FunctorOp::FDIV:
        case FunctorOp::FEXP:
            return arity == 2;

        /** Ternary Functor Operators */
        case FunctorOp::SUBSTR:
            return arity == 3;

        /** Non-fixed */
        case FunctorOp::MAX:
        case FunctorOp::MIN:
        case FunctorOp::UMAX:
        case FunctorOp::UMIN:
        case FunctorOp::FMAX:
        case FunctorOp::FMIN:
        case FunctorOp::CAT:
            return arity >= 2;

        /** Undefined */
        case FunctorOp::__UNDEFINED__:
            break;
    }

    assert(false && "unsupported operator");
    return false;
}

/**
 * Return a string representation of a functor.
 */
inline std::string getSymbolForFunctorOp(const FunctorOp op) {
    switch (op) {
        /** Unary Functor Operators */
        case FunctorOp::ITOF:
            return "itof";
        case FunctorOp::ITOU:
            return "itou";
        case FunctorOp::UTOF:
            return "utof";
        case FunctorOp::UTOI:
            return "utoi";
        case FunctorOp::FTOI:
            return "ftoi";
        case FunctorOp::FTOU:
            return "ftou";
        case FunctorOp::ORD:
            return "ord";
        case FunctorOp::STRLEN:
            return "strlen";
        case FunctorOp::NEG:
        case FunctorOp::FNEG:
            return "-";
        case FunctorOp::BNOT:
        case FunctorOp::UBNOT:
            return "bnot";
        case FunctorOp::LNOT:
        case FunctorOp::ULNOT:
            return "lnot";
        case FunctorOp::TONUMBER:
            return "to_number";
        case FunctorOp::TOSTRING:
            return "to_string";

        /** Binary Functor Operators */
        case FunctorOp::ADD:
        case FunctorOp::FADD:
        case FunctorOp::UADD:
            return "+";
        case FunctorOp::SUB:
        case FunctorOp::USUB:
        case FunctorOp::FSUB:
            return "-";
        case FunctorOp::MUL:
        case FunctorOp::UMUL:
        case FunctorOp::FMUL:
            return "*";
        case FunctorOp::DIV:
        case FunctorOp::UDIV:
        case FunctorOp::FDIV:
            return "/";
        case FunctorOp::EXP:
        case FunctorOp::FEXP:
        case FunctorOp::UEXP:
            return "^";
        case FunctorOp::MOD:
        case FunctorOp::UMOD:
            return "%";
        case FunctorOp::BAND:
        case FunctorOp::UBAND:
            return "band";
        case FunctorOp::BOR:
        case FunctorOp::UBOR:
            return "bor";
        case FunctorOp::BXOR:
        case FunctorOp::UBXOR:
            return "bxor";
        case FunctorOp::LAND:
        case FunctorOp::ULAND:
            return "land";
        case FunctorOp::LOR:
        case FunctorOp::ULOR:
            return "lor";
        case FunctorOp::MAX:
        case FunctorOp::UMAX:
        case FunctorOp::FMAX:
            return "max";
        case FunctorOp::MIN:
        case FunctorOp::UMIN:
        case FunctorOp::FMIN:
            return "min";
        case FunctorOp::CAT:
            return "cat";

        /** Ternary Functor Operators */
        case FunctorOp::SUBSTR:
            return "substr";

        /** Undefined */
        case FunctorOp::__UNDEFINED__:
            break;
    }

    assert(false && "unsupported operator");
    exit(EXIT_FAILURE);
    return "?";
}

/**
 * Check a functor's return type (codomain).
 */
inline RamTypeAttribute functorReturnType(const FunctorOp op) {
    switch (op) {
        case FunctorOp::ORD:
        case FunctorOp::STRLEN:
        case FunctorOp::NEG:
        case FunctorOp::BNOT:
        case FunctorOp::LNOT:
        case FunctorOp::TONUMBER:
        case FunctorOp::ADD:
        case FunctorOp::SUB:
        case FunctorOp::MUL:
        case FunctorOp::DIV:
        case FunctorOp::EXP:
        case FunctorOp::BAND:
        case FunctorOp::BOR:
        case FunctorOp::BXOR:
        case FunctorOp::LAND:
        case FunctorOp::LOR:
        case FunctorOp::MOD:
        case FunctorOp::MAX:
        case FunctorOp::MIN:
        case FunctorOp::FTOI:
        case FunctorOp::UTOI:
            return RamTypeAttribute::Signed;
        case FunctorOp::UBNOT:
        case FunctorOp::ITOU:
        case FunctorOp::FTOU:
        case FunctorOp::ULNOT:
        case FunctorOp::UADD:
        case FunctorOp::USUB:
        case FunctorOp::UMUL:
        case FunctorOp::UDIV:
        case FunctorOp::UEXP:
        case FunctorOp::UMAX:
        case FunctorOp::UMIN:
        case FunctorOp::UMOD:
        case FunctorOp::UBAND:
        case FunctorOp::UBOR:
        case FunctorOp::UBXOR:
        case FunctorOp::ULAND:
        case FunctorOp::ULOR:
            return RamTypeAttribute::Unsigned;
        case FunctorOp::FMAX:
        case FunctorOp::FMIN:
        case FunctorOp::FNEG:
        case FunctorOp::FADD:
        case FunctorOp::FSUB:
        case FunctorOp::ITOF:
        case FunctorOp::UTOF:
        case FunctorOp::FMUL:
        case FunctorOp::FDIV:
        case FunctorOp::FEXP:
            return RamTypeAttribute::Float;
        case FunctorOp::TOSTRING:
        case FunctorOp::CAT:
        case FunctorOp::SUBSTR:
            return RamTypeAttribute::Symbol;
        case FunctorOp::__UNDEFINED__:
            break;
    }
    assert(false && "Bad functor return type query");
    exit(EXIT_FAILURE);
    return RamTypeAttribute::Record;  // Silence warning.
}

/**
 * Check the type of argument indicated by arg (0-indexed) of a functor op.
 */
inline RamTypeAttribute functorOpArgType(const size_t arg, const FunctorOp op) {
    switch (op) {
        case FunctorOp::ITOF:
        case FunctorOp::ITOU:
        case FunctorOp::NEG:
        case FunctorOp::BNOT:
        case FunctorOp::LNOT:
        case FunctorOp::TOSTRING:
            assert(arg == 0 && "unary functor out of bound");
            return RamTypeAttribute::Signed;
        case FunctorOp::FNEG:
        case FunctorOp::FTOI:
        case FunctorOp::FTOU:
            assert(arg == 0 && "unary functor out of bound");
            return RamTypeAttribute::Float;
        case FunctorOp::ORD:
        case FunctorOp::STRLEN:
        case FunctorOp::TONUMBER:
            assert(arg == 0 && "unary functor out of bound");
            return RamTypeAttribute::Symbol;
        case FunctorOp::UBNOT:
        case FunctorOp::ULNOT:
        case FunctorOp::UTOI:
        case FunctorOp::UTOF:
            assert(arg == 0 && "unary functor out of bound");
            return RamTypeAttribute::Unsigned;
        case FunctorOp::ADD:
        case FunctorOp::SUB:
        case FunctorOp::MUL:
        case FunctorOp::DIV:
        case FunctorOp::EXP:
        case FunctorOp::MOD:
        case FunctorOp::BAND:
        case FunctorOp::BOR:
        case FunctorOp::BXOR:
        case FunctorOp::LAND:
        case FunctorOp::LOR:
            assert(arg < 2 && "binary functor out of bound");
            return RamTypeAttribute::Signed;
        case FunctorOp::UADD:
        case FunctorOp::USUB:
        case FunctorOp::UMUL:
        case FunctorOp::UDIV:
        case FunctorOp::UEXP:
        case FunctorOp::UMOD:
        case FunctorOp::UBAND:
        case FunctorOp::UBOR:
        case FunctorOp::UBXOR:
        case FunctorOp::ULAND:
        case FunctorOp::ULOR:
            assert(arg < 2 && "binary functor out of bound");
            return RamTypeAttribute::Unsigned;
        case FunctorOp::FADD:
        case FunctorOp::FSUB:
        case FunctorOp::FMUL:
        case FunctorOp::FDIV:
        case FunctorOp::FEXP:
            assert(arg < 2 && "binary functor out of bound");
            return RamTypeAttribute::Float;
        case FunctorOp::SUBSTR:
            assert(arg < 3 && "ternary functor out of bound");
            if (arg == 0) {
                return RamTypeAttribute::Symbol;
            } else {
                return RamTypeAttribute::Signed;  // In the future: Change to unsigned
            }
        case FunctorOp::MAX:
        case FunctorOp::MIN:
            return RamTypeAttribute::Signed;
        case FunctorOp::UMAX:
        case FunctorOp::UMIN:
            return RamTypeAttribute::Unsigned;
        case FunctorOp::FMAX:
        case FunctorOp::FMIN:
            return RamTypeAttribute::Float;
        case FunctorOp::CAT:
            return RamTypeAttribute::Symbol;

        case FunctorOp::__UNDEFINED__:
            break;
    }
    assert(false && "unsupported operator");
    exit(EXIT_FAILURE);
    return RamTypeAttribute::Record;  // silence warning.
}

/**
 * Indicate whether a functor is overloaded.
 * At the moment, the signed versions are treated as representatives (because parser always returns a signed
 * version).
 */
inline bool isOverloadedFunctor(const FunctorOp functor) {
    switch (functor) {
        /* Unary */
        case FunctorOp::NEG:
        case FunctorOp::BNOT:
        case FunctorOp::LNOT:
        /* Binary */
        case FunctorOp::ADD:
        case FunctorOp::SUB:
        case FunctorOp::MUL:
        case FunctorOp::DIV:
        case FunctorOp::EXP:
        case FunctorOp::BAND:
        case FunctorOp::BOR:
        case FunctorOp::BXOR:
        case FunctorOp::LAND:
        case FunctorOp::LOR:
        case FunctorOp::MOD:
        case FunctorOp::MAX:
        case FunctorOp::MIN:
            return true;
            break;
        default:
            break;
    }

    return false;
}

/**
 * Convert an overloaded functor, so that it works with the requested type.
 * Example: toType = Float, functor = PLUS -> FPLUS (version of plus working on floats)
 */
inline FunctorOp convertOverloadedFunctor(const FunctorOp functor, const RamTypeAttribute toType) {
    switch (functor) {
        case FunctorOp::NEG:
            assert(toType == RamTypeAttribute::Float && "Invalid functor conversion from NEG");
            return FunctorOp::FNEG;
        case FunctorOp::BNOT:
            assert(toType == RamTypeAttribute::Unsigned && "Invalid functor conversion from UBNOT");
            return FunctorOp::UBNOT;
        case FunctorOp::LNOT:
            assert(toType == RamTypeAttribute::Unsigned && "Invalid functor conversion from LNOT");
            return FunctorOp::ULNOT;
        /* Binary */
        case FunctorOp::ADD:
            if (toType == RamTypeAttribute::Float) {
                return FunctorOp::FADD;
            } else if (toType == RamTypeAttribute::Unsigned) {
                return FunctorOp::UADD;
            }
            assert(false && "Invalid functor conversion");
            break;
        case FunctorOp::SUB:
            if (toType == RamTypeAttribute::Float) {
                return FunctorOp::FSUB;
            } else if (toType == RamTypeAttribute::Unsigned) {
                return FunctorOp::USUB;
            }
            assert(false && "Invalid functor conversion");
            break;
        case FunctorOp::MUL:
            if (toType == RamTypeAttribute::Float) {
                return FunctorOp::FMUL;
            } else if (toType == RamTypeAttribute::Unsigned) {
                return FunctorOp::UMUL;
            }
            assert(false && "Invalid functor conversion");
            break;
        case FunctorOp::DIV:
            if (toType == RamTypeAttribute::Float) {
                return FunctorOp::FDIV;
            } else if (toType == RamTypeAttribute::Unsigned) {
                return FunctorOp::UDIV;
            }
            assert(false && "Invalid functor conversion");
            break;
        case FunctorOp::EXP:
            if (toType == RamTypeAttribute::Float) {
                return FunctorOp::FEXP;
            } else if (toType == RamTypeAttribute::Unsigned) {
                return FunctorOp::UEXP;
            }
            assert(false && "Invalid functor conversion");
            break;
        case FunctorOp::MAX:
            if (toType == RamTypeAttribute::Float) {
                return FunctorOp::FMAX;
            } else if (toType == RamTypeAttribute::Unsigned) {
                return FunctorOp::UMAX;
            }
            assert(false && "Invalid functor conversion");
            break;
        case FunctorOp::MIN:
            if (toType == RamTypeAttribute::Float) {
                return FunctorOp::FMIN;
            } else if (toType == RamTypeAttribute::Unsigned) {
                return FunctorOp::UMIN;
            }
            assert(false && "Invalid functor conversion");
            break;
        case FunctorOp::BAND:
            assert(toType == RamTypeAttribute::Unsigned && "Invalid functor conversion");
            return FunctorOp::UBAND;
        case FunctorOp::BOR:
            assert(toType == RamTypeAttribute::Unsigned && "Invalid functor conversion");
            return FunctorOp::UBOR;
        case FunctorOp::BXOR:
            assert(toType == RamTypeAttribute::Unsigned && "Invalid functor conversion");
            return FunctorOp::UBXOR;
        case FunctorOp::LAND:
            assert(toType == RamTypeAttribute::Unsigned && "Invalid functor conversion");
            return FunctorOp::ULAND;
        case FunctorOp::LOR:
            assert(toType == RamTypeAttribute::Unsigned && "Invalid functor conversion");
            return FunctorOp::ULOR;
        case FunctorOp::MOD:
            assert(toType == RamTypeAttribute::Unsigned && "Invalid functor conversion");
            return FunctorOp::UMOD;
        default:
            break;
    }

    assert(false && "Invalid functor");
    exit(EXIT_FAILURE);
    return FunctorOp::__UNDEFINED__;
}

/**
 * Determines whether a functor should be written using infix notation (e.g. `a + b + c`)
 * or prefix notation (e.g. `+(a,b,c)`)
 */
inline bool isInfixFunctorOp(const FunctorOp op) {
    switch (op) {
        case FunctorOp::ADD:
        case FunctorOp::FADD:
        case FunctorOp::UADD:
        case FunctorOp::SUB:
        case FunctorOp::USUB:
        case FunctorOp::FSUB:
        case FunctorOp::MUL:
        case FunctorOp::FMUL:
        case FunctorOp::UMUL:
        case FunctorOp::DIV:
        case FunctorOp::FDIV:
        case FunctorOp::UDIV:
        case FunctorOp::EXP:
        case FunctorOp::FEXP:
        case FunctorOp::UEXP:
        case FunctorOp::BAND:
        case FunctorOp::UBAND:
        case FunctorOp::BOR:
        case FunctorOp::UBOR:
        case FunctorOp::BXOR:
        case FunctorOp::UBXOR:
        case FunctorOp::LAND:
        case FunctorOp::ULAND:
        case FunctorOp::LOR:
        case FunctorOp::ULOR:
        case FunctorOp::MOD:
        case FunctorOp::UMOD:
            return true;
        default:
            return false;
    }
}

}  // end of namespace souffle
