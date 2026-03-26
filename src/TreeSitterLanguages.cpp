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

#include <fstream>
#include <sstream>
#include <unordered_set>

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
    const TSLanguage* tree_sitter_sql();
    const TSLanguage* tree_sitter_markdown();
    const TSLanguage* tree_sitter_qore();
}

std::unordered_map<std::string, const TSLanguage*> TreeSitterLanguages::languages;
std::unordered_map<std::string, std::string> TreeSitterLanguages::query_cache;
std::mutex TreeSitterLanguages::query_cache_mutex;
bool TreeSitterLanguages::initialized = false;

// Map of language names to their canonical names for query file lookup
static const std::unordered_map<std::string, std::string> query_lang_map = {
    {"python", "python"},
    {"java", "java"},
    {"json", "json"},
    {"yaml", "yaml"},
    {"javascript", "javascript"},
    {"js", "javascript"},
    {"kotlin", "kotlin"},
    {"kt", "kotlin"},
    {"typescript", "typescript"},
    {"ts", "typescript"},
    {"tsx", "typescript"},
    {"sql", "sql"},
    {"markdown", "markdown"},
    {"md", "markdown"},
};

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
    languages["sql"] = tree_sitter_sql();
    languages["markdown"] = tree_sitter_markdown();
    languages["md"] = tree_sitter_markdown();  // Alias
    languages["qore"] = tree_sitter_qore();

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
    list->push(new QoreStringNode("sql"), nullptr);
    list->push(new QoreStringNode("markdown"), nullptr);
    list->push(new QoreStringNode("qore"), nullptr);
    return list;
}

bool TreeSitterLanguages::isLanguageSupported(const char* name) {
    initLanguages();
    return languages.find(name) != languages.end();
}

// Valid query types (prevents path traversal via query_type parameter)
static const std::unordered_set<std::string> valid_query_types = {
    "highlights", "locals", "injections", "tags", "folds", "indents",
};

QoreStringNode* TreeSitterLanguages::getQuery(const char* name, const char* query_type,
                                               ExceptionSink* xsink) {
    std::string lang_name(name);
    std::string qtype(query_type);

    // Validate query type to prevent path traversal
    if (valid_query_types.find(qtype) == valid_query_types.end()) {
        xsink->raiseException("TREESITTER-QUERY-ERROR",
            "unknown query type '%s'; valid types: highlights, locals, injections, tags, folds, indents",
            query_type);
        return nullptr;
    }

    // Resolve aliases to canonical language names
    auto alias_it = query_lang_map.find(lang_name);
    if (alias_it == query_lang_map.end()) {
        xsink->raiseException("TREESITTER-QUERY-ERROR",
            "no queries available for language '%s'", name);
        return nullptr;
    }
    const std::string& canonical = alias_it->second;
    std::string cache_key = canonical + "/" + qtype;

    std::lock_guard<std::mutex> lock(query_cache_mutex);

    // Check cache
    auto cache_it = query_cache.find(cache_key);
    if (cache_it != query_cache.end()) {
        return new QoreStringNode(cache_it->second);
    }

    // Build the file path and read (file I/O is fast for small .scm files)
    std::string path = std::string(TREESITTER_QUERY_DIR) + "/" + canonical + "/" + qtype + ".scm";

    std::ifstream file(path);
    if (!file.is_open()) {
        xsink->raiseException("TREESITTER-QUERY-ERROR",
            "cannot open %s query file for language '%s': %s", query_type, name, path.c_str());
        return nullptr;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    query_cache[cache_key] = ss.str();
    const std::string& content = query_cache[cache_key];

    return new QoreStringNode(content);
}

QoreStringNode* TreeSitterLanguages::getHighlightQuery(const char* name, ExceptionSink* xsink) {
    return getQuery(name, "highlights", xsink);
}
