/* -*- mode: c++; indent-tabs-mode: nil -*- */
/** @file TreeSitterTree.cpp TreeSitterTree implementation */
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

#include "TreeSitterTree.h"

TreeSitterTree::TreeSitterTree(TSTree* tree, const std::string& source)
    : tree(tree), source(source) {
}

TreeSitterTree::~TreeSitterTree() {
    if (tree) {
        ts_tree_delete(tree);
    }
}

TreeSitterNode* TreeSitterTree::getRootNode() const {
    if (!tree) {
        return nullptr;
    }
    TSNode root = ts_tree_root_node(tree);
    return new TreeSitterNode(root, source);
}

TreeSitterTree* TreeSitterTree::copy() const {
    if (!tree) {
        return nullptr;
    }
    TSTree* tree_copy = ts_tree_copy(tree);
    return new TreeSitterTree(tree_copy, source);
}

void TreeSitterTree::edit(uint32_t start_byte, uint32_t old_end_byte, uint32_t new_end_byte,
                          uint32_t start_row, uint32_t start_column,
                          uint32_t old_end_row, uint32_t old_end_column,
                          uint32_t new_end_row, uint32_t new_end_column) {
    if (!tree) {
        return;
    }

    TSInputEdit edit;
    edit.start_byte = start_byte;
    edit.old_end_byte = old_end_byte;
    edit.new_end_byte = new_end_byte;
    edit.start_point = {start_row, start_column};
    edit.old_end_point = {old_end_row, old_end_column};
    edit.new_end_point = {new_end_row, new_end_column};

    ts_tree_edit(tree, &edit);
}

QoreListNode* TreeSitterTree::getChangedRanges(TreeSitterTree* other, ExceptionSink* xsink) const {
    if (!tree || !other || !other->tree) {
        return new QoreListNode(autoTypeInfo);
    }

    uint32_t range_count;
    TSRange* ranges = ts_tree_get_changed_ranges(tree, other->tree, &range_count);

    QoreListNode* list = new QoreListNode(autoTypeInfo);
    for (uint32_t i = 0; i < range_count; i++) {
        QoreHashNode* range = new QoreHashNode(autoTypeInfo);
        range->setKeyValue("start_byte", static_cast<int64>(ranges[i].start_byte), xsink);
        range->setKeyValue("end_byte", static_cast<int64>(ranges[i].end_byte), xsink);
        range->setKeyValue("start_row", static_cast<int64>(ranges[i].start_point.row), xsink);
        range->setKeyValue("start_column", static_cast<int64>(ranges[i].start_point.column), xsink);
        range->setKeyValue("end_row", static_cast<int64>(ranges[i].end_point.row), xsink);
        range->setKeyValue("end_column", static_cast<int64>(ranges[i].end_point.column), xsink);
        list->push(range, xsink);
    }

    free(ranges);
    return list;
}

const TSLanguage* TreeSitterTree::getLanguage() const {
    if (!tree) {
        return nullptr;
    }
    return ts_tree_language(tree);
}
