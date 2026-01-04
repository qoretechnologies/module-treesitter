/* -*- mode: c++; indent-tabs-mode: nil -*- */
/** @file TreeSitterQuery.cpp TreeSitterQuery implementation */
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

#include "TreeSitterQuery.h"

TreeSitterQuery::TreeSitterQuery(const char* lang_name, const char* query_source, ExceptionSink* xsink)
    : query(nullptr), language(nullptr), source(query_source) {

    language = TreeSitterLanguages::getLanguage(lang_name);
    if (!language) {
        raiseTreeSitterException(xsink, "TREESITTER-LANGUAGE-ERROR",
            "Unknown language: '%s'", lang_name);
        return;
    }

    uint32_t error_offset;
    TSQueryError error_type;

    query = ts_query_new(language, query_source, strlen(query_source), &error_offset, &error_type);

    if (!query) {
        const char* error_msg;
        switch (error_type) {
            case TSQueryErrorSyntax:
                error_msg = "syntax error";
                break;
            case TSQueryErrorNodeType:
                error_msg = "invalid node type";
                break;
            case TSQueryErrorField:
                error_msg = "invalid field name";
                break;
            case TSQueryErrorCapture:
                error_msg = "invalid capture name";
                break;
            case TSQueryErrorStructure:
                error_msg = "invalid query structure";
                break;
            case TSQueryErrorLanguage:
                error_msg = "language version mismatch";
                break;
            default:
                error_msg = "unknown error";
                break;
        }
        raiseTreeSitterException(xsink, "TREESITTER-QUERY-ERROR",
            "Query error at offset %u: %s", error_offset, error_msg);
    }
}

TreeSitterQuery::~TreeSitterQuery() {
    if (query) {
        ts_query_delete(query);
    }
}

QoreListNode* TreeSitterQuery::execute(TreeSitterNode* node, ExceptionSink* xsink) {
    if (!query || !node) {
        return new QoreListNode(autoTypeInfo);
    }

    TSQueryCursor* cursor = ts_query_cursor_new();
    ts_query_cursor_exec(cursor, query, node->getNode());

    QoreListNode* matches = new QoreListNode(autoTypeInfo);
    TSQueryMatch match;

    while (ts_query_cursor_next_match(cursor, &match)) {
        QoreHashNode* match_hash = new QoreHashNode(autoTypeInfo);
        match_hash->setKeyValue("pattern_index", static_cast<int64>(match.pattern_index), xsink);

        QoreListNode* captures = new QoreListNode(autoTypeInfo);
        for (uint16_t i = 0; i < match.capture_count; i++) {
            const TSQueryCapture& capture = match.captures[i];

            QoreHashNode* capture_hash = new QoreHashNode(autoTypeInfo);
            uint32_t name_len;
            const char* name = ts_query_capture_name_for_id(query, capture.index, &name_len);
            capture_hash->setKeyValue("name", new QoreStringNode(name, name_len, QCS_UTF8), xsink);

            // Get the node text
            uint32_t start = ts_node_start_byte(capture.node);
            uint32_t end = ts_node_end_byte(capture.node);

            QoreHashNode* node_info = new QoreHashNode(autoTypeInfo);
            node_info->setKeyValue("type", new QoreStringNode(ts_node_type(capture.node)), xsink);
            node_info->setKeyValue("start_byte", static_cast<int64>(start), xsink);
            node_info->setKeyValue("end_byte", static_cast<int64>(end), xsink);

            TSPoint start_point = ts_node_start_point(capture.node);
            TSPoint end_point = ts_node_end_point(capture.node);
            node_info->setKeyValue("start_row", static_cast<int64>(start_point.row), xsink);
            node_info->setKeyValue("start_column", static_cast<int64>(start_point.column), xsink);
            node_info->setKeyValue("end_row", static_cast<int64>(end_point.row), xsink);
            node_info->setKeyValue("end_column", static_cast<int64>(end_point.column), xsink);

            capture_hash->setKeyValue("node", node_info, xsink);
            captures->push(capture_hash, xsink);
        }
        match_hash->setKeyValue("captures", captures, xsink);
        matches->push(match_hash, xsink);
    }

    ts_query_cursor_delete(cursor);
    return matches;
}

QoreListNode* TreeSitterQuery::captures(TreeSitterNode* node, ExceptionSink* xsink) {
    if (!query || !node) {
        return new QoreListNode(autoTypeInfo);
    }

    TSQueryCursor* cursor = ts_query_cursor_new();
    ts_query_cursor_exec(cursor, query, node->getNode());

    QoreListNode* all_captures = new QoreListNode(autoTypeInfo);
    TSQueryMatch match;
    uint32_t capture_index;

    while (ts_query_cursor_next_capture(cursor, &match, &capture_index)) {
        const TSQueryCapture& capture = match.captures[capture_index];

        QoreHashNode* capture_hash = new QoreHashNode(autoTypeInfo);
        uint32_t name_len;
        const char* name = ts_query_capture_name_for_id(query, capture.index, &name_len);
        capture_hash->setKeyValue("name", new QoreStringNode(name, name_len, QCS_UTF8), xsink);
        capture_hash->setKeyValue("pattern_index", static_cast<int64>(match.pattern_index), xsink);

        QoreHashNode* node_info = new QoreHashNode(autoTypeInfo);
        node_info->setKeyValue("type", new QoreStringNode(ts_node_type(capture.node)), xsink);
        node_info->setKeyValue("start_byte", static_cast<int64>(ts_node_start_byte(capture.node)), xsink);
        node_info->setKeyValue("end_byte", static_cast<int64>(ts_node_end_byte(capture.node)), xsink);

        TSPoint start_point = ts_node_start_point(capture.node);
        TSPoint end_point = ts_node_end_point(capture.node);
        node_info->setKeyValue("start_row", static_cast<int64>(start_point.row), xsink);
        node_info->setKeyValue("start_column", static_cast<int64>(start_point.column), xsink);
        node_info->setKeyValue("end_row", static_cast<int64>(end_point.row), xsink);
        node_info->setKeyValue("end_column", static_cast<int64>(end_point.column), xsink);

        capture_hash->setKeyValue("node", node_info, xsink);
        all_captures->push(capture_hash, xsink);
    }

    ts_query_cursor_delete(cursor);
    return all_captures;
}

uint32_t TreeSitterQuery::getPatternCount() const {
    return query ? ts_query_pattern_count(query) : 0;
}

uint32_t TreeSitterQuery::getCaptureCount() const {
    return query ? ts_query_capture_count(query) : 0;
}

uint32_t TreeSitterQuery::getStringCount() const {
    return query ? ts_query_string_count(query) : 0;
}

const char* TreeSitterQuery::getCaptureName(uint32_t index) const {
    if (!query) return nullptr;
    uint32_t len;
    return ts_query_capture_name_for_id(query, index, &len);
}

uint32_t TreeSitterQuery::getPatternStartByte(uint32_t pattern_index) const {
    return query ? ts_query_start_byte_for_pattern(query, pattern_index) : 0;
}

bool TreeSitterQuery::isPatternGuaranteedAtStep(uint32_t pattern_index) const {
    return query ? ts_query_is_pattern_guaranteed_at_step(query, pattern_index) : false;
}

void TreeSitterQuery::disableCapture(const char* name) {
    if (query) {
        ts_query_disable_capture(query, name, strlen(name));
    }
}

void TreeSitterQuery::disablePattern(uint32_t pattern_index) {
    if (query) {
        ts_query_disable_pattern(query, pattern_index);
    }
}

void TreeSitterQuery::setByteRange(uint32_t start, uint32_t end) {
    // Byte range is set on the cursor during execution
    // This would require storing the range and applying it in execute()
}

void TreeSitterQuery::setPointRange(TSPoint start, TSPoint end) {
    // Point range is set on the cursor during execution
}
