/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2015, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file RamNode.h
 *
 * Top level syntactic element of intermediate representation,
 * i.e., a node of the RAM machine code.
 *
 ***********************************************************************/

#pragma once

#include <cassert>
#include <iostream>
#include <memory>
#include <typeinfo>
#include <vector>

namespace souffle {

enum RamNodeKind {
	// Relation
	RK_Relation,
	RK_RelationReference,

	// Expressions
	RK_TupleElement,
	RK_Number,
	RK_IntrinsicOperator,
	RK_UserDefinedOperator,
	RK_AutoIncrement,
	RK_PackRecord,
	RK_SubroutineArgument,
	RK_UndefValue,

	// Conditions
	RK_True,
	RK_False,
	RK_EmptinessCheck,
	RK_ExistenceCheck,
	RK_ProvenanceExistenceCheck,
	RK_Conjunction,
	RK_Negation,
	RK_Constraint,

	// Operations
	RK_Filter,
	RK_Break,
	RK_Project,
	RK_SubroutineReturnValue,
	RK_UnpackRecord,
	RK_ParallelScan,
	RK_Scan,
	RK_ParallelIndexScan,
	RK_IndexScan,
	RK_ParallelChoice,
	RK_Choice,
	RK_ParallelIndexChoice,
	RK_IndexChoice,
	RK_Aggregate,
	RK_IndexAggregate,

	// Statements
	RK_Create,
	RK_Fact,
	RK_Load,
	RK_Store,
	RK_Query,
	RK_Clear,
	RK_Drop,
	RK_LogSize,

	RK_Merge,
	RK_Swap,

	// Control-flow
	RK_Program,
	RK_Sequence,
	RK_Loop,
	RK_Parallel,
	RK_Exit,
	RK_LogTimer,
	RK_LogRelationTimer,
	RK_DebugInfo,
	RK_Stratum
};

class RamNodeMapper;

/**
 *  @class RamNode
 *  @brief RamNode is a superclass for all RAM IR classes.
 */
class RamNode {
public:
	const RamNodeKind kind;
    RamNode(RamNodeKind kind) : kind(kind) {}

    /*
     * @brief A virtual destructor for RAM nodes
     */
    virtual ~RamNode() = default;

    /**
     * @brief Equivalence check for two RAM nodes
     */
    bool operator==(const RamNode& other) const {
        return this == &other || (typeid(*this) == typeid(other) && equal(other));
    }

    /**
     * @brief Inequality check for two RAM nodes
     */
    bool operator!=(const RamNode& other) const {
        return !(*this == other);
    }

    /**
     * @brief Create a clone (i.e. deep copy) of this node
     */
    virtual RamNode* clone() const = 0;

    /**
     * @brief Apply the mapper to all child nodes
     */
    virtual void apply(const RamNodeMapper& mapper) {}

    /**
     * @brief Obtain list of all embedded child nodes
     */
    virtual std::vector<const RamNode*> getChildNodes() const {
        return {};
    }

    /**
     * @brief Print RAM node
     */
    virtual void print(std::ostream& out = std::cout) const = 0;

    /**
     * Print RAM on a stream
     */
    friend std::ostream& operator<<(std::ostream& out, const RamNode& node) {
        node.print(out);
        return out;
    }

protected:
    /**
     * @brief Equality check for two RAM nodes.
     * Default action is that nothing needs to be checked.
     */
    virtual bool equal(const RamNode& other) const {
        return true;
    }
};

/**
 * @class RamNodeMapper
 * @brief An abstract class for manipulating RAM Nodes by substitution
 */
class RamNodeMapper {
public:
    virtual ~RamNodeMapper() = default;

    /**
     * @brief Abstract replacement method for a node.
     *
     * If the given nodes is to be replaced, the handed in node
     * will be destroyed by the mapper and the returned node
     * will become owned by the caller.
     */
    virtual std::unique_ptr<RamNode> operator()(std::unique_ptr<RamNode> node) const = 0;

    /**
     * @brief Wrapper for any subclass of the RAM node hierarchy performing type casts.
     */
    template <typename T>
    std::unique_ptr<T> operator()(std::unique_ptr<T> node) const {
        std::unique_ptr<RamNode> resPtr =
                (*this)(std::unique_ptr<RamNode>(static_cast<RamNode*>(node.release())));
        assert(nullptr != dynamic_cast<T*>(resPtr.get()) && "Invalid target node!");
        return std::unique_ptr<T>(dynamic_cast<T*>(resPtr.release()));
    }
};

namespace detail {

/**
 * @class LambdaRamNodeMapper
 * @brief A special RamNodeMapper wrapping a lambda conducting node transformations.
 */
template <typename Lambda>
class LambdaRamNodeMapper : public RamNodeMapper {
    const Lambda& lambda;

public:
    /**
     * @brief Constructor for LambdaRamNodeMapper
     */
    LambdaRamNodeMapper(const Lambda& lambda) : lambda(lambda) {}

    /**
     * @brief Applies lambda
     */
    std::unique_ptr<RamNode> operator()(std::unique_ptr<RamNode> node) const override {
        return lambda(std::move(node));
    }
};
}  // namespace detail

/**
 * @brief Creates a node mapper based on a corresponding lambda expression.
 */
template <typename Lambda>
detail::LambdaRamNodeMapper<Lambda> makeLambdaRamMapper(const Lambda& lambda) {
    return detail::LambdaRamNodeMapper<Lambda>(lambda);
}

}  // end of namespace souffle
