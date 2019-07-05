/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2019, The Souffle Developers. All rights reserved.
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file RAMIRelation.h
 *
 * Defines RAMI Relations
 *
 ***********************************************************************/

#pragma once

#include "ParallelUtils.h"
#include "RAMIIndex.h"
#include "RamIndexAnalysis.h"
#include "RamTypes.h"
#include "LVMRelation.h"

#include <deque>
#include <map>
#include <memory>
#include <utility>
#include <vector>

namespace souffle {

// Re-use LVM relation.
using RAMIRelation = LVMRelation;
using RAMIEqRelation = LVMEqRelation;

}  // end of namespace souffle
