/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2020, The Souffle Developers. All rights reserved.
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file AuxArityAnalysis.h
 *
 * Define of AST analyses classes
 *
 ***********************************************************************/

#pragma once

#include "AstAnalysis.h"
#include "AstLiteral.h"
#include "AstProgram.h"
#include "AstTranslationUnit.h"

namespace souffle {
class AstRelation;

/**
 * Determine the auxiliary arity for relations
 */
class AuxiliaryArity : public AstAnalysis {
public:
    static constexpr const char* name = "auxiliary-arity";

    void run(const AstTranslationUnit& translationUnit) override {
        program = translationUnit.getProgram();
    }

    /**
     * Returns the number of auxiliary parameters of an atom's relation
     * @param atom the atom to report on
     * @return number of auxiliary attributes
     */
    const size_t getArity(const AstAtom* atom) const {
        return computeArity(program->getRelation(atom->getName()));
    }

    /**
     * Returns the number of auxiliary parameters of a relation
     * @param relation the relation to report on
     * @return number of auxiliary attributes
     */
    const size_t getArity(const AstRelation* relation) const {
        return computeArity(relation);
    }

private:
    /**
     * Returns the number of auxiliary parameters of a relation
     * @param relation the relation to report on
     * @return number of auxiliary attributes
     */
    const size_t computeArity(const AstRelation* relation) const;

    const AstProgram* program;
};

}  // end of namespace souffle
