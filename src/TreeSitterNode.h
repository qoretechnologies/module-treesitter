/* -*- mode: c++; indent-tabs-mode: nil -*- */
/** @file TreeSitterNode.h TreeSitterNode class definition */
/*
    Qore Programming Language

    Copyright (C) 2026 Qore Technologies, s.r.o.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _QORE_TREESITTER_NODE_H
#define _QORE_TREESITTER_NODE_H

#include "treesitter-module.h"

#include <string>
#include <vector>

// Forward declaration
class TreeSitterTree;

//! Wraps a tree-sitter node
class TreeSitterNode : public AbstractPrivateData {
public:
    //! Create a node wrapper
    DLLLOCAL TreeSitterNode(TSNode node, const std::string& source);

    //! Destructor
    DLLLOCAL virtual ~TreeSitterNode();

    //! Get the node type (e.g., "function_definition", "identifier")
    DLLLOCAL const char* getType() const;

    //! Get the node type as a symbol ID
    DLLLOCAL TSSymbol getSymbol() const;

    //! Get the start byte position
    DLLLOCAL uint32_t getStartByte() const;

    //! Get the end byte position
    DLLLOCAL uint32_t getEndByte() const;

    //! Get the start position as (row, column)
    DLLLOCAL TSPoint getStartPoint() const;

    //! Get the end position as (row, column)
    DLLLOCAL TSPoint getEndPoint() const;

    //! Get the text content of this node
    DLLLOCAL QoreStringNode* getText() const;

    //! Get the number of children
    DLLLOCAL uint32_t getChildCount() const;

    //! Get a child by index
    DLLLOCAL TreeSitterNode* getChild(uint32_t index) const;

    //! Get a child by field name
    DLLLOCAL TreeSitterNode* getChildByFieldName(const char* field_name) const;

    //! Get the number of named children
    DLLLOCAL uint32_t getNamedChildCount() const;

    //! Get a named child by index
    DLLLOCAL TreeSitterNode* getNamedChild(uint32_t index) const;

    //! Get the parent node
    DLLLOCAL TreeSitterNode* getParent() const;

    //! Get the next sibling
    DLLLOCAL TreeSitterNode* getNextSibling() const;

    //! Get the previous sibling
    DLLLOCAL TreeSitterNode* getPrevSibling() const;

    //! Get the next named sibling
    DLLLOCAL TreeSitterNode* getNextNamedSibling() const;

    //! Get the previous named sibling
    DLLLOCAL TreeSitterNode* getPrevNamedSibling() const;

    //! Check if this is a named node
    DLLLOCAL bool isNamed() const;

    //! Check if this node is missing (error recovery)
    DLLLOCAL bool isMissing() const;

    //! Check if this node has an error
    DLLLOCAL bool hasError() const;

    //! Check if this is an "extra" node (e.g., comments)
    DLLLOCAL bool isExtra() const;

    //! Check if this node is null
    DLLLOCAL bool isNull() const;

    //! Get the field name for this node (if it's a field child)
    DLLLOCAL const char* getFieldName() const;

    //! Get all children as a vector (caller must wrap in QoreObjects)
    DLLLOCAL std::vector<TreeSitterNode*> getChildren() const;

    //! Get all named children as a vector (caller must wrap in QoreObjects)
    DLLLOCAL std::vector<TreeSitterNode*> getNamedChildren() const;

    //! Get the underlying TSNode
    DLLLOCAL TSNode getNode() const { return node; }

    //! Get node info as a hash
    DLLLOCAL QoreHashNode* toHash() const;

    //! Get the S-expression representation
    DLLLOCAL QoreStringNode* toSexp() const;

    //! Find the smallest node that spans the given byte range
    DLLLOCAL TreeSitterNode* getDescendantForByteRange(uint32_t start, uint32_t end) const;

    //! Find the smallest named node that spans the given byte range
    DLLLOCAL TreeSitterNode* getNamedDescendantForByteRange(uint32_t start, uint32_t end) const;

    //! Find the smallest node that spans the given point range
    DLLLOCAL TreeSitterNode* getDescendantForPointRange(TSPoint start, TSPoint end) const;

    //! Find the smallest named node that spans the given point range
    DLLLOCAL TreeSitterNode* getNamedDescendantForPointRange(TSPoint start, TSPoint end) const;

    //! Compare with another node for equality
    DLLLOCAL bool equals(TreeSitterNode* other) const;

    //! Get the full source string
    DLLLOCAL const std::string& getSource() const { return source; }

private:
    TSNode node;
    std::string source;
};

#endif // _QORE_TREESITTER_NODE_H
