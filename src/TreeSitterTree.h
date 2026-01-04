/* -*- mode: c++; indent-tabs-mode: nil -*- */
/** @file TreeSitterTree.h TreeSitterTree class definition */
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

#ifndef _QORE_TREESITTER_TREE_H
#define _QORE_TREESITTER_TREE_H

#include "treesitter-module.h"
#include "TreeSitterNode.h"

#include <string>

//! Wraps a tree-sitter syntax tree
class TreeSitterTree : public AbstractPrivateData {
public:
    //! Create a tree from a TSTree pointer (takes ownership)
    DLLLOCAL TreeSitterTree(TSTree* tree, const std::string& source);

    //! Destructor
    DLLLOCAL virtual ~TreeSitterTree();

    //! Get the root node of the tree
    DLLLOCAL TreeSitterNode* getRootNode() const;

    //! Get the source code
    DLLLOCAL const std::string& getSource() const { return source; }

    //! Get the underlying TSTree
    DLLLOCAL TSTree* getTree() const { return tree; }

    //! Create a copy of the tree
    DLLLOCAL TreeSitterTree* copy() const;

    //! Edit the tree to reflect a change in the source code
    /** @param start_byte the start position of the change
        @param old_end_byte the old end position
        @param new_end_byte the new end position
        @param start_row the start row (0-indexed)
        @param start_column the start column (0-indexed)
        @param old_end_row the old end row
        @param old_end_column the old end column
        @param new_end_row the new end row
        @param new_end_column the new end column
    */
    DLLLOCAL void edit(uint32_t start_byte, uint32_t old_end_byte, uint32_t new_end_byte,
                       uint32_t start_row, uint32_t start_column,
                       uint32_t old_end_row, uint32_t old_end_column,
                       uint32_t new_end_row, uint32_t new_end_column);

    //! Get changed ranges between this tree and another
    /** @param other the other tree to compare with
        @param xsink exception sink
        @return list of changed ranges as hashes
    */
    DLLLOCAL QoreListNode* getChangedRanges(TreeSitterTree* other, ExceptionSink* xsink) const;

    //! Check if the tree is valid
    DLLLOCAL bool isValid() const { return tree != nullptr; }

    //! Get the language of the tree
    DLLLOCAL const TSLanguage* getLanguage() const;

private:
    TSTree* tree;
    std::string source;

    // Prevent copying (use copy() method instead)
    TreeSitterTree(const TreeSitterTree&) = delete;
    TreeSitterTree& operator=(const TreeSitterTree&) = delete;
};

#endif // _QORE_TREESITTER_TREE_H
