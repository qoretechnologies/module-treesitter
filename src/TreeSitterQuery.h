/* -*- mode: c++; indent-tabs-mode: nil -*- */
/** @file TreeSitterQuery.h TreeSitterQuery class definition */
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

#ifndef _QORE_TREESITTER_QUERY_H
#define _QORE_TREESITTER_QUERY_H

#include "treesitter-module.h"
#include "TreeSitterNode.h"

#include <string>
#include <vector>
#include <regex>
#include <unordered_map>

//! Wraps a tree-sitter query for pattern matching
class TreeSitterQuery : public AbstractPrivateData {
public:
    //! Create a query for the given language
    /** @param language the language name
        @param query_source the S-expression query pattern
        @param xsink exception sink for error reporting
    */
    DLLLOCAL TreeSitterQuery(const char* language, const char* query_source, ExceptionSink* xsink);

    //! Destructor
    DLLLOCAL virtual ~TreeSitterQuery();

    //! Execute the query on a node
    /** @param node the node to search
        @param xsink exception sink
        @return list of matches, each containing capture information
    */
    DLLLOCAL QoreListNode* execute(TreeSitterNode* node, ExceptionSink* xsink);

    //! Execute the query and return all captures
    /** @param node the node to search
        @param xsink exception sink
        @return list of captures with node and capture name
    */
    DLLLOCAL QoreListNode* captures(TreeSitterNode* node, ExceptionSink* xsink);

    //! Get the number of patterns in the query
    DLLLOCAL uint32_t getPatternCount() const;

    //! Get the number of capture names in the query
    DLLLOCAL uint32_t getCaptureCount() const;

    //! Get the number of string literals in the query
    DLLLOCAL uint32_t getStringCount() const;

    //! Get a capture name by index
    DLLLOCAL const char* getCaptureName(uint32_t index) const;

    //! Get the start byte for a pattern
    DLLLOCAL uint32_t getPatternStartByte(uint32_t pattern_index) const;

    //! Check if a pattern is guaranteed to match at most once
    DLLLOCAL bool isPatternGuaranteedAtStep(uint32_t pattern_index) const;

    //! Check if the query is valid
    DLLLOCAL bool isValid() const { return query != nullptr; }

    //! Get the underlying TSQuery
    DLLLOCAL TSQuery* getQuery() const { return query; }

    //! Disable a capture by name
    DLLLOCAL void disableCapture(const char* name);

    //! Disable a pattern by index
    DLLLOCAL void disablePattern(uint32_t pattern_index);

    //! Set the byte range for matching
    DLLLOCAL void setByteRange(uint32_t start, uint32_t end);

    //! Set the point range for matching
    DLLLOCAL void setPointRange(TSPoint start, TSPoint end);

    //! Set the maximum start depth for query execution
    DLLLOCAL void setMaxStartDepth(uint32_t depth);

    //! Clear the byte range restriction
    DLLLOCAL void clearByteRange();

    //! Clear the point range restriction
    DLLLOCAL void clearPointRange();

private:
    TSQuery* query;
    const TSLanguage* language;
    std::string source;

    // Range restrictions
    bool has_byte_range = false;
    uint32_t byte_range_start = 0;
    uint32_t byte_range_end = 0;

    bool has_point_range = false;
    TSPoint point_range_start = {0, 0};
    TSPoint point_range_end = {0, 0};

    bool has_max_start_depth = false;
    uint32_t max_start_depth = UINT32_MAX;

    // Compiled regex cache for #match? predicates
    mutable std::unordered_map<std::string, std::regex> regex_cache;
    mutable std::mutex regex_cache_mutex;

    //! Apply stored cursor settings to a query cursor
    DLLLOCAL void applyCursorSettings(TSQueryCursor* cursor) const;

    //! Evaluate predicates for a pattern match
    /** @return true if all predicates pass (match is valid) */
    DLLLOCAL bool evaluatePredicates(uint32_t pattern_index, const TSQueryMatch& match,
                                      const std::string& src) const;

    //! Get the text of a node from the source string
    DLLLOCAL static std::string getNodeText(TSNode node, const std::string& src);

    // Prevent copying
    TreeSitterQuery(const TreeSitterQuery&) = delete;
    TreeSitterQuery& operator=(const TreeSitterQuery&) = delete;
};

#endif // _QORE_TREESITTER_QUERY_H
