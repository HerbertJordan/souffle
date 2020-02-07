/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2015, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file CompiledIndexUtils.h
 *
 * The central file covering the data structure utilized by
 * the souffle compiler for representing relations in compiled queries.
 *
 ***********************************************************************/

#pragma once

#include "BTree.h"
#include "Brie.h"
#include "CompiledTuple.h"
#include "EquivalenceRelation.h"
#include "IterUtils.h"
#include "RamTypes.h"
#include "Util.h"
#include <cassert>
#include <iterator>
#include <ostream>
#include <string>
#include <type_traits>
#include <vector>

namespace souffle {

namespace ram {

/**
 * A namespace enclosing template-meta-programming utilities for handling
 * parameter lists for templates.
 */
namespace column_utils {

// -- checks whether a given number is contained in a list of numbers --

template <unsigned E, unsigned... List>
struct contains;

template <unsigned E>
struct contains<E> {
    static constexpr size_t value = 0u;
};

template <unsigned E, unsigned F, unsigned... Rest>
struct contains<E, F, Rest...> {
    static constexpr size_t value = (E == F) || contains<E, Rest...>::value;
};

// -- check uniqueness of a list of integer values --

template <unsigned... List>
struct unique;

template <>
struct unique<> {
    static constexpr size_t value = 1u;
};

template <unsigned E, unsigned... Rest>
struct unique<E, Rest...> {
    static constexpr size_t value = !contains<E, Rest...>::value && unique<Rest...>::value;
};

}  // end namespace column_utils

/**
 * A namespace enclosing utilities required by indices.
 */
namespace index_utils {

// -------- generic tuple comparator ----------

template <unsigned... Columns>
struct comparator;

template <unsigned First, unsigned... Rest>
struct comparator<First, Rest...> {
    template <typename T>
    int operator()(const T& a, const T& b) const {
        return (a[First] < b[First]) ? -1 : ((a[First] > b[First]) ? 1 : comparator<Rest...>()(a, b));
    }
    template <typename T>
    bool less(const T& a, const T& b) const {
        return a[First] < b[First] || (a[First] == b[First] && comparator<Rest...>().less(a, b));
    }
    template <typename T>
    bool equal(const T& a, const T& b) const {
        return a[First] == b[First] && comparator<Rest...>().equal(a, b);
    }
};

template <>
struct comparator<> {
    template <typename T>
    int operator()(const T& a, const T& b) const {
        return 0;
    }
    template <typename T>
    bool less(const T& a, const T& b) const {
        return false;
    }
    template <typename T>
    bool equal(const T& a, const T& b) const {
        return true;
    }
};

// ----- a comparator wrapper dereferencing pointers ----------
//         (required for handling indirect indices)

template <typename Comp>
struct deref_compare {
    template <typename T>
    int operator()(const T& a, const T& b) const {
        Comp comp;
        return comp(*a, *b);
    }
    template <typename T>
    bool less(const T& a, const T& b) const {
        Comp comp;
        return comp.less(*a, *b);
    }
    template <typename T>
    bool equal(const T& a, const T& b) const {
        Comp comp;
        return comp.equal(*a, *b);
    }
};

// ----- a utility for printing lists of parameters -------
//    (required for printing descriptions of relations)

template <unsigned... Columns>
struct print;

template <>
struct print<> {
    friend std::ostream& operator<<(std::ostream& out, const print&) {
        return out;
    }
};

template <unsigned Last>
struct print<Last> {
    friend std::ostream& operator<<(std::ostream& out, const print&) {
        return out << Last;
    }
};

template <unsigned First, unsigned Second, unsigned... Rest>
struct print<First, Second, Rest...> {
    friend std::ostream& operator<<(std::ostream& out, const print&) {
        return out << First << "," << print<Second, Rest...>();
    }
};

}  // namespace index_utils

/**
 * The index class is utilized as a template-meta-programming structure
 * to specify and realize indices.
 *
 * @tparam Columns ... the order in which elements of the relation to be indexed
 * 				shell be considered by this index.
 */
template <unsigned... Columns>
struct index {
    // check validity of this index - column fields need to be unique
    static_assert(column_utils::unique<Columns...>::value, "Invalid duplication of columns!");

    // the comparator associated to this index
    using comparator = index_utils::comparator<Columns...>;

    // enables to check whether the given column is covered by this index or not
    template <unsigned Col>
    struct covers {
        static constexpr size_t value = column_utils::contains<Col, Columns...>::value;
    };

    // the length of the index
    static constexpr size_t size = sizeof...(Columns);

    // enables instances of this class to be printed (for printing relation-structure descriptions)
    friend std::ostream& operator<<(std::ostream& out, const index& index) {
        return out << "<" << index_utils::print<Columns...>() << ">";
    }
};

/**
 * A namespace enclosing utilities required relations to handle indices.
 */
namespace index_utils {

// -------------------------------------------------------------
//                     Static Index Utilities
// -------------------------------------------------------------

// -- check whether a given list only consists of indices --

template <typename... Index>
struct all_indices {
    static constexpr size_t value = 0u;
};

template <>
struct all_indices<> {
    static constexpr size_t value = 1u;
};

template <unsigned... Columns, typename... Rest>
struct all_indices<index<Columns...>, Rest...> {
    static constexpr size_t value = all_indices<Rest...>::value;
};

// -- checks whether a list of typed contains a certain type --

template <typename E, typename... List>
struct contains;

template <typename E>
struct contains<E> {
    static constexpr size_t value = 0u;
};

template <typename E, typename F, typename... Rest>
struct contains<E, F, Rest...> {
    static constexpr size_t value = contains<E, Rest...>::value;
};

template <typename E, typename... Rest>
struct contains<E, E, Rest...> {
    static constexpr size_t value = 1u;
};

// -- check whether a given list is a list of unique indices --

template <typename... Index>
struct unique;

template <>
struct unique<> {
    static constexpr size_t value = 1u;
};

template <typename First, typename... Rest>
struct unique<First, Rest...> {
    static constexpr size_t value = all_indices<First, Rest...>::value && !contains<First, Rest...>::value;
};

// -- a utility extending a given index by another column --
//   e.g. index<1,0>   =>    index<1,0,2>

template <typename Index, unsigned column>
struct extend;

template <unsigned... Columns, unsigned Col>
struct extend<index<Columns...>, Col> {
    using type = index<Columns..., Col>;
};

// -- obtains a full index for a given arity --

template <unsigned arity>
struct get_full_index {
    using type = typename extend<typename get_full_index<arity - 1>::type, arity - 1>::type;
};

template <>
struct get_full_index<0> {
    using type = index<>;
};

}  // namespace index_utils

}  // end namespace ram

}  // end namespace souffle
