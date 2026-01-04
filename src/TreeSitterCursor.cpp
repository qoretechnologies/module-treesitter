/* -*- mode: c++; indent-tabs-mode: nil -*- */
/** @file TreeSitterCursor.cpp TreeSitterCursor implementation */
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

#include "TreeSitterCursor.h"

TreeSitterCursor::TreeSitterCursor(TreeSitterNode* node)
    : source(node ? "" : "") {
    if (node) {
        cursor = ts_tree_cursor_new(node->getNode());
        // Store source for creating nodes - use getSource() to avoid memory leak
        source = node->getSource();
    }
}

TreeSitterCursor::~TreeSitterCursor() {
    ts_tree_cursor_delete(&cursor);
}

void TreeSitterCursor::reset(TreeSitterNode* node) {
    if (node) {
        ts_tree_cursor_reset(&cursor, node->getNode());
    }
}

TreeSitterNode* TreeSitterCursor::getCurrentNode() const {
    TSNode node = ts_tree_cursor_current_node(&cursor);
    if (ts_node_is_null(node)) {
        return nullptr;
    }
    return new TreeSitterNode(node, source);
}

const char* TreeSitterCursor::getCurrentFieldName() const {
    return ts_tree_cursor_current_field_name(&cursor);
}

TSFieldId TreeSitterCursor::getCurrentFieldId() const {
    return ts_tree_cursor_current_field_id(&cursor);
}

bool TreeSitterCursor::gotoParent() {
    return ts_tree_cursor_goto_parent(&cursor);
}

bool TreeSitterCursor::gotoNextSibling() {
    return ts_tree_cursor_goto_next_sibling(&cursor);
}

bool TreeSitterCursor::gotoPrevSibling() {
    return ts_tree_cursor_goto_previous_sibling(&cursor);
}

bool TreeSitterCursor::gotoFirstChild() {
    return ts_tree_cursor_goto_first_child(&cursor);
}

bool TreeSitterCursor::gotoLastChild() {
    return ts_tree_cursor_goto_last_child(&cursor);
}

int64_t TreeSitterCursor::gotoFirstChildForByte(uint32_t byte_offset) {
    return ts_tree_cursor_goto_first_child_for_byte(&cursor, byte_offset);
}

int64_t TreeSitterCursor::gotoFirstChildForPoint(TSPoint point) {
    return ts_tree_cursor_goto_first_child_for_point(&cursor, point);
}

uint32_t TreeSitterCursor::getCurrentDepth() const {
    return ts_tree_cursor_current_depth(&cursor);
}

TreeSitterCursor* TreeSitterCursor::copy() const {
    TreeSitterCursor* new_cursor = new TreeSitterCursor(nullptr);
    new_cursor->cursor = ts_tree_cursor_copy(&cursor);
    new_cursor->source = source;
    return new_cursor;
}
