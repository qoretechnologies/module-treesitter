/* -*- mode: c++; indent-tabs-mode: nil -*- */
/** @file TreeSitterCursor.h TreeSitterCursor class definition */
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

#ifndef _QORE_TREESITTER_CURSOR_H
#define _QORE_TREESITTER_CURSOR_H

#include "treesitter-module.h"
#include "TreeSitterNode.h"

#include <string>

//! Wraps a tree-sitter tree cursor for efficient tree traversal
class TreeSitterCursor : public AbstractPrivateData {
public:
    //! Create a cursor starting at the given node
    DLLLOCAL TreeSitterCursor(TreeSitterNode* node);

    //! Destructor
    DLLLOCAL virtual ~TreeSitterCursor();

    //! Reset the cursor to a new node
    DLLLOCAL void reset(TreeSitterNode* node);

    //! Get the current node
    DLLLOCAL TreeSitterNode* getCurrentNode() const;

    //! Get the field name of the current node (if it's a field child)
    DLLLOCAL const char* getCurrentFieldName() const;

    //! Get the field ID of the current node
    DLLLOCAL TSFieldId getCurrentFieldId() const;

    //! Move to the parent node
    /** @return true if successful, false if already at root */
    DLLLOCAL bool gotoParent();

    //! Move to the next sibling
    /** @return true if successful, false if no next sibling */
    DLLLOCAL bool gotoNextSibling();

    //! Move to the previous sibling
    /** @return true if successful, false if no previous sibling */
    DLLLOCAL bool gotoPrevSibling();

    //! Move to the first child
    /** @return true if successful, false if no children */
    DLLLOCAL bool gotoFirstChild();

    //! Move to the last child
    /** @return true if successful, false if no children */
    DLLLOCAL bool gotoLastChild();

    //! Move to the first child that extends beyond the given byte offset
    /** @param byte_offset the byte offset
        @return the index of the child, or -1 if not found
    */
    DLLLOCAL int64_t gotoFirstChildForByte(uint32_t byte_offset);

    //! Move to the first child that extends beyond the given point
    /** @param point the point (row, column)
        @return the index of the child, or -1 if not found
    */
    DLLLOCAL int64_t gotoFirstChildForPoint(TSPoint point);

    //! Get the depth of the current node in the tree
    DLLLOCAL uint32_t getCurrentDepth() const;

    //! Create a copy of this cursor
    DLLLOCAL TreeSitterCursor* copy() const;

private:
    TSTreeCursor cursor;
    std::string source;

    // Prevent copying (use copy() method instead)
    TreeSitterCursor(const TreeSitterCursor&) = delete;
    TreeSitterCursor& operator=(const TreeSitterCursor&) = delete;
};

#endif // _QORE_TREESITTER_CURSOR_H
