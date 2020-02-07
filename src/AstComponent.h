/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file AstComponent.h
 *
 * Defines the class utilized to model a component within the input program.
 *
 ***********************************************************************/

#pragma once

#include "AstClause.h"
#include "AstIO.h"
#include "AstNode.h"
#include "AstRelation.h"
#include "AstType.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace souffle {

/**
 * A component type is the class utilized to represent a construct of the form
 *
 *                  name < Type, Type, ... >
 *
 * where name is the name of the component and < Type, Type, ... > is an optional
 * list of type parameters.
 */
class AstComponentType : public AstNode {
public:
    /**
     * Creates a new component type based on the given name and parameters.
     */
    AstComponentType(
            std::string name = "", std::vector<AstTypeIdentifier> params = std::vector<AstTypeIdentifier>())
            : name(std::move(name)), typeParams(std::move(params)) {}

    void print(std::ostream& os) const override {
        os << name;
        if (!typeParams.empty()) {
            os << "<" << join(typeParams, ",") << ">";
        }
    }

    /** get name */
    const std::string& getName() const {
        return name;
    }

    /** set name */
    void setName(const std::string& n) {
        name = n;
    }

    /** get type parameters */
    const std::vector<AstTypeIdentifier>& getTypeParameters() const {
        return typeParams;
    }

    /** set type parameters */
    void setTypeParameters(const std::vector<AstTypeIdentifier>& params) {
        typeParams = params;
    }

    AstComponentType* clone() const override {
        auto* res = new AstComponentType(name, typeParams);
        res->setSrcLoc(getSrcLoc());
        return res;
    }

protected:
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstComponentType*>(&node));
        const auto& other = static_cast<const AstComponentType&>(node);
        return name == other.name && typeParams == other.typeParams;
    }

private:
    /** The name of the addressed component. */
    std::string name;

    /** The list of associated type parameters. */
    std::vector<AstTypeIdentifier> typeParams;
};

/**
 * A node type representing expressions utilized to initialize components by
 * binding them to a name.
 */
class AstComponentInit : public AstNode {
public:
    void print(std::ostream& os) const override {
        os << ".init " << instanceName << " = ";
        componentType->print(os);
    }

    /** get instance name */
    const std::string& getInstanceName() const {
        return instanceName;
    }

    /** set instance name */
    void setInstanceName(const std::string& name) {
        instanceName = name;
    }

    /** get component type */
    const AstComponentType* getComponentType() const {
        return componentType.get();
    }

    /** set component type */
    void setComponentType(std::unique_ptr<AstComponentType> type) {
        componentType = std::move(type);
    }

    AstComponentInit* clone() const override {
        auto res = new AstComponentInit();
        res->setComponentType(std::unique_ptr<AstComponentType>(componentType->clone()));
        res->setInstanceName(instanceName);
        return res;
    }

    void apply(const AstNodeMapper& mapper) override {
        componentType = mapper(std::move(componentType));
    }

    std::vector<const AstNode*> getChildNodes() const override {
        std::vector<const AstNode*> res;
        res.push_back(componentType.get());
        return res;
    }

protected:
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstComponentInit*>(&node));
        const auto& other = static_cast<const AstComponentInit&>(node);
        return instanceName == other.instanceName && *componentType == *other.componentType;
    }

    /** The name of the resulting component instance. */
    std::string instanceName;

    /** The type of the component to be instantiated. */
    std::unique_ptr<AstComponentType> componentType;
};

/**
 * A AST node describing a component within the input program.
 */
class AstComponent : public AstNode {
public:
    void print(std::ostream& os) const override {
        os << ".comp " << *getComponentType() << " ";

        if (!baseComponents.empty()) {
            os << ": " << join(baseComponents, ",", print_deref<std::unique_ptr<AstComponentType>>()) << " ";
        }
        os << "{\n";

        if (!components.empty()) {
            os << join(components, "\n", print_deref<std::unique_ptr<AstComponent>>()) << "\n";
        }
        if (!instantiations.empty()) {
            os << join(instantiations, "\n", print_deref<std::unique_ptr<AstComponentInit>>()) << "\n";
        }
        if (!types.empty()) {
            os << join(types, "\n", print_deref<std::unique_ptr<AstType>>()) << "\n";
        }
        if (!relations.empty()) {
            os << join(relations, "\n", print_deref<std::unique_ptr<AstRelation>>()) << "\n";
        }
        for (const auto& cur : overrideRules) {
            os << ".override " << cur << "\n";
        }
        if (!clauses.empty()) {
            os << join(clauses, "\n\n", print_deref<std::unique_ptr<AstClause>>()) << "\n";
        }
        if (!loads.empty()) {
            os << join(loads, "\n\n", print_deref<std::unique_ptr<AstLoad>>()) << "\n";
        }
        if (!printSizes.empty()) {
            os << join(printSizes, "\n\n", print_deref<std::unique_ptr<AstPrintSize>>()) << "\n";
        }
        if (!stores.empty()) {
            os << join(stores, "\n\n", print_deref<std::unique_ptr<AstStore>>()) << "\n";
        }

        os << "}\n";
    }

    /** get component type */
    const AstComponentType* getComponentType() const {
        return type.get();
    }

    /** set component type */
    void setComponentType(std::unique_ptr<AstComponentType> other) {
        type = std::move(other);
    }

    /** get base components */
    const std::vector<AstComponentType*> getBaseComponents() const {
        return toPtrVector(baseComponents);
    }

    /** add base components */
    void addBaseComponent(std::unique_ptr<AstComponentType> component) {
        baseComponents.push_back(std::move(component));
    }

    /** add type */
    void addType(std::unique_ptr<AstType> t) {
        types.push_back(std::move(t));
    }

    /** get types */
    std::vector<AstType*> getTypes() const {
        return toPtrVector(types);
    }

    /** copy base components */
    void copyBaseComponents(const AstComponent* other) {
        baseComponents.clear();
        for (const auto& baseComponent : other->getBaseComponents()) {
            baseComponents.emplace_back(baseComponent->clone());
        }
    }

    /** add relation */
    void addRelation(std::unique_ptr<AstRelation> r) {
        relations.push_back(std::move(r));
    }

    /** get relations */
    std::vector<AstRelation*> getRelations() const {
        return toPtrVector(relations);
    }

    /** add clause */
    void addClause(std::unique_ptr<AstClause> c) {
        clauses.push_back(std::move(c));
    }

    /** get clauses */
    std::vector<AstClause*> getClauses() const {
        return toPtrVector(clauses);
    }

    /** add load */
    void addLoad(std::unique_ptr<AstLoad> load) {
        loads.push_back(std::move(load));
    }

    /** add print size */
    void addPrintSize(std::unique_ptr<AstPrintSize> printSize) {
        printSizes.push_back(std::move(printSize));
    }

    /** add store */
    void addStore(std::unique_ptr<AstStore> store) {
        stores.push_back(std::move(store));
    }

    /** get loads */
    std::vector<AstLoad*> getLoads() const {
        return toPtrVector(loads);
    }

    /** get print sizes */
    std::vector<AstPrintSize*> getPrintSizes() const {
        return toPtrVector(printSizes);
    }

    /** get stores */
    std::vector<AstStore*> getStores() const {
        return toPtrVector(stores);
    }

    /** add components */
    void addComponent(std::unique_ptr<AstComponent> c) {
        components.push_back(std::move(c));
    }

    /** get components */
    std::vector<AstComponent*> getComponents() const {
        return toPtrVector(components);
    }

    /** add instantiation */
    void addInstantiation(std::unique_ptr<AstComponentInit> i) {
        instantiations.push_back(std::move(i));
    }

    /** get instantiation */
    std::vector<AstComponentInit*> getInstantiations() const {
        return toPtrVector(instantiations);
    }

    /** add override */
    void addOverride(const std::string& name) {
        overrideRules.insert(name);
    }

    /** get override */
    const std::set<std::string>& getOverridden() const {
        return overrideRules;
    }

    AstComponent* clone() const override {
        auto* res = new AstComponent();
        res->setComponentType(std::unique_ptr<AstComponentType>(type->clone()));

        for (const auto& cur : baseComponents) {
            res->baseComponents.emplace_back(cur->clone());
        }
        for (const auto& cur : components) {
            res->components.emplace_back(cur->clone());
        }
        for (const auto& cur : instantiations) {
            res->instantiations.emplace_back(cur->clone());
        }
        for (const auto& cur : types) {
            res->types.emplace_back(cur->clone());
        }
        for (const auto& cur : relations) {
            res->relations.emplace_back(cur->clone());
        }
        for (const auto& cur : clauses) {
            res->clauses.emplace_back(cur->clone());
        }
        for (const auto& cur : loads) {
            res->loads.emplace_back(cur->clone());
        }
        for (const auto& cur : printSizes) {
            res->printSizes.emplace_back(cur->clone());
        }
        for (const auto& cur : stores) {
            res->stores.emplace_back(cur->clone());
        }
        for (const auto& cur : overrideRules) {
            res->overrideRules.insert(cur);
        }

        return res;
    }

    void apply(const AstNodeMapper& mapper) override {
        type = mapper(std::move(type));
        for (auto& cur : components) {
            cur = mapper(std::move(cur));
        }
        for (auto& cur : baseComponents) {
            cur = mapper(std::move(cur));
        }
        for (auto& cur : instantiations) {
            cur = mapper(std::move(cur));
        }
        for (auto& cur : types) {
            cur = mapper(std::move(cur));
        }
        for (auto& cur : relations) {
            cur = mapper(std::move(cur));
        }
        for (auto& cur : clauses) {
            cur = mapper(std::move(cur));
        }
        for (auto& cur : loads) {
            cur = mapper(std::move(cur));
        }
        for (auto& cur : printSizes) {
            cur = mapper(std::move(cur));
        }
        for (auto& cur : stores) {
            cur = mapper(std::move(cur));
        }
    }

    std::vector<const AstNode*> getChildNodes() const override {
        std::vector<const AstNode*> res;

        res.push_back(type.get());
        for (const auto& cur : components) {
            res.push_back(cur.get());
        }
        for (const auto& cur : baseComponents) {
            res.push_back(cur.get());
        }
        for (const auto& cur : instantiations) {
            res.push_back(cur.get());
        }
        for (const auto& cur : types) {
            res.push_back(cur.get());
        }
        for (const auto& cur : relations) {
            res.push_back(cur.get());
        }
        for (const auto& cur : clauses) {
            res.push_back(cur.get());
        }
        for (const auto& cur : loads) {
            res.push_back(cur.get());
        }
        for (const auto& cur : printSizes) {
            res.push_back(cur.get());
        }
        for (const auto& cur : stores) {
            res.push_back(cur.get());
        }

        return res;
    }

protected:
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstComponent*>(&node));
        const auto& other = static_cast<const AstComponent&>(node);

        // compare all fields
        return *type == *other.type && equal_targets(baseComponents, other.baseComponents) &&
               equal_targets(types, other.types) && equal_targets(relations, other.relations) &&
               equal_targets(clauses, other.clauses) && equal_targets(loads, other.loads) &&
               equal_targets(printSizes, other.printSizes) && equal_targets(stores, other.stores) &&
               equal_targets(components, other.components) &&
               equal_targets(instantiations, other.instantiations);
    }

    /** The type of this component, including its name and type parameters. */
    std::unique_ptr<AstComponentType> type;

    /** A list of base types to inherit relations and clauses from. */
    std::vector<std::unique_ptr<AstComponentType>> baseComponents;

    /** A list of types declared in this component. */
    std::vector<std::unique_ptr<AstType>> types;

    /** A list of relations declared in this component. */
    std::vector<std::unique_ptr<AstRelation>> relations;

    /** A list of clauses defined in this component. */
    std::vector<std::unique_ptr<AstClause>> clauses;

    /** A list of IO directives defined in this component. */
    std::vector<std::unique_ptr<AstLoad>> loads;
    std::vector<std::unique_ptr<AstPrintSize>> printSizes;
    std::vector<std::unique_ptr<AstStore>> stores;

    /** A list of nested components. */
    std::vector<std::unique_ptr<AstComponent>> components;

    /** A list of nested component instantiations. */
    std::vector<std::unique_ptr<AstComponentInit>> instantiations;

    /** Set of relations that are overwritten */
    std::set<std::string> overrideRules;
};

}  // end of namespace souffle
