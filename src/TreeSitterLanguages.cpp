/* -*- mode: c++; indent-tabs-mode: nil -*- */
/** @file TreeSitterLanguages.cpp Language registry implementation */
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

#include "treesitter-module.h"

// External language declarations
extern "C" {
    const TSLanguage* tree_sitter_python();
    const TSLanguage* tree_sitter_java();
    const TSLanguage* tree_sitter_json();
    const TSLanguage* tree_sitter_yaml();
    const TSLanguage* tree_sitter_javascript();
    const TSLanguage* tree_sitter_kotlin();
    const TSLanguage* tree_sitter_typescript();
    const TSLanguage* tree_sitter_tsx();
}

std::unordered_map<std::string, const TSLanguage*> TreeSitterLanguages::languages;
bool TreeSitterLanguages::initialized = false;

void TreeSitterLanguages::initLanguages() {
    if (initialized) {
        return;
    }

    languages["python"] = tree_sitter_python();
    languages["java"] = tree_sitter_java();
    languages["json"] = tree_sitter_json();
    languages["yaml"] = tree_sitter_yaml();
    languages["javascript"] = tree_sitter_javascript();
    languages["js"] = tree_sitter_javascript();  // Alias
    languages["kotlin"] = tree_sitter_kotlin();
    languages["kt"] = tree_sitter_kotlin();  // Alias
    languages["typescript"] = tree_sitter_typescript();
    languages["ts"] = tree_sitter_typescript();  // Alias
    languages["tsx"] = tree_sitter_tsx();

    initialized = true;
}

const TSLanguage* TreeSitterLanguages::getLanguage(const char* name) {
    initLanguages();

    auto it = languages.find(name);
    if (it != languages.end()) {
        return it->second;
    }
    return nullptr;
}

QoreListNode* TreeSitterLanguages::getLanguageList() {
    initLanguages();

    QoreListNode* list = new QoreListNode(autoTypeInfo);
    list->push(new QoreStringNode("python"), nullptr);
    list->push(new QoreStringNode("java"), nullptr);
    list->push(new QoreStringNode("json"), nullptr);
    list->push(new QoreStringNode("yaml"), nullptr);
    list->push(new QoreStringNode("javascript"), nullptr);
    list->push(new QoreStringNode("kotlin"), nullptr);
    list->push(new QoreStringNode("typescript"), nullptr);
    list->push(new QoreStringNode("tsx"), nullptr);
    return list;
}

bool TreeSitterLanguages::isLanguageSupported(const char* name) {
    initLanguages();
    return languages.find(name) != languages.end();
}
