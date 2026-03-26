/* -*- mode: c++; indent-tabs-mode: nil -*- */
/** @file TreeSitterQueryCursor.h TreeSitterQueryCursor class definition */
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

#ifndef _QORE_TREESITTER_QUERY_CURSOR_H
#define _QORE_TREESITTER_QUERY_CURSOR_H

#include "treesitter-module.h"
#include "TreeSitterQuery.h"
#include "TreeSitterNode.h"

#include <string>
#include <mutex>

//! Wraps a tree-sitter query cursor for reusable query execution
class TreeSitterQueryCursor : public AbstractPrivateData {
public:
    //! Create a query cursor from a query
    DLLLOCAL TreeSitterQueryCursor(TreeSitterQuery* query);

    //! Destructor
    DLLLOCAL virtual ~TreeSitterQueryCursor();

    //! Start executing the query on a node
    DLLLOCAL void exec(TreeSitterNode* node);

    //! Get the next match
    /** @return a match hash, or nullptr if no more matches */
    DLLLOCAL QoreHashNode* nextMatch(ExceptionSink* xsink);

    //! Get the next capture
    /** @return a capture hash, or nullptr if no more captures */
    DLLLOCAL QoreHashNode* nextCapture(ExceptionSink* xsink);

    //! Remove a match by ID (used during capture iteration)
    DLLLOCAL void removeMatch(uint32_t match_id);

    //! Set the byte range for matching
    DLLLOCAL void setByteRange(uint32_t start, uint32_t end);

    //! Set the point range for matching
    DLLLOCAL void setPointRange(TSPoint start, TSPoint end);

    //! Set the maximum start depth
    DLLLOCAL void setMaxStartDepth(uint32_t depth);

    //! Set the match limit
    DLLLOCAL void setMatchLimit(uint32_t limit);

    //! Get the match limit
    DLLLOCAL uint32_t getMatchLimit() const;

    //! Check if the match limit was exceeded
    DLLLOCAL bool didExceedMatchLimit() const;

    //! Clear the byte range restriction
    DLLLOCAL void clearByteRange();

    //! Clear the point range restriction
    DLLLOCAL void clearPointRange();

private:
    TSQueryCursor* cursor;
    TreeSitterQuery* query;    //!< ref-counted reference
    std::string source;        //!< source from last exec() call
    mutable std::mutex mutex;

    // Prevent copying
    TreeSitterQueryCursor(const TreeSitterQueryCursor&) = delete;
    TreeSitterQueryCursor& operator=(const TreeSitterQueryCursor&) = delete;
};

#endif // _QORE_TREESITTER_QUERY_CURSOR_H
