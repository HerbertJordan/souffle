/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file AstLiteral.h
 *
 * Define classes for Literals and its subclasses atoms, negated atoms,
 * and binary relations.
 *
 ***********************************************************************/

#pragma once

#include "AstAbstract.h"
#include "AstNode.h"
#include "AstRelationIdentifier.h"
#include "BinaryConstraintOps.h"
#include "Util.h"

#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "AstAbstract.h"
#include "AstNode.h"

namespace souffle {

class AstRelation;
class AstClause;
class AstProgram;
class AstAtom;

/**
 * Subclass of Literal that represents the use of a relation
 * either in the head or in the body of a Clause, e.g., parent(x,y).
 * The arguments of the atom can be variables or constants.
 */
class AstAtom : public AstLiteral {
public:
    AstAtom(AstRelationIdentifier name = AstRelationIdentifier()) : name(std::move(name)) {}

    /** Return the name of this atom */
    const AstRelationIdentifier& getName() const {
        return name;
    }

    /** Return the arity of the atom */
    size_t getArity() const {
        return arguments.size();
    }

    /** Set atom name */
    void setName(const AstRelationIdentifier& n) {
        name = n;
    }

    /** Returns this class as the referenced atom */
    const AstAtom* getAtom() const override {
        return this;
    }

    /** Add argument to the atom */
    void addArgument(std::unique_ptr<AstArgument> arg) {
        arguments.push_back(std::move(arg));
    }

    /** Return the i-th argument of the atom */
    AstArgument* getArgument(size_t idx) const {
        return arguments[idx].get();
    }

    /** Replace the argument at the given index with the given argument */
    void setArgument(size_t idx, std::unique_ptr<AstArgument> newArg) {
        arguments[idx].swap(newArg);
    }

    /** Provides access to the list of arguments of this atom */
    std::vector<AstArgument*> getArguments() const {
        return toPtrVector(arguments);
    }

    /** Return the number of arguments */
    size_t argSize() const {
        return arguments.size();
    }

    void print(std::ostream& os) const override {
        os << getName() << "(";

        for (size_t i = 0; i < arguments.size(); ++i) {
            if (i != 0) {
                os << ",";
            }
            if (arguments[i] != nullptr) {
                arguments[i]->print(os);
            } else {
                os << "_";
            }
        }
        os << ")";
    }

    AstAtom* clone() const override {
        auto res = new AstAtom(name);
        res->setSrcLoc(getSrcLoc());
        for (const auto& cur : arguments) {
            res->arguments.emplace_back(cur->clone());
        }
        return res;
    }

    void apply(const AstNodeMapper& map) override {
        for (auto& arg : arguments) {
            arg = map(std::move(arg));
        }
    }

    std::vector<const AstNode*> getChildNodes() const override {
        std::vector<const AstNode*> res;
        for (auto& cur : arguments) {
            res.push_back(cur.get());
        }
        return res;
    }

protected:
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstAtom*>(&node));
        const auto& other = static_cast<const AstAtom&>(node);
        return name == other.name && equal_targets(arguments, other.arguments);
    }

    /** Name of the atom */
    AstRelationIdentifier name;

    /** Arguments of the atom */
    std::vector<std::unique_ptr<AstArgument>> arguments;
};

/**
 * Subclass of Literal that represents a negated atom, * e.g., !parent(x,y).
 * A Negated atom occurs in a body of clause and cannot occur in a head of a clause.
 */
class AstNegation : public AstLiteral {
public:
    AstNegation(std::unique_ptr<AstAtom> atom) : atom(std::move(atom)) {}

    /** Returns the nested atom as the referenced atom */
    const AstAtom* getAtom() const override {
        return atom.get();
    }

    /** Return the negated atom */
    AstAtom* getAtom() {
        return atom.get();
    }

    /** Output to a given stream */
    void print(std::ostream& os) const override {
        os << "!";
        atom->print(os);
    }

    /** Creates a clone of this AST sub-structure */
    AstNegation* clone() const override {
        auto* res = new AstNegation(std::unique_ptr<AstAtom>(atom->clone()));
        res->setSrcLoc(getSrcLoc());
        return res;
    }

    /** Mutates this node */
    void apply(const AstNodeMapper& map) override {
        atom = map(std::move(atom));
    }

    /** Obtains a list of all embedded child nodes */
    std::vector<const AstNode*> getChildNodes() const override {
        return {atom.get()};
    }

protected:
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstNegation*>(&node));
        const auto& other = static_cast<const AstNegation&>(node);
        return *atom == *other.atom;
    }

    /** A pointer to the negated Atom */
    std::unique_ptr<AstAtom> atom;
};

/**
 * Subclass of Literal that represents a negated atom, * e.g., !parent(x,y).
 * A Negated atom occurs in a body of clause and cannot occur in a head of a clause.
 *
 * Specialised for provenance: used for existence check that tuple doesn't already exist
 */
class AstProvenanceNegation : public AstLiteral {
public:
    AstProvenanceNegation(std::unique_ptr<AstAtom> atom) : atom(std::move(atom)) {}

    /** Returns the nested atom as the referenced atom */
    const AstAtom* getAtom() const override {
        return atom.get();
    }

    /** Return the negated atom */
    AstAtom* getAtom() {
        return atom.get();
    }

    void print(std::ostream& os) const override {
        os << "prov!";
        atom->print(os);
    }

    AstProvenanceNegation* clone() const override {
        auto* res = new AstProvenanceNegation(std::unique_ptr<AstAtom>(atom->clone()));
        res->setSrcLoc(getSrcLoc());
        return res;
    }

    void apply(const AstNodeMapper& map) override {
        atom = map(std::move(atom));
    }

    std::vector<const AstNode*> getChildNodes() const override {
        return {atom.get()};
    }

protected:
    bool equal(const AstNode& node) const override {
        assert(dynamic_cast<const AstProvenanceNegation*>(&node));
        const auto& other = static_cast<const AstProvenanceNegation&>(node);
        return *atom == *other.atom;
    }

    /** A pointer to the negated Atom */
    std::unique_ptr<AstAtom> atom;
};

/**
 * Subclass of Literal that represents a logical constraint
 */
class AstConstraint : public AstLiteral {
public:
    const AstAtom* getAtom() const override {
        // This kind of literal has no nested atom
        return nullptr;
    }

    /** Negates the constraint */
    virtual void negate() = 0;

    AstConstraint* clone() const override = 0;
};

/**
 * Subclass of Constraint that represents a constant 'true'
 * or 'false' value.
 *
 * TODO (b-scholz): Let's make two separate classes AstTrue/AstFalse
 */
class AstBooleanConstraint : public AstConstraint {
public:
    AstBooleanConstraint(bool truthValue) : truthValue(truthValue) {}

    bool isTrue() const {
        return truthValue;
    }

    void negate() override {
        truthValue = !truthValue;
    }

    void print(std::ostream& os) const override {
        os << (truthValue ? "true" : "false");
    }

    AstBooleanConstraint* clone() const override {
        auto* res = new AstBooleanConstraint(truthValue);
        res->setSrcLoc(getSrcLoc());
        return res;
    }

protected:
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstBooleanConstraint*>(&node));
        const auto& other = static_cast<const AstBooleanConstraint&>(node);
        return truthValue == other.truthValue;
    }

    /** Truth value */
    bool truthValue;
};

/**
 * Subclass of Constraint that represents a binary constraint
 * e.g., x = y.
 */
class AstBinaryConstraint : public AstConstraint {
public:
    AstBinaryConstraint(
            BinaryConstraintOp o, std::unique_ptr<AstArgument> ls, std::unique_ptr<AstArgument> rs)
            : operation(o), lhs(std::move(ls)), rhs(std::move(rs)) {}

    AstBinaryConstraint(
            const std::string& op, std::unique_ptr<AstArgument> ls, std::unique_ptr<AstArgument> rs)
            : operation(toBinaryConstraintOp(op)), lhs(std::move(ls)), rhs(std::move(rs)) {}

    /** Return LHS argument */
    AstArgument* getLHS() const {
        return lhs.get();
    }

    /** Return RHS argument */
    AstArgument* getRHS() const {
        return rhs.get();
    }

    /** Return binary operator */
    BinaryConstraintOp getOperator() const {
        return operation;
    }

    /** Update the binary operator */
    void setOperator(BinaryConstraintOp op) {
        operation = op;
    }

    /** Negates the constraint */
    void negate() override {
        setOperator(souffle::negatedConstraintOp(operation));
    }

    /** Check whether constraint is a numeric constraint */
    bool isNumerical() const {
        return isNumericBinaryConstraintOp(operation);
    }

    /** Check whether constraint is a symbolic constraint */
    bool isSymbolic() const {
        return isSymbolicBinaryConstraintOp(operation);
    }

    void print(std::ostream& os) const override {
        lhs->print(os);
        os << " " << toBinaryConstraintSymbol(operation) << " ";
        rhs->print(os);
    }

    AstBinaryConstraint* clone() const override {
        auto* res = new AstBinaryConstraint(operation, std::unique_ptr<AstArgument>(lhs->clone()),
                std::unique_ptr<AstArgument>(rhs->clone()));
        res->setSrcLoc(getSrcLoc());
        return res;
    }

    void apply(const AstNodeMapper& map) override {
        lhs = map(std::move(lhs));
        rhs = map(std::move(rhs));
    }

    std::vector<const AstNode*> getChildNodes() const override {
        return {lhs.get(), rhs.get()};
    }

protected:
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstBinaryConstraint*>(&node));
        const auto& other = static_cast<const AstBinaryConstraint&>(node);
        return operation == other.operation && *lhs == *other.lhs && *rhs == *other.rhs;
    }

    /** The operator in this relation */
    BinaryConstraintOp operation;

    /** Left-hand side argument of a binary operation */
    std::unique_ptr<AstArgument> lhs;

    /** Right-hand side argument of a binary operation */
    std::unique_ptr<AstArgument> rhs;
};

}  // end of namespace souffle
