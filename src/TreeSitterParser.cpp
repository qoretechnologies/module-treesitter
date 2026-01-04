/* -*- mode: c++; indent-tabs-mode: nil -*- */
/** @file TreeSitterParser.cpp TreeSitterParser implementation */
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

#include "TreeSitterParser.h"
#include "TreeSitterTree.h"

TreeSitterParser::TreeSitterParser(const char* language, ExceptionSink* xsink)
    : parser(nullptr), language_name(language) {
    const TSLanguage* lang = TreeSitterLanguages::getLanguage(language);
    if (!lang) {
        raiseTreeSitterException(xsink, "TREESITTER-LANGUAGE-ERROR",
            "Unknown language: '%s'. Supported languages: python, java, json, yaml, javascript, sql",
            language);
        return;
    }

    parser = ts_parser_new();
    if (!parser) {
        raiseTreeSitterException(xsink, "TREESITTER-PARSER-ERROR",
            "Failed to create parser");
        return;
    }

    if (!ts_parser_set_language(parser, lang)) {
        ts_parser_delete(parser);
        parser = nullptr;
        raiseTreeSitterException(xsink, "TREESITTER-LANGUAGE-ERROR",
            "Failed to set language '%s' - ABI version mismatch", language);
        return;
    }
}

TreeSitterParser::~TreeSitterParser() {
    if (parser) {
        ts_parser_delete(parser);
    }
}

TreeSitterTree* TreeSitterParser::parse(const QoreStringNode* source, ExceptionSink* xsink) {
    if (!parser) {
        raiseTreeSitterException(xsink, "TREESITTER-PARSER-ERROR",
            "Parser is not valid");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(mutex);

    const char* src = source->c_str();
    size_t len = source->size();

    TSTree* tree = ts_parser_parse_string(parser, nullptr, src, len);
    if (!tree) {
        raiseTreeSitterException(xsink, "TREESITTER-PARSE-ERROR",
            "Failed to parse source code");
        return nullptr;
    }

    return new TreeSitterTree(tree, std::string(src, len));
}

TreeSitterTree* TreeSitterParser::parseIncremental(const QoreStringNode* source,
                                                    TreeSitterTree* old_tree,
                                                    ExceptionSink* xsink) {
    if (!parser) {
        raiseTreeSitterException(xsink, "TREESITTER-PARSER-ERROR",
            "Parser is not valid");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(mutex);

    const char* src = source->c_str();
    size_t len = source->size();

    TSTree* old_ts_tree = old_tree ? old_tree->getTree() : nullptr;
    TSTree* tree = ts_parser_parse_string(parser, old_ts_tree, src, len);
    if (!tree) {
        raiseTreeSitterException(xsink, "TREESITTER-PARSE-ERROR",
            "Failed to parse source code incrementally");
        return nullptr;
    }

    return new TreeSitterTree(tree, std::string(src, len));
}

void TreeSitterParser::setTimeout(uint64_t timeout_micros) {
    if (parser) {
        ts_parser_set_timeout_micros(parser, timeout_micros);
    }
}

uint64_t TreeSitterParser::getTimeout() const {
    if (parser) {
        return ts_parser_timeout_micros(parser);
    }
    return 0;
}
