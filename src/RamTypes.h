/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2014, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file RamTypes.h
 *
 * Defines tuple element type and data type for keys on table columns
 *
 ***********************************************************************/

#pragma once

#include <cassert>
#include <cstdint>
#include <limits>
#include <ostream>
#include <type_traits>

namespace souffle {

enum class RamTypeAttribute {
    Symbol,
    Signed,    // Signed number
    Unsigned,  // Unsigned number
    Float,     // Floating point number.
    Record,
};

// Printing of the RamTypeAttribute Enum.
// To be utilised in synthesizer.
inline std::ostream& operator<<(std::ostream& os, RamTypeAttribute T) {
    switch (T) {
        case RamTypeAttribute::Symbol:
            os << "RamTypeAttribute::Symbol";
            break;
        case RamTypeAttribute::Signed:
            os << "RamTypeAttribute::Signed";
            break;
        case RamTypeAttribute::Float:
            os << "RamTypeAttribute::Float";
            break;
        case RamTypeAttribute::Unsigned:
            os << "RamTypeAttribute::Unsigned";
            break;
        case RamTypeAttribute::Record:
            os << "RamTypeAttribute::Record";
    }
    return os;
}

/** Convert a char to RamTypeAttribute */
inline RamTypeAttribute RamPrimitiveFromChar(char c) {
    RamTypeAttribute RamType;
    switch (c) {
        case 's':
            RamType = RamTypeAttribute::Symbol;
            break;
        case 'i':
            RamType = RamTypeAttribute::Signed;
            break;
        case 'f':
            RamType = RamTypeAttribute::Float;
            break;
        case 'u':
            RamType = RamTypeAttribute::Unsigned;
            break;
        case 'r':
            RamType = RamTypeAttribute::Record;
            break;
        default:
            assert(false && "Invalid RamTypeAttribute Char");
    }
    return RamType;
}

/**
 * Check if type is numeric.
 */
inline bool isNumericType(RamTypeAttribute ramType) {
    switch (ramType) {
        case RamTypeAttribute::Signed:
        case RamTypeAttribute::Unsigned:
        case RamTypeAttribute::Float:
            return true;
        case RamTypeAttribute::Symbol:
        case RamTypeAttribute::Record:
            return false;
    }
    return false;  // silence warning
}

/**
 * Types of elements in a tuple.
 *
 * Default domain has size of 32 bits; may be overridden by user
 * defining RAM_DOMAIN_SIZE.
 */

#ifndef RAM_DOMAIN_SIZE
#define RAM_DOMAIN_SIZE 32
#endif

#if RAM_DOMAIN_SIZE == 64
using RamDomain = int64_t;
using RamSigned = RamDomain;
using RamUnsigned = uint64_t;
// There is not standard fixed size double/float.
using RamFloat = double;
#else
using RamDomain = int32_t;
using RamSigned = RamDomain;
using RamUnsigned = uint32_t;
// There is no standard - fixed size double/float.
using RamFloat = float;
#endif

// Compile time sanity checks
static_assert(std::is_integral<RamSigned>::value && std::is_signed<RamSigned>::value,
        "RamSigned must be represented by a signed type.");
static_assert(std::is_integral<RamUnsigned>::value && !std::is_signed<RamUnsigned>::value,
        "RamUnsigned must be represented by an unsigned type.");
static_assert(std::is_floating_point<RamFloat>::value && sizeof(RamFloat) * 8 == RAM_DOMAIN_SIZE,
        "RamFloat must be represented by a floating point and have the same size as other types.");

template <typename T>
constexpr bool isRamType = (std::is_same<T, RamDomain>::value || std::is_same<T, RamSigned>::value ||
                            std::is_same<T, RamUnsigned>::value || std::is_same<T, RamFloat>::value);

/**
In C++20 there will be a new way to cast between types by reinterpreting bits (std::bit_cast),
but as of January 2020 it is not yet supported.
**/

/** Cast a type by reinterpreting its bits. Domain is restricted to Ram Types only.
 * Template takes two types (second type is never necessary because it can be deduced from the argument)
 * The following always holds
 * For type T and a : T
 * ramBitCast<T>(ramBitCast<RamDomain>(a)) == a
 **/
template <typename To = RamDomain, typename From>
inline To ramBitCast(From RamElement) {
    static_assert(isRamType<From> && isRamType<To>, "Bit casting should only be used on Ram Types.");
    union {
        From source;
        To destination;
    } Union;
    Union.source = RamElement;
    return Union.destination;
}

/** lower and upper boundaries for the ram domain **/
#define MIN_RAM_DOMAIN (std::numeric_limits<RamDomain>::min())
#define MAX_RAM_DOMAIN (std::numeric_limits<RamDomain>::max())

/** search signature of a RAM operation; each bit represents an attribute of a relation.
 * A one represents that the attribute has an assigned value; a zero represents that
 * no value exists (i.e. attribute is unbounded) in the search. */
using SearchSignature = uint64_t;

}  // end of namespace souffle
