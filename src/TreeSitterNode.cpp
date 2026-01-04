/* -*- mode: c++; indent-tabs-mode: nil -*- */
/** @file TreeSitterNode.cpp TreeSitterNode implementation */
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

#include "TreeSitterNode.h"

TreeSitterNode::TreeSitterNode(TSNode node, const std::string& source)
    : node(node), source(source) {
}

TreeSitterNode::~TreeSitterNode() {
    // TSNode is a value type, no cleanup needed
}

const char* TreeSitterNode::getType() const {
    return ts_node_type(node);
}

TSSymbol TreeSitterNode::getSymbol() const {
    return ts_node_symbol(node);
}

uint32_t TreeSitterNode::getStartByte() const {
    return ts_node_start_byte(node);
}

uint32_t TreeSitterNode::getEndByte() const {
    return ts_node_end_byte(node);
}

TSPoint TreeSitterNode::getStartPoint() const {
    return ts_node_start_point(node);
}

TSPoint TreeSitterNode::getEndPoint() const {
    return ts_node_end_point(node);
}

QoreStringNode* TreeSitterNode::getText() const {
    uint32_t start = ts_node_start_byte(node);
    uint32_t end = ts_node_end_byte(node);
    if (start >= source.size() || end > source.size()) {
        return new QoreStringNode();
    }
    return new QoreStringNode(source.substr(start, end - start));
}

uint32_t TreeSitterNode::getChildCount() const {
    return ts_node_child_count(node);
}

TreeSitterNode* TreeSitterNode::getChild(uint32_t index) const {
    TSNode child = ts_node_child(node, index);
    if (ts_node_is_null(child)) {
        return nullptr;
    }
    return new TreeSitterNode(child, source);
}

TreeSitterNode* TreeSitterNode::getChildByFieldName(const char* field_name) const {
    TSNode child = ts_node_child_by_field_name(node, field_name, strlen(field_name));
    if (ts_node_is_null(child)) {
        return nullptr;
    }
    return new TreeSitterNode(child, source);
}

uint32_t TreeSitterNode::getNamedChildCount() const {
    return ts_node_named_child_count(node);
}

TreeSitterNode* TreeSitterNode::getNamedChild(uint32_t index) const {
    TSNode child = ts_node_named_child(node, index);
    if (ts_node_is_null(child)) {
        return nullptr;
    }
    return new TreeSitterNode(child, source);
}

TreeSitterNode* TreeSitterNode::getParent() const {
    TSNode parent = ts_node_parent(node);
    if (ts_node_is_null(parent)) {
        return nullptr;
    }
    return new TreeSitterNode(parent, source);
}

TreeSitterNode* TreeSitterNode::getNextSibling() const {
    TSNode sibling = ts_node_next_sibling(node);
    if (ts_node_is_null(sibling)) {
        return nullptr;
    }
    return new TreeSitterNode(sibling, source);
}

TreeSitterNode* TreeSitterNode::getPrevSibling() const {
    TSNode sibling = ts_node_prev_sibling(node);
    if (ts_node_is_null(sibling)) {
        return nullptr;
    }
    return new TreeSitterNode(sibling, source);
}

TreeSitterNode* TreeSitterNode::getNextNamedSibling() const {
    TSNode sibling = ts_node_next_named_sibling(node);
    if (ts_node_is_null(sibling)) {
        return nullptr;
    }
    return new TreeSitterNode(sibling, source);
}

TreeSitterNode* TreeSitterNode::getPrevNamedSibling() const {
    TSNode sibling = ts_node_prev_named_sibling(node);
    if (ts_node_is_null(sibling)) {
        return nullptr;
    }
    return new TreeSitterNode(sibling, source);
}

bool TreeSitterNode::isNamed() const {
    return ts_node_is_named(node);
}

bool TreeSitterNode::isMissing() const {
    return ts_node_is_missing(node);
}

bool TreeSitterNode::hasError() const {
    return ts_node_has_error(node);
}

bool TreeSitterNode::isExtra() const {
    return ts_node_is_extra(node);
}

bool TreeSitterNode::isNull() const {
    return ts_node_is_null(node);
}

const char* TreeSitterNode::getFieldName() const {
    // This is only valid when the node was retrieved via a parent
    // In tree-sitter, this is context-dependent, so we return nullptr
    return nullptr;
}

std::vector<TreeSitterNode*> TreeSitterNode::getChildren() const {
    std::vector<TreeSitterNode*> result;
    uint32_t count = ts_node_child_count(node);
    result.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        TSNode child = ts_node_child(node, i);
        result.push_back(new TreeSitterNode(child, source));
    }
    return result;
}

std::vector<TreeSitterNode*> TreeSitterNode::getNamedChildren() const {
    std::vector<TreeSitterNode*> result;
    uint32_t count = ts_node_named_child_count(node);
    result.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        TSNode child = ts_node_named_child(node, i);
        result.push_back(new TreeSitterNode(child, source));
    }
    return result;
}

QoreHashNode* TreeSitterNode::toHash() const {
    QoreHashNode* h = new QoreHashNode(autoTypeInfo);
    ExceptionSink xsink;

    h->setKeyValue("type", new QoreStringNode(getType()), &xsink);
    h->setKeyValue("start_byte", static_cast<int64>(getStartByte()), &xsink);
    h->setKeyValue("end_byte", static_cast<int64>(getEndByte()), &xsink);

    TSPoint start = getStartPoint();
    TSPoint end = getEndPoint();
    h->setKeyValue("start_row", static_cast<int64>(start.row), &xsink);
    h->setKeyValue("start_column", static_cast<int64>(start.column), &xsink);
    h->setKeyValue("end_row", static_cast<int64>(end.row), &xsink);
    h->setKeyValue("end_column", static_cast<int64>(end.column), &xsink);
    h->setKeyValue("is_named", isNamed(), &xsink);
    h->setKeyValue("is_missing", isMissing(), &xsink);
    h->setKeyValue("has_error", hasError(), &xsink);
    h->setKeyValue("text", getText(), &xsink);

    return h;
}

QoreStringNode* TreeSitterNode::toSexp() const {
    char* sexp = ts_node_string(node);
    QoreStringNode* result = new QoreStringNode(sexp);
    free(sexp);
    return result;
}

TreeSitterNode* TreeSitterNode::getDescendantForByteRange(uint32_t start, uint32_t end) const {
    TSNode desc = ts_node_descendant_for_byte_range(node, start, end);
    if (ts_node_is_null(desc)) {
        return nullptr;
    }
    return new TreeSitterNode(desc, source);
}

TreeSitterNode* TreeSitterNode::getNamedDescendantForByteRange(uint32_t start, uint32_t end) const {
    TSNode desc = ts_node_named_descendant_for_byte_range(node, start, end);
    if (ts_node_is_null(desc)) {
        return nullptr;
    }
    return new TreeSitterNode(desc, source);
}

TreeSitterNode* TreeSitterNode::getDescendantForPointRange(TSPoint start, TSPoint end) const {
    TSNode desc = ts_node_descendant_for_point_range(node, start, end);
    if (ts_node_is_null(desc)) {
        return nullptr;
    }
    return new TreeSitterNode(desc, source);
}

TreeSitterNode* TreeSitterNode::getNamedDescendantForPointRange(TSPoint start, TSPoint end) const {
    TSNode desc = ts_node_named_descendant_for_point_range(node, start, end);
    if (ts_node_is_null(desc)) {
        return nullptr;
    }
    return new TreeSitterNode(desc, source);
}

bool TreeSitterNode::equals(TreeSitterNode* other) const {
    if (!other) {
        return false;
    }
    return ts_node_eq(node, other->node);
}
