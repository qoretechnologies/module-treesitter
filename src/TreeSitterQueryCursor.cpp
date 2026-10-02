/* -*- mode: c++; indent-tabs-mode: nil -*- */
/** @file TreeSitterQueryCursor.cpp TreeSitterQueryCursor implementation */
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

#include "TreeSitterQueryCursor.h"
#include "TreeSitterTree.h"

TreeSitterQueryCursor::TreeSitterQueryCursor(TreeSitterQuery* query)
    : cursor(ts_query_cursor_new()), query(query) {
    query->ref();
}

TreeSitterQueryCursor::~TreeSitterQueryCursor() {
    if (cursor) {
        ts_query_cursor_delete(cursor);
    }
    query->deref();
}

void TreeSitterQueryCursor::exec(TreeSitterNode* node) {
    std::lock_guard<std::mutex> lock(mutex);
    ts_query_cursor_exec(cursor, query->getQuery(), node->getNode());
    tree = node->getTreeOwner();
}

QoreHashNode* TreeSitterQueryCursor::nextMatch(ExceptionSink* xsink) {
    std::lock_guard<std::mutex> lock(mutex);
    TSQueryMatch match;
    unsigned int checked = 0;

    while (ts_query_cursor_next_match(cursor, &match)) {
        if (!(checked++ % 100) && qore_check_cancel(xsink, "iterating tree-sitter query results")) {
            return nullptr;
        }
        if (!query->evaluatePredicates(match.pattern_index, match, tree->getSource())) {
            continue;
        }

        ReferenceHolder<QoreHashNode> match_hash(new QoreHashNode(autoTypeInfo), xsink);
        match_hash->setKeyValue("pattern_index", static_cast<int64>(match.pattern_index), xsink);
        if (*xsink) {
            return nullptr;
        }

        ReferenceHolder<QoreListNode> captures(new QoreListNode(autoTypeInfo), xsink);
        for (uint16_t i = 0; i < match.capture_count; i++) {
            if (!(i % 100) && qore_check_cancel(xsink, "collecting tree-sitter query captures")) {
                return nullptr;
            }
            const TSQueryCapture& capture = match.captures[i];

            ReferenceHolder<QoreHashNode> capture_hash(new QoreHashNode(autoTypeInfo), xsink);
            uint32_t name_len;
            const char* name = ts_query_capture_name_for_id(query->getQuery(), capture.index, &name_len);
            capture_hash->setKeyValue("name", new QoreStringNode(name, name_len, QCS_UTF8), xsink);
            if (*xsink) {
                return nullptr;
            }
            capture_hash->setKeyValue("node", TreeSitterNode::buildNodeInfo(capture.node, tree->getSource(), xsink), xsink);
            if (*xsink) {
                return nullptr;
            }
            captures->push(capture_hash.release(), xsink);
            if (*xsink) {
                return nullptr;
            }
        }
        match_hash->setKeyValue("captures", captures.release(), xsink);
        if (*xsink) {
            return nullptr;
        }
        return match_hash.release();
    }

    return nullptr;
}

QoreHashNode* TreeSitterQueryCursor::nextCapture(ExceptionSink* xsink) {
    std::lock_guard<std::mutex> lock(mutex);
    TSQueryMatch match;
    unsigned int checked = 0;
    uint32_t capture_index;

    while (ts_query_cursor_next_capture(cursor, &match, &capture_index)) {
        if (!(checked++ % 100) && qore_check_cancel(xsink, "iterating tree-sitter query results")) {
            return nullptr;
        }
        if (!query->evaluatePredicates(match.pattern_index, match, tree->getSource())) {
            ts_query_cursor_remove_match(cursor, match.id);
            continue;
        }

        const TSQueryCapture& capture = match.captures[capture_index];

        ReferenceHolder<QoreHashNode> capture_hash(new QoreHashNode(autoTypeInfo), xsink);
        uint32_t name_len;
        const char* name = ts_query_capture_name_for_id(query->getQuery(), capture.index, &name_len);
        capture_hash->setKeyValue("name", new QoreStringNode(name, name_len, QCS_UTF8), xsink);
        if (*xsink) {
            return nullptr;
        }
        capture_hash->setKeyValue("pattern_index", static_cast<int64>(match.pattern_index), xsink);
        if (*xsink) {
            return nullptr;
        }
        capture_hash->setKeyValue("node", TreeSitterNode::buildNodeInfo(capture.node, tree->getSource(), xsink), xsink);
        if (*xsink) {
            return nullptr;
        }
        return capture_hash.release();
    }

    return nullptr;
}

void TreeSitterQueryCursor::removeMatch(uint32_t match_id) {
    std::lock_guard<std::mutex> lock(mutex);
    ts_query_cursor_remove_match(cursor, match_id);
}

void TreeSitterQueryCursor::setByteRange(uint32_t start, uint32_t end) {
    std::lock_guard<std::mutex> lock(mutex);
    ts_query_cursor_set_byte_range(cursor, start, end);
}

void TreeSitterQueryCursor::setPointRange(TSPoint start, TSPoint end) {
    std::lock_guard<std::mutex> lock(mutex);
    ts_query_cursor_set_point_range(cursor, start, end);
}

void TreeSitterQueryCursor::setMaxStartDepth(uint32_t depth) {
    std::lock_guard<std::mutex> lock(mutex);
    ts_query_cursor_set_max_start_depth(cursor, depth);
}

void TreeSitterQueryCursor::setMatchLimit(uint32_t limit) {
    std::lock_guard<std::mutex> lock(mutex);
    ts_query_cursor_set_match_limit(cursor, limit);
}

uint32_t TreeSitterQueryCursor::getMatchLimit() const {
    std::lock_guard<std::mutex> lock(mutex);
    return ts_query_cursor_match_limit(cursor);
}

bool TreeSitterQueryCursor::didExceedMatchLimit() const {
    std::lock_guard<std::mutex> lock(mutex);
    return ts_query_cursor_did_exceed_match_limit(cursor);
}

void TreeSitterQueryCursor::clearByteRange() {
    std::lock_guard<std::mutex> lock(mutex);
    ts_query_cursor_set_byte_range(cursor, 0, UINT32_MAX);
}

void TreeSitterQueryCursor::clearPointRange() {
    std::lock_guard<std::mutex> lock(mutex);
    TSPoint start = {0, 0};
    TSPoint end = {UINT32_MAX, UINT32_MAX};
    ts_query_cursor_set_point_range(cursor, start, end);
}
