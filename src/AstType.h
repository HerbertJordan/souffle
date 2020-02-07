/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2014, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file AstType.h
 *
 * Defines a type, i.e., disjoint supersets of the universe
 *
 ***********************************************************************/

#pragma once

#include "AstNode.h"
#include "RamTypes.h"

#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace souffle {

/**
 * The type of identifier utilized for referencing types. Type
 * name identifiers are hierarchically qualified names, e.g.
 *
 *          problem.graph.edge
 *
 * TODO (b-scholz): merge with AstNameIdentifier??
 *                  put it into own header file
 *
 */
class AstTypeIdentifier {
public:
    // -- constructors --

    AstTypeIdentifier() : names() {}

    AstTypeIdentifier(const std::string& name) : names({name}) {}

    AstTypeIdentifier(const char* name) : AstTypeIdentifier(std::string(name)) {}

    AstTypeIdentifier(const std::vector<std::string> names) : names(names) {}

    AstTypeIdentifier(const AstTypeIdentifier&) = default;
    AstTypeIdentifier(AstTypeIdentifier&&) = default;

    // -- assignment operators --

    AstTypeIdentifier& operator=(const AstTypeIdentifier&) = default;
    AstTypeIdentifier& operator=(AstTypeIdentifier&&) = default;

    // -- mutators --

    void append(const std::string& name) {
        names.push_back(name);
    }

    void prepend(const std::string& name) {
        names.insert(names.begin(), name);
    }

    // -- getters and setters --

    bool empty() const {
        return names.empty();
    }

    const std::vector<std::string>& getNames() const {
        return names;
    }

    // -- comparison operators --

    bool operator==(const AstTypeIdentifier& other) const {
        return names == other.names;
    }

    bool operator!=(const AstTypeIdentifier& other) const {
        return !(*this == other);
    }

    bool operator<(const AstTypeIdentifier& other) const {
        return std::lexicographical_compare(
                names.begin(), names.end(), other.names.begin(), other.names.end());
    }

    void print(std::ostream& out) const {
        out << join(names, ".");
    }

    friend std::ostream& operator<<(std::ostream& out, const AstTypeIdentifier& id) {
        id.print(out);
        return out;
    }

private:
    /** The list of names forming this identifier. */
    std::vector<std::string> names;
};

/**
 * A overloaded operator to add a new prefix to a given relation identifier.
 */
inline AstTypeIdentifier operator+(const std::string& name, const AstTypeIdentifier& id) {
    AstTypeIdentifier res = id;
    res.prepend(name);
    return res;
}

/**
 *  @class Type
 *  @brief An abstract base class for types within the AST.
 *
 *  TODO (b-scholz): Move to AstAbstract.h
 */
class AstType : public AstNode {
public:
    AstType(AstTypeIdentifier name = {""}) : name(std::move(name)) {}

    /** get type name */
    const AstTypeIdentifier& getName() const {
        return name;
    }

    /** set type name */
    void setName(const AstTypeIdentifier& name) {
        this->name = name;
    }

    AstType* clone() const override = 0;

private:
    /** type name */
    AstTypeIdentifier name;
};

/**
 * A primitive type is named type that can either be a sub-type of
 * the build-in number or symbol type. Primitive types are the most
 * basic building blocks of souffle's type system.
 */
class AstPrimitiveType : public AstType {
public:
    /** Creates a new primitive type */
    AstPrimitiveType(const AstTypeIdentifier& name, RamTypeAttribute type) : AstType(name), type(type) {}

    /** Prints a summary of this type to the given stream */
    void print(std::ostream& os) const override {
        os << ".type " << getName() << (type == RamTypeAttribute::Signed ? "= number" : "");
    }

    /** Tests whether this type is a numeric type */
    bool isNumeric() const {
        return isNumericType(type);
    }

    /** Tests whether this type is a symbolic type */
    bool isSymbolic() const {
        return type == RamTypeAttribute::Symbol;
    }

    AstPrimitiveType* clone() const override {
        auto res = new AstPrimitiveType(getName(), type);
        res->setSrcLoc(getSrcLoc());
        return res;
    }

protected:
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstPrimitiveType*>(&node));
        const auto& other = static_cast<const AstPrimitiveType&>(node);
        return getName() == other.getName() && type == other.type;
    }

private:
    /** type attribute */
    RamTypeAttribute type;
};

/**
 * A union type combines multiple types into a new super type.
 * Each of the enumerated types become a sub-type of the new
 * union type.
 */
class AstUnionType : public AstType {
public:
    void print(std::ostream& os) const override {
        os << ".type " << getName() << " = " << join(types, " | ");
    }

    /** Obtains a reference to the list element types */
    const std::vector<AstTypeIdentifier>& getTypes() const {
        return types;
    }

    /** Adds another element type */
    void add(const AstTypeIdentifier& type) {
        types.push_back(type);
    }

    /** Set variant type */
    void setVariantType(size_t idx, const AstTypeIdentifier& type) {
        assert(idx < types.size() && "union variant index out of bounds");
        types[idx] = type;
    }

    AstUnionType* clone() const override {
        auto res = new AstUnionType();
        res->setSrcLoc(getSrcLoc());
        res->setName(getName());
        res->types = types;
        return res;
    }

protected:
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstUnionType*>(&node));
        const auto& other = static_cast<const AstUnionType&>(node);
        return getName() == other.getName() && types == other.types;
    }

private:
    /** The list of types aggregated by this union type */
    std::vector<AstTypeIdentifier> types;
};

/**
 * A record type aggregates a list of fields into a new type.
 * Each record type has a name making it unique. Two record
 * types are unrelated to all other types (they do not have
 * any super or sub types).
 */
class AstRecordType : public AstType {
public:
    /** record field */
    struct Field {
        std::string name;        // < the field name
        AstTypeIdentifier type;  // < the field type

        bool operator==(const Field& other) const {
            return this == &other || (name == other.name && type == other.type);
        }
    };

    void print(std::ostream& os) const override {
        os << ".type " << getName() << " = "
           << "[";
        for (unsigned i = 0; i < fields.size(); i++) {
            if (i != 0) {
                os << ",";
            }
            os << fields[i].name;
            os << ":";
            os << fields[i].type;
        }
        os << "]";
    }

    /** add field to record type */
    void add(const std::string& name, const AstTypeIdentifier& type) {
        fields.push_back(Field({name, type}));
    }

    /** get fields of record */
    const std::vector<Field>& getFields() const {
        return fields;
    }

    /** set field type */
    void setFieldType(size_t idx, const AstTypeIdentifier& type) {
        assert(idx < fields.size() && "record field index out of bounds");
        fields[idx].type = type;
    }

    AstRecordType* clone() const override {
        auto res = new AstRecordType();
        res->setSrcLoc(getSrcLoc());
        res->setName(getName());
        res->fields = fields;
        return res;
    }

protected:
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstRecordType*>(&node));
        const auto& other = static_cast<const AstRecordType&>(node);
        return getName() == other.getName() && fields == other.fields;
    }

private:
    /** record fields */
    std::vector<Field> fields;
};

}  // end of namespace souffle
