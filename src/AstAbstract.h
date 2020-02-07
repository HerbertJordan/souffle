/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2014, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file AstAbstract.h
 *
 * Abstract class definitions for AST nodes
 *
 ***********************************************************************/

#pragma once

#include "AstNode.h"

namespace souffle {

/**
 * Intermediate representation of atoms, binary relations,
 * and negated atoms in the body and head of a clause.
 */
class AstAtom;

class AstLiteral : public AstNode {
public:
    /** Obtains the atom referenced by this literal - if any */
    /* TODO (b-scholz): revisit the following methods */
    virtual const AstAtom* getAtom() const = 0;

    /** Creates a clone of this AST sub-structure */
    AstLiteral* clone() const override = 0;
};

/**
 * Intermediate representation of an argument
 */
class AstArgument : public AstNode {
public:
    /** Create clone */
    AstArgument* clone() const override = 0;
};

}  // end of namespace souffle
