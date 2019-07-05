/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2019, The Souffle Developers. All rights reserved.
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file RAMIProgInterface.h
 *
 * Defines classes that implement the SouffleInterface abstract class
 *
 ***********************************************************************/

#pragma once

#include "RAMIInterface.h"
#include "RamVisitor.h"
#include "SouffleInterface.h"

#include <array>
#include <utility>

namespace souffle {

/**
 * Wrapper class for interpreter relations
 */
class RAMIRelInterface : public Relation {
public:
    RAMIRelInterface(RAMIRelation& r, SymbolTable& s, std::string n, std::vector<std::string> t,
            std::vector<std::string> an, uint32_t i)
            : relation(r), symTable(s), name(std::move(n)), types(std::move(t)), attrNames(std::move(an)),
              id(i) {}
    ~RAMIRelInterface() override = default;

    /** Insert tuple */
    void insert(const tuple& t) override {
        relation.insert(t.data);
    }

    /** Check whether tuple exists */
    bool contains(const tuple& t) const override {
        return relation.contains(TupleRef(&t.data[0], t.size()));
    }

    /** Iterator to first tuple */
    iterator begin() const override {
    	assert(false && "Not supported.");
        return RAMIRelInterface::iterator(new RAMIRelInterface::iterator_base(id, this, nullptr));
    }

    /** Iterator to last tuple */
    iterator end() const override {
    	assert(false && "Not supported.");
        return RAMIRelInterface::iterator(new RAMIRelInterface::iterator_base(id, this, nullptr));
    }

    /** Get name */
    std::string getName() const override {
        return name;
    }

    /** Get arity */
    size_t getArity() const override {
        return relation.getArity();
    }

    /** Get symbol table */
    SymbolTable& getSymbolTable() const override {
        return symTable;
    }

    /** Get attribute type */
    const char* getAttrType(size_t idx) const override {
        assert(idx < getArity() && "exceeded tuple size");
        return types[idx].c_str();
    }

    /** Get attribute name */
    const char* getAttrName(size_t idx) const override {
        assert(idx < getArity() && "exceeded tuple size");
        return attrNames[idx].c_str();
    }

    /** Get number of tuples in relation */
    std::size_t size() const override {
        return relation.size();
    }

    /** Eliminate all the tuples in relation*/
    void purge() override {
        relation.purge();
    }

protected:
    /**
     * Iterator wrapper class
     */
    class iterator_base : public Relation::iterator_base {
    public:
        iterator_base(uint32_t arg_id, const RAMIRelInterface* r, int* i)
                : Relation::iterator_base(arg_id), ramRelationInterface(r), it(i), tup(r) {}
        ~iterator_base() override = default;

        /** Increment iterator */
        void operator++() override {
            ++it;
        }

        /** Get current tuple */
        tuple& operator*() override {
            // set tuple stream to first element
            tup.rewind();

            // construct the tuple to return
            for (size_t i = 0; i < ramRelationInterface->getArity(); i++) {
				assert(false && "Not implemented.");
                if (*(ramRelationInterface->getAttrType(i)) == 's') {
//                    std::string s = ramRelationInterface->getSymbolTable().resolve((*it)[i]);
//                    tup << s;
                } else {
//                    tup << (*it)[i];
                }
            }
            tup.rewind();
            return tup;
        }

        /** Clone iterator */
        iterator_base* clone() const override {
            return new RAMIRelInterface::iterator_base(getId(), ramRelationInterface, it);
        }

    protected:
        /** Check equivalence */
        bool equal(const Relation::iterator_base& o) const override {
            try {
                auto iter = dynamic_cast<const RAMIRelInterface::iterator_base&>(o);
                return ramRelationInterface == iter.ramRelationInterface && it == iter.it;
            } catch (const std::bad_cast& e) {
                return false;
            }
        }

    private:
        const RAMIRelInterface* ramRelationInterface;
        int* it;
        tuple tup;
    };

private:
    /** Wrapped interpreter relation */
    RAMIRelation& relation;

    /** Symbol table */
    SymbolTable& symTable;

    /** Name of relation */
    std::string name;

    /** Attribute type */
    std::vector<std::string> types;

    /** Attribute Names */
    std::vector<std::string> attrNames;

    /** Unique id for wrapper */
    uint32_t id;
};

/**
 * Implementation of SouffleProgram interface for an interpreter instance
 */
class RAMIProgInterface : public SouffleProgram {
public:
    RAMIProgInterface(RAMIInterface& interp)
            : prog(*interp.getTranslationUnit().getProgram()), exec(interp),
              symTable(interp.getTranslationUnit().getSymbolTable()) {
        uint32_t id = 0;

        // Retrieve AST Relations and store them in a map
        std::map<std::string, const RamRelation*> map;
        visitDepthFirst(prog, [&](const RamRelation& rel) { map[rel.getName()] = &rel; });

        // Build wrapper relations for Souffle's interface
        for (auto& rel_pair : exec.getRelationMap()) {
            auto& name = rel_pair.first;
            auto& interpreterRel = **rel_pair.second;
            assert(map[name]);
            const RamRelation& rel = *map[name];

            // construct types and names vectors
            std::vector<std::string> types;
            std::vector<std::string> attrNames;
            for (size_t i = 0; i < rel.getArity(); i++) {
                std::string t = rel.getArgTypeQualifier(i);
                types.push_back(t);

                std::string n = rel.getArg(i);
                attrNames.push_back(n);
            }
            auto* interface =
                    new RAMIRelInterface(interpreterRel, symTable, rel.getName(), types, attrNames, id);
            interfaces.push_back(interface);
            bool input;
            bool output;
            visitDepthFirst(prog, [&](const RamStore& store) {
                if (store.getRelation() == rel) {
                    output = true;
                }
            });
            visitDepthFirst(prog, [&](const RamLoad& load) {
                if (load.getRelation() == rel) {
                    input = true;
                }
            });
            addRelation(rel.getName(), interface, input, output);
            id++;
        }
    }
    ~RAMIProgInterface() override {
        for (auto* interface : interfaces) {
            delete interface;
        }
    }

    /** Run program instance: not implemented */
    void run(size_t) override {}

    /** Load data, run program instance, store data: not implemented */
    void runAll(std::string, std::string, size_t) override {}

    /** Load input data: not implemented */
    void loadAll(std::string) override {}

    /** Print output data: not implemented */
    void printAll(std::string) override {}

    /** Dump inputs: not implemented */
    void dumpInputs(std::ostream&) override {}

    /** Dump outputs: not implemented */
    void dumpOutputs(std::ostream&) override {}

    /** Run subroutine */
    void executeSubroutine(std::string name, const std::vector<RamDomain>& args, std::vector<RamDomain>& ret,
            std::vector<bool>& err) override {
        exec.executeSubroutine(name, args, ret, err);
    }

    /** Get symbol table */
    SymbolTable& getSymbolTable() override {
        return symTable;
    }

private:
    const RamProgram& prog;
    RAMIInterface& exec;
    SymbolTable& symTable;
    std::vector<RAMIRelInterface*> interfaces;
};

}  // end of namespace souffle
