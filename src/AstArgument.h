/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file AstArgument.h
 *
 * Define the classes Argument, Variable, and Constant to represent
 * variables and constants in literals. Variable and Constant are
 * sub-classes of class argument.
 *
 ***********************************************************************/

#pragma once

#include "AstAbstract.h"
#include "AstNode.h"
#include "AstType.h"
#include "FunctorOps.h"
#include "SymbolTable.h"
#include "Util.h"
#include <cassert>
#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace souffle {

/**
 * Named Variable
 */
class AstVariable : public AstArgument {
public:
    AstVariable(std::string name) : name(name) {}

    void print(std::ostream& os) const override {
        os << name;
    }

    /** set variable name */
    void setName(const std::string& name) {
        this->name = name;
    }

    /** @return variable name */
    const std::string& getName() const {
        return name;
    }

    AstVariable* clone() const override {
        auto* res = new AstVariable(name);
        res->setSrcLoc(getSrcLoc());
        return res;
    }

protected:
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstVariable*>(&node));
        const auto& other = static_cast<const AstVariable&>(node);
        return name == other.name;
    }

    /** variable name */
    std::string name;
};

/**
 * Unnamed Variable
 */
class AstUnnamedVariable : public AstArgument {
public:
    void print(std::ostream& os) const override {
        os << "_";
    }

    AstUnnamedVariable* clone() const override {
        auto* res = new AstUnnamedVariable();
        res->setSrcLoc(getSrcLoc());
        return res;
    }
};

/**
 * Counter
 */
class AstCounter : public AstArgument {
public:
    void print(std::ostream& os) const override {
        os << "$";
    }

    AstCounter* clone() const override {
        auto* res = new AstCounter();
        res->setSrcLoc(getSrcLoc());
        return res;
    }
};

/**
 * Abstract Constant
 */
class AstConstant : public AstArgument {
protected:
    AstConstant(RamDomain i) : ramRepresentation(i) {}

public:
    /** @return Return the ram representation of this constant */
    RamDomain getRamRepresentation() const {
        return ramRepresentation;
    }

protected:
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstConstant*>(&node));
        const auto& other = static_cast<const AstConstant&>(node);
        return ramRepresentation == other.ramRepresentation;
    }

    /** Constant represented as RamDomain value.
     * In the case of a string this is the entry in symbol table.
     * In the case of a float/unsigned this is the bit cast of the value. */
    RamDomain ramRepresentation;
};

/**
 * String Constant
 */
class AstStringConstant : public AstConstant {
public:
    AstStringConstant(SymbolTable& symTable, const std::string& c)
            : AstConstant(symTable.lookup(c)), symTable(symTable) {}

    void print(std::ostream& os) const override {
        os << "\"" << getConstant() << "\"";
    }

    /** @return String representation of this Constant */
    const std::string& getConstant() const {
        return symTable.resolve(ramRepresentation);
    }

    AstStringConstant* clone() const override {
        auto* res = new AstStringConstant(symTable, ramRepresentation);
        res->setSrcLoc(getSrcLoc());
        return res;
    }

private:
    // TODO (b-scholz): Remove Symbol Table / store as string / change hierarchy
    // Don't use numbers to store strings in AST
    AstStringConstant(SymbolTable& symTable, size_t index) : AstConstant(index), symTable(symTable) {}

    /** Symbol table */
    SymbolTable& symTable;
};

/**
 * Numeric Constant
 */
template <typename numericType>  // numericType ⲉ {RamSigned, RamUnsigned, RamFloat}
class AstNumericConstant : public AstConstant {
public:
    AstNumericConstant(numericType value) : AstConstant(ramBitCast(value)) {}

    void print(std::ostream& os) const override {
        os << getConstant();
    }

    /** Get the value of the constant. */
    numericType getConstant() const {
        return ramBitCast<numericType>(ramRepresentation);
    }

    AstNumericConstant<numericType>* clone() const override {
        auto* copy = new AstNumericConstant<numericType>(getConstant());
        copy->setSrcLoc(getSrcLoc());
        return copy;
    }
};

// This definitions are used by AstVisitor.
using AstNumberConstant = AstNumericConstant<RamSigned>;
using AstFloatConstant = AstNumericConstant<RamFloat>;
using AstUnsignedConstant = AstNumericConstant<RamUnsigned>;

/**
 * Nil Constant
 */
class AstNilConstant : public AstConstant {
public:
    AstNilConstant() : AstConstant(0) {}

    void print(std::ostream& os) const override {
        os << "nil";
    }

    AstNilConstant* clone() const override {
        auto* res = new AstNilConstant();
        res->setSrcLoc(getSrcLoc());
        return res;
    }
};

/**
 * Abstract Term
 */
class AstTerm : public AstArgument {
protected:
    AstTerm() = default;
    AstTerm(std::vector<std::unique_ptr<AstArgument>> operands) : args(std::move(operands)){};

public:
    /** get arguments */
    std::vector<AstArgument*> getArguments() const {
        return toPtrVector(args);
    }

    /** get number of arguments */
    size_t getArity() const {
        return args.size();
    }

    /** get argument at idx */
    AstArgument* getArg(const size_t idx) const {
        assert(idx < args.size() && "argument index out of bounds");
        return args[idx].get();
    }

    /** set argument */
    void setArg(const size_t idx, std::unique_ptr<AstArgument> arg) {
        assert(idx < args.size() && "argument index out of bounds");
        args[idx] = std::move(arg);
    }

    /** add argument to argument list */
    void add(std::unique_ptr<AstArgument> arg) {
        args.push_back(std::move(arg));
    }

    std::vector<const AstNode*> getChildNodes() const override {
        auto res = AstArgument::getChildNodes();
        for (auto& cur : args) {
            res.push_back(cur.get());
        }
        return res;
    }

    void apply(const AstNodeMapper& map) override {
        for (auto& arg : args) {
            arg = map(std::move(arg));
        }
    }

protected:
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstTerm*>(&node));
        const auto& other = static_cast<const AstTerm&>(node);
        return equal_targets(args, other.args);
    }

    /** Arguments */
    std::vector<std::unique_ptr<AstArgument>> args;
};

/**
 * Functor class
 */

class AstFunctor : public AstTerm {
protected:
    AstFunctor() = default;
    AstFunctor(std::vector<std::unique_ptr<AstArgument>> operands) : AstTerm(std::move(operands)) {}
};

/**
 * Intrinsic Functor
 */
class AstIntrinsicFunctor : public AstFunctor {
public:
    template <typename... Operands>
    AstIntrinsicFunctor(FunctorOp function, Operands... operands) : function(function) {
        std::unique_ptr<AstArgument> tmp[] = {std::move(operands)...};
        for (auto& cur : tmp) {
            add(std::move(cur));
        }
        assert(isValidFunctorOpArity(function, args.size()) && "invalid number of arguments for functor");
    }

    AstIntrinsicFunctor(FunctorOp function, std::vector<std::unique_ptr<AstArgument>> operands)
            : AstFunctor(std::move(operands)), function(function) {
        assert(isValidFunctorOpArity(function, args.size()) && "invalid number of arguments for functor");
    }

    void print(std::ostream& os) const override {
        if (isInfixFunctorOp(function)) {
            os << "(";
            os << join(args, getSymbolForFunctorOp(function), print_deref<std::unique_ptr<AstArgument>>());
            os << ")";
        } else {
            os << getSymbolForFunctorOp(function);
            os << "(";
            os << join(args, ",", print_deref<std::unique_ptr<AstArgument>>());
            os << ")";
        }
    }

    /** get function */
    FunctorOp getFunction() const {
        return function;
    }

    /** set function */
    void setFunction(const FunctorOp functor) {
        function = functor;
    }

    /** get the return type of the functor. */
    RamTypeAttribute getReturnType() const {
        return functorReturnType(function);
    }

    /** get type of the functor argument*/
    RamTypeAttribute getArgType(const size_t arg) const {
        return functorOpArgType(arg, function);
    }

    AstIntrinsicFunctor* clone() const override {
        std::vector<std::unique_ptr<AstArgument>> argsCopy;
        for (auto& arg : args) {
            argsCopy.emplace_back(arg->clone());
        }
        auto res = new AstIntrinsicFunctor(function, std::move(argsCopy));
        res->setSrcLoc(getSrcLoc());
        return res;
    }

protected:
    /** Implements the node comparison for this node type */
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstIntrinsicFunctor*>(&node));
        const auto& other = static_cast<const AstIntrinsicFunctor&>(node);
        return function == other.function && AstFunctor::equal(node);
    }

    /** Function */
    FunctorOp function;
};

/**
 * User-Defined Functor
 */
class AstUserDefinedFunctor : public AstFunctor {
public:
    AstUserDefinedFunctor() = default;
    AstUserDefinedFunctor(std::string name, std::vector<std::unique_ptr<AstArgument>> args)
            : AstFunctor(std::move(args)), name(std::move(name)){};

    /** print user-defined functor */
    void print(std::ostream& os) const override {
        os << '@' << name << "(" << join(args, ",", print_deref<std::unique_ptr<AstArgument>>()) << ")";
    }

    /** get name */
    const std::string& getName() const {
        return name;
    }

    /** set name */
    void setName(const std::string& name) {
        this->name = name;
    }

    AstUserDefinedFunctor* clone() const override {
        auto res = new AstUserDefinedFunctor();
        for (auto& cur : args) {
            res->args.emplace_back(cur->clone());
        }
        res->setSrcLoc(getSrcLoc());
        res->setName(getName());
        return res;
    }

protected:
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstUserDefinedFunctor*>(&node));
        const auto& other = static_cast<const AstUserDefinedFunctor&>(node);
        return name == other.name && AstFunctor::equal(node);
    }

    /** name of user-defined functor */
    std::string name;
};

/**
 * Record
 */
class AstRecordInit : public AstTerm {
public:
    void print(std::ostream& os) const override {
        os << "[" << join(args, ",", print_deref<std::unique_ptr<AstArgument>>()) << "]";
    }

    AstRecordInit* clone() const override {
        auto res = new AstRecordInit();
        for (auto& cur : args) {
            res->args.emplace_back(cur->clone());
        }
        res->setSrcLoc(getSrcLoc());
        return res;
    }
};

/**
 * An argument capable of casting a value of one type into another.
 */
class AstTypeCast : public AstArgument {
public:
    AstTypeCast(std::unique_ptr<AstArgument> value, AstTypeIdentifier type)
            : value(std::move(value)), type(std::move(type)) {}

    void print(std::ostream& os) const override {
        os << "as(" << *value << "," << type << ")";
    }

    /** Get value */
    AstArgument* getValue() const {
        return value.get();
    }

    /** Get type */
    const AstTypeIdentifier& getType() const {
        return type;
    }

    /** Set type */
    void setType(const AstTypeIdentifier& type) {
        this->type = type;
    }

    std::vector<const AstNode*> getChildNodes() const override {
        auto res = AstArgument::getChildNodes();
        res.push_back(value.get());
        return res;
    }

    AstTypeCast* clone() const override {
        auto res = new AstTypeCast(std::unique_ptr<AstArgument>(value->clone()), type);
        res->setSrcLoc(getSrcLoc());
        return res;
    }

    void apply(const AstNodeMapper& map) override {
        value = map(std::move(value));
    }

protected:
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstTypeCast*>(&node));
        const auto& other = static_cast<const AstTypeCast&>(node);
        return type == other.type && equal_ptr(value, other.value);
    }

    /** The value to be casted */
    std::unique_ptr<AstArgument> value;

    /** The target type name */
    AstTypeIdentifier type;
};

/**
 * An argument aggregating a value from a sub-query.
 * TODO (b-scholz): fix body literal interface;
 * remove getters/setters for individual literals
 */
class AstAggregator : public AstArgument {
public:
    /**
     * The kind of utilised aggregation operator.
     * Note: lower-case is utilized due to a collision with
     *  constants in the parser.
     */
    enum Op { min, max, count, sum };

    /** Creates a new aggregation node */
    AstAggregator(Op fun) : fun(fun), expression(nullptr) {}

    void print(std::ostream& os) const override {
        switch (fun) {
            case sum:
                os << "sum";
                break;
            case min:
                os << "min";
                break;
            case max:
                os << "max";
                break;
            case count:
                os << "count";
                break;
            default:
                break;
        }
        if (expression) {
            os << " " << *expression;
        }
        os << " : ";
        if (body.size() > 1) {
            os << "{ ";
        }
        os << join(body, ", ", print_deref<std::unique_ptr<AstLiteral>>());
        if (body.size() > 1) {
            os << " }";
        }
    }

    /** Get aggregate operator */
    Op getOperator() const {
        return fun;
    }

    /** Set target expression */
    void setTargetExpression(std::unique_ptr<AstArgument> arg) {
        expression = std::move(arg);
    }

    /** Get target expression */
    const AstArgument* getTargetExpression() const {
        return expression.get();
    }

    /** Get body literals */
    std::vector<AstLiteral*> getBodyLiterals() const {
        return toPtrVector(body);
    }

    /** Clear body literals */
    void clearBodyLiterals() {
        body.clear();
    }

    /** Add body literal */
    void addBodyLiteral(std::unique_ptr<AstLiteral> lit) {
        body.push_back(std::move(lit));
    }

    std::vector<const AstNode*> getChildNodes() const override {
        auto res = AstArgument::getChildNodes();
        if (expression) {
            res.push_back(expression.get());
        }
        for (auto& cur : body) {
            res.push_back(cur.get());
        }
        return res;
    }

    AstAggregator* clone() const override {
        auto res = new AstAggregator(fun);
        res->expression = (expression) ? std::unique_ptr<AstArgument>(expression->clone()) : nullptr;
        for (const auto& cur : body) {
            res->body.emplace_back(cur->clone());
        }
        res->setSrcLoc(getSrcLoc());
        return res;
    }

    void apply(const AstNodeMapper& map) override {
        if (expression) {
            expression = map(std::move(expression));
        }
        for (auto& cur : body) {
            cur = map(std::move(cur));
        }
    }

protected:
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstAggregator*>(&node));
        const auto& other = static_cast<const AstAggregator&>(node);
        return fun == other.fun && equal_ptr(expression, other.expression) && equal_targets(body, other.body);
    }

private:
    /** The aggregation operator of this aggregation step */
    Op fun;

    /** The expression to be aggregated */
    std::unique_ptr<AstArgument> expression;

    /** A list of body-literals forming a sub-query which's result is projected and aggregated */
    std::vector<std::unique_ptr<AstLiteral>> body;
};

/**
 * Subroutine Argument
 */
class AstSubroutineArgument : public AstArgument {
public:
    AstSubroutineArgument(size_t index) : index(index) {}

    void print(std::ostream& os) const override {
        os << "arg_" << index;
    }

    /** Return argument index */
    size_t getNumber() const {
        return index;
    }

    AstSubroutineArgument* clone() const override {
        auto* res = new AstSubroutineArgument(index);
        res->setSrcLoc(getSrcLoc());
        return res;
    }

protected:
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstSubroutineArgument*>(&node));
        const auto& other = static_cast<const AstSubroutineArgument&>(node);
        return index == other.index;
    }

private:
    /** Index of argument in argument list*/
    size_t index;
};

}  // end of namespace souffle
