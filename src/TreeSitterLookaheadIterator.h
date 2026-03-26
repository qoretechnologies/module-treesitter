/* -*- mode: c++; indent-tabs-mode: nil -*- */
/** @file TreeSitterLookaheadIterator.h TreeSitterLookaheadIterator class definition */
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

#ifndef _QORE_TREESITTER_LOOKAHEAD_ITERATOR_H
#define _QORE_TREESITTER_LOOKAHEAD_ITERATOR_H

#include "treesitter-module.h"

#include <mutex>

//! Wraps a tree-sitter lookahead iterator for code completion
class TreeSitterLookaheadIterator : public AbstractPrivateData {
public:
    //! Create a lookahead iterator from a language and parse state
    DLLLOCAL TreeSitterLookaheadIterator(const char* language, TSStateId state, ExceptionSink* xsink);

    //! Destructor
    DLLLOCAL virtual ~TreeSitterLookaheadIterator();

    //! Reset to a new language and state
    DLLLOCAL bool reset(const char* language, TSStateId state, ExceptionSink* xsink);

    //! Reset to a new state (same language)
    DLLLOCAL bool resetState(TSStateId state);

    //! Advance to the next symbol
    DLLLOCAL bool next();

    //! Get the current symbol ID
    DLLLOCAL TSSymbol getCurrentSymbol() const;

    //! Get the current symbol name
    DLLLOCAL const char* getCurrentSymbolName() const;

    //! Check if the iterator is valid
    DLLLOCAL bool isValid() const { return iterator != nullptr; }

private:
    TSLookaheadIterator* iterator;
    mutable std::mutex mutex;

    // Prevent copying
    TreeSitterLookaheadIterator(const TreeSitterLookaheadIterator&) = delete;
    TreeSitterLookaheadIterator& operator=(const TreeSitterLookaheadIterator&) = delete;
};

#endif // _QORE_TREESITTER_LOOKAHEAD_ITERATOR_H
