/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2019, The Souffle Developers. All rights reserved.
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file RAMI.h
 *
 * Declares the RAMI (RamInterpreter) class for executing RAM programs.
 *
 ***********************************************************************/

#pragma once

#include "RAMIContext.h"
#include "RAMIInterface.h"
#include "RAMIRelation.h"
#include "RamCondition.h"
#include "RamRelation.h"
#include "RamStatement.h"
#include "RamTranslationUnit.h"
#include "RamTypes.h"
#include "RelationRepresentation.h"

#include <atomic>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <dlfcn.h>

namespace souffle {

class RMAIProgInterface;
class RamOperation;
class RamExpression;
class SymbolTable;

/**
 * Interpreter executing a RAM translation unit
 */

class RAMI : public RAMIInterface {
public:
    RAMI(RamTranslationUnit& tUnit) : RAMIInterface(tUnit), profiling_enabled(Global::config().has("profile")) {}
    ~RAMI() {
        for (auto& x : environment) {
            delete x.second;
        }
    }

    /** Execute main program */
    void executeMain() override;

    /* Execute subroutine */
    void executeSubroutine(const std::string& name, const std::vector<RamDomain>& arguments,
            std::vector<RamDomain>& returnValues, std::vector<bool>& returnErrors) override;

protected:

    /** Evaluate value */
    RamDomain evalExpr(const RamExpression& value, const RAMIContext& ctxt = RAMIContext());

    /** Evaluate operation */
    void evalOp(const RamOperation& op, const RAMIContext& ctxt = RAMIContext());

    /** Evaluate conditions */
    bool evalCond(const RamCondition& cond, const RAMIContext& ctxt = RAMIContext());

    /** Evaluate statement */
    void evalStmt(const RamStatement& stmt, const RAMIContext& ctxt = RAMIContext());

    /** Get symbol table */
    SymbolTable& getSymbolTable() {
        return translationUnit.getSymbolTable();
    }

    /** Get counter */
    int getCounter() const {
        return counter;
    }

    /** Get Iteration Number */
    size_t getIterationNumber() const {
        return iteration;
    }

    /** Increment counter */
    int incCounter() {
        return counter++;
    }

    /** Increment iteration number */
    void incIterationNumber() {
        iteration++;
    }

    /** Reset iteration number */
    void resetIterationNumber() {
        iteration = 0;
    }

    void createRelation(const RamRelation& id, const MinIndexSelection* orderSet) {
        RelationHandle res;
        assert(environment.find(id.getName()) == environment.end());
        if (id.getRepresentation() == RelationRepresentation::EQREL) {
        	res = std::make_unique<RAMIEqRelation>(id.getArity(), id.getName(), std::vector<std::string>(), *orderSet);
        } else {
        	res = std::make_unique<RAMIRelation>(id.getArity(), id.getName(), std::vector<std::string>(), *orderSet);
        }
        environment[id.getName()] = new RelationHandle(std::move(res));
    }

private:
    /** Get relation */
    RelationHandle& getRelationHandle(const std::string& name) {
        // look up relation
        auto pos = environment.find(name);
        assert(pos != environment.end());
        return *pos->second;
    }
public:

    /** Get relation */
    inline RAMIRelation& getRelation(const RamRelation& id) {
    	if (id.relation) return **static_cast<RelationHandle*>(id.relation);
        auto& handle = getRelationHandle(id.getName());
        id.relation = &handle;
        return *handle;
    }

    /** Drop relation */
    void dropRelation(const RamRelation& id) {
        RAMIRelation& rel = getRelation(id);
        environment.erase(id.getName());
        delete &rel;
    }

    /** Swap relation */
    void swapRelation(const RamRelation& ramRel1, const RamRelation& ramRel2) {
        RelationHandle& rel1 = getRelationHandle(ramRel1.getName());
        RelationHandle& rel2 = getRelationHandle(ramRel2.getName());
        std::swap(rel1,rel2);
    }

private:
    friend RAMIProgInterface;

    /** relation environment type */
    using relation_map = std::map<std::string, RelationHandle*>;

    /** Get relation map */
    virtual std::map<std::string, RelationHandle*>& getRelationMap() override {
        return environment;
    }

    /** counters for atom profiling */
    std::map<std::string, std::map<size_t, size_t>> frequencies;

    /** counters for non-existence checks */
    std::map<std::string, std::atomic<size_t>> reads;

    /** counter for $ operator */
    int counter = 0;

    /** iteration number (in a fix-point calculation) */
    size_t iteration = 0;

    /** Relation Environment */
    relation_map environment;

    bool profiling_enabled;
};

}  // end of namespace souffle
