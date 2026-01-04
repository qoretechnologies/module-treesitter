/* -*- mode: c++; indent-tabs-mode: nil -*- */
/** @file TreeSitterParser.h TreeSitterParser class definition */
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

#ifndef _QORE_TREESITTER_PARSER_H
#define _QORE_TREESITTER_PARSER_H

#include "treesitter-module.h"

#include <string>
#include <mutex>

//! Wraps a tree-sitter parser for a specific language
class TreeSitterParser : public AbstractPrivateData {
public:
    //! Create a parser for the given language
    /** @param language the language name (e.g., "python", "java", "json")
        @param xsink exception sink for error reporting
    */
    DLLLOCAL TreeSitterParser(const char* language, ExceptionSink* xsink);

    //! Destructor
    DLLLOCAL virtual ~TreeSitterParser();

    //! Parse source code and return the syntax tree
    /** @param source the source code to parse
        @param xsink exception sink for error reporting
        @return the parsed tree or nullptr on error
    */
    DLLLOCAL TreeSitterTree* parse(const QoreStringNode* source, ExceptionSink* xsink);

    //! Parse source code incrementally, reusing the old tree
    /** @param source the new source code
        @param old_tree the previous parse tree
        @param xsink exception sink for error reporting
        @return the new parsed tree or nullptr on error
    */
    DLLLOCAL TreeSitterTree* parseIncremental(const QoreStringNode* source,
                                               TreeSitterTree* old_tree,
                                               ExceptionSink* xsink);

    //! Get the language name
    DLLLOCAL const std::string& getLanguageName() const { return language_name; }

    //! Get the underlying TSParser
    DLLLOCAL TSParser* getParser() const { return parser; }

    //! Set timeout in microseconds (0 = no timeout)
    DLLLOCAL void setTimeout(uint64_t timeout_micros);

    //! Get the current timeout
    DLLLOCAL uint64_t getTimeout() const;

    //! Check if parser is valid
    DLLLOCAL bool isValid() const { return parser != nullptr; }

private:
    TSParser* parser;
    std::string language_name;
    mutable std::mutex mutex;
    uint64_t timeout_micros = 0;  //!< Stored but not actively enforced in v0.26+

    // Prevent copying
    TreeSitterParser(const TreeSitterParser&) = delete;
    TreeSitterParser& operator=(const TreeSitterParser&) = delete;
};

#endif // _QORE_TREESITTER_PARSER_H
