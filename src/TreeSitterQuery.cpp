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

std::string TreeSitterQuery::getNodeText(TSNode node, const std::string& src) {
    uint32_t start = ts_node_start_byte(node);
    uint32_t end = ts_node_end_byte(node);
    if (start >= src.size() || end > src.size()) {
        return "";
    }
    return src.substr(start, end - start);
}

void TreeSitterQuery::applyCursorSettings(TSQueryCursor* cursor) const {
    if (has_byte_range) {
        ts_query_cursor_set_byte_range(cursor, byte_range_start, byte_range_end);
    }
    if (has_point_range) {
        ts_query_cursor_set_point_range(cursor, point_range_start, point_range_end);
    }
    if (has_max_start_depth) {
        ts_query_cursor_set_max_start_depth(cursor, max_start_depth);
    }
}

bool TreeSitterQuery::evaluatePredicates(uint32_t pattern_index, const TSQueryMatch& match,
                                          const std::string& src,
                                          const QoreHashNode* metadata) const {
    if (!query) {
        return true;
    }

    uint32_t step_count;
    const TSQueryPredicateStep* steps = ts_query_predicates_for_pattern(query, pattern_index, &step_count);
    if (step_count == 0) {
        return true;
    }

    // Parse predicate steps into individual predicates (delimited by Done steps)
    uint32_t i = 0;
    while (i < step_count) {
        // First step should be the predicate name (a String type)
        if (steps[i].type != TSQueryPredicateStepTypeString) {
            // Skip to next Done
            while (i < step_count && steps[i].type != TSQueryPredicateStepTypeDone) {
                ++i;
            }
            if (i < step_count) {
                ++i;
            }
            continue;
        }

        uint32_t name_len;
        const char* pred_name = ts_query_string_value_for_id(query, steps[i].value_id, &name_len);
        std::string predicate(pred_name, name_len);
        ++i;

        // Collect arguments until Done
        struct PredicateArg {
            enum { CAPTURE, STRING } type;
            uint32_t value_id;  // capture index or string id
        };
        std::vector<PredicateArg> args;
        while (i < step_count && steps[i].type != TSQueryPredicateStepTypeDone) {
            PredicateArg arg;
            if (steps[i].type == TSQueryPredicateStepTypeCapture) {
                arg.type = PredicateArg::CAPTURE;
                arg.value_id = steps[i].value_id;
            } else {
                arg.type = PredicateArg::STRING;
                arg.value_id = steps[i].value_id;
            }
            args.push_back(arg);
            ++i;
        }
        if (i < step_count) {
            ++i;  // skip Done
        }

        // Helper to get text for a capture argument from the match
        auto getCaptureText = [&](uint32_t capture_index) -> std::string {
            for (uint16_t c = 0; c < match.capture_count; c++) {
                if (match.captures[c].index == capture_index) {
                    return getNodeText(match.captures[c].node, src);
                }
            }
            return "";
        };

        auto getStringValue = [&](uint32_t string_id) -> std::string {
            uint32_t len;
            const char* val = ts_query_string_value_for_id(query, string_id, &len);
            return std::string(val, len);
        };

        // Evaluate the predicate
        if (predicate == "eq?" || predicate == "not-eq?") {
            if (args.size() < 2) {
                continue;
            }
            std::string lhs;
            if (args[0].type == PredicateArg::CAPTURE) {
                lhs = getCaptureText(args[0].value_id);
            } else {
                lhs = getStringValue(args[0].value_id);
            }
            std::string rhs;
            if (args[1].type == PredicateArg::CAPTURE) {
                rhs = getCaptureText(args[1].value_id);
            } else {
                rhs = getStringValue(args[1].value_id);
            }
            bool eq = (lhs == rhs);
            if (predicate == "eq?" && !eq) {
                return false;
            }
            if (predicate == "not-eq?" && eq) {
                return false;
            }
        } else if (predicate == "match?" || predicate == "not-match?") {
            if (args.size() < 2 || args[0].type != PredicateArg::CAPTURE
                || args[1].type != PredicateArg::STRING) {
                continue;
            }
            std::string text = getCaptureText(args[0].value_id);
            std::string pattern = getStringValue(args[1].value_id);
            bool matched = false;
            try {
                std::lock_guard<std::mutex> lock(regex_cache_mutex);
                auto cache_it = regex_cache.find(pattern);
                if (cache_it == regex_cache.end()) {
                    regex_cache.emplace(pattern, std::regex(pattern, std::regex::ECMAScript));
                    cache_it = regex_cache.find(pattern);
                }
                matched = std::regex_search(text, cache_it->second);
            } catch (const std::regex_error&) {
                // Invalid regex - treat as non-matching
                matched = false;
            }
            if (predicate == "match?" && !matched) {
                return false;
            }
            if (predicate == "not-match?" && matched) {
                return false;
            }
        } else if (predicate == "any-of?" || predicate == "not-any-of?") {
            if (args.size() < 2 || args[0].type != PredicateArg::CAPTURE) {
                continue;
            }
            std::string text = getCaptureText(args[0].value_id);
            bool found = false;
            for (size_t a = 1; a < args.size(); a++) {
                std::string val;
                if (args[a].type == PredicateArg::STRING) {
                    val = getStringValue(args[a].value_id);
                } else {
                    val = getCaptureText(args[a].value_id);
                }
                if (text == val) {
                    found = true;
                    break;
                }
            }
            if (predicate == "any-of?" && !found) {
                return false;
            }
            if (predicate == "not-any-of?" && found) {
                return false;
            }
        } else if (predicate == "is?" || predicate == "is-not?") {
            // #is? and #is-not? check metadata set by a prior query (e.g., locals.scm)
            if (args.size() < 1) {
                continue;
            }
            // First arg is the property name (usually a string like "local")
            std::string prop_name;
            uint32_t capture_idx = UINT32_MAX;
            if (args[0].type == PredicateArg::CAPTURE) {
                capture_idx = args[0].value_id;
                // Property name comes from arg[1] if present, otherwise use "local" default
                if (args.size() >= 2 && args[1].type == PredicateArg::STRING) {
                    prop_name = getStringValue(args[1].value_id);
                } else {
                    continue;
                }
            } else {
                prop_name = getStringValue(args[0].value_id);
                // Capture is arg[1] if present
                if (args.size() >= 2 && args[1].type == PredicateArg::CAPTURE) {
                    capture_idx = args[1].value_id;
                }
            }

            // Check if the captured node is in the metadata
            bool found_in_metadata = false;
            if (metadata && capture_idx != UINT32_MAX) {
                // Find the capture node
                TSNode cap_node = {};
                bool have_node = false;
                for (uint16_t c = 0; c < match.capture_count; c++) {
                    if (match.captures[c].index == capture_idx) {
                        cap_node = match.captures[c].node;
                        have_node = true;
                        break;
                    }
                }
                if (have_node) {
                    uint32_t node_start = ts_node_start_byte(cap_node);
                    uint32_t node_end = ts_node_end_byte(cap_node);
                    // Look up prop_name in metadata; value is a list of {start_byte, end_byte} hashes
                    QoreValue prop_val = metadata->getKeyValue(prop_name.c_str());
                    if (prop_val.getType() == NT_LIST) {
                        const QoreListNode* ranges = prop_val.get<const QoreListNode>();
                        ConstListIterator li(ranges);
                        while (li.next()) {
                            if (li.getValue().getType() == NT_HASH) {
                                const QoreHashNode* rh = li.getValue().get<const QoreHashNode>();
                                uint32_t rs = static_cast<uint32_t>(rh->getKeyValue("start_byte").getAsBigInt());
                                uint32_t re = static_cast<uint32_t>(rh->getKeyValue("end_byte").getAsBigInt());
                                if (rs == node_start && re == node_end) {
                                    found_in_metadata = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            if (predicate == "is?" && !found_in_metadata) {
                return false;
            }
            if (predicate == "is-not?" && found_in_metadata) {
                return false;
            }
        } else if (predicate == "set!") {
            // #set! is a directive, not a filter — skip it during predicate evaluation
            continue;
        }
        // Other unknown predicates are silently ignored
    }

    return true;
}

QoreHashNode* TreeSitterQuery::collectDirectives(uint32_t pattern_index, const TSQueryMatch& match,
                                                   const std::string& src, ExceptionSink* xsink) const {
    if (!query) {
        return nullptr;
    }

    uint32_t step_count;
    const TSQueryPredicateStep* steps = ts_query_predicates_for_pattern(query, pattern_index, &step_count);
    if (step_count == 0) {
        return nullptr;
    }

    QoreHashNode* directives = nullptr;
    uint32_t i = 0;
    while (i < step_count) {
        if (steps[i].type != TSQueryPredicateStepTypeString) {
            while (i < step_count && steps[i].type != TSQueryPredicateStepTypeDone) {
                ++i;
            }
            if (i < step_count) {
                ++i;
            }
            continue;
        }

        uint32_t name_len;
        const char* pred_name = ts_query_string_value_for_id(query, steps[i].value_id, &name_len);
        std::string predicate(pred_name, name_len);
        ++i;

        // Collect arguments
        std::vector<std::pair<int, uint32_t>> dargs;
        while (i < step_count && steps[i].type != TSQueryPredicateStepTypeDone) {
            dargs.push_back({steps[i].type == TSQueryPredicateStepTypeCapture ? 0 : 1, steps[i].value_id});
            ++i;
        }
        if (i < step_count) {
            ++i;
        }

        if (predicate == "set!") {
            if (dargs.size() >= 2 && dargs[0].first == 1 && dargs[1].first == 1) {
                uint32_t klen, vlen;
                const char* key = ts_query_string_value_for_id(query, dargs[0].second, &klen);
                const char* val = ts_query_string_value_for_id(query, dargs[1].second, &vlen);
                if (key && val) {
                    if (!directives) {
                        directives = new QoreHashNode(autoTypeInfo);
                    }
                    directives->setKeyValue(std::string(key, klen).c_str(),
                        new QoreStringNode(val, vlen, QCS_UTF8), xsink);
                }
            } else if (dargs.size() >= 1 && dargs[0].first == 1) {
                uint32_t klen;
                const char* key = ts_query_string_value_for_id(query, dargs[0].second, &klen);
                if (key) {
                    if (!directives) {
                        directives = new QoreHashNode(autoTypeInfo);
                    }
                    directives->setKeyValue(std::string(key, klen).c_str(), true, xsink);
                }
            }
        }
    }

    return directives;
}

QoreHashNode* TreeSitterQuery::resolveLocals(const char* lang_name, TreeSitterNode* node,
                                              ExceptionSink* xsink) {
    QoreStringNode* locals_src = TreeSitterLanguages::getQuery(lang_name, "locals", xsink);
    if (!locals_src || *xsink) {
        if (locals_src) {
            locals_src->deref();
        }
        return nullptr;
    }

    TreeSitterQuery locals_query(lang_name, locals_src->c_str(), xsink);
    locals_src->deref();
    if (*xsink || !locals_query.isValid()) {
        return nullptr;
    }

    const std::string& src = node->getSource();

    TSQueryCursor* cursor = ts_query_cursor_new();
    ts_query_cursor_exec(cursor, locals_query.getQuery(), node->getNode());

    struct ScopeInfo {
        uint32_t start_byte;
        uint32_t end_byte;
        std::vector<std::string> definitions;
    };
    std::vector<ScopeInfo> scopes;

    struct RefInfo {
        uint32_t start_byte;
        uint32_t end_byte;
        std::string name;
    };
    std::vector<RefInfo> references;

    TSQueryMatch match;
    while (ts_query_cursor_next_match(cursor, &match)) {
        for (uint16_t i = 0; i < match.capture_count; i++) {
            uint32_t cname_len;
            const char* cap_name = ts_query_capture_name_for_id(locals_query.getQuery(),
                match.captures[i].index, &cname_len);
            std::string capture_name(cap_name, cname_len);
            TSNode cap_node = match.captures[i].node;

            if (capture_name == "local.scope") {
                ScopeInfo scope;
                scope.start_byte = ts_node_start_byte(cap_node);
                scope.end_byte = ts_node_end_byte(cap_node);
                scopes.push_back(scope);
            } else if (capture_name == "local.definition") {
                std::string def_name = getNodeText(cap_node, src);
                uint32_t def_start = ts_node_start_byte(cap_node);
                for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
                    if (def_start >= it->start_byte && def_start < it->end_byte) {
                        it->definitions.push_back(def_name);
                        break;
                    }
                }
            } else if (capture_name == "local.reference") {
                RefInfo ref;
                ref.start_byte = ts_node_start_byte(cap_node);
                ref.end_byte = ts_node_end_byte(cap_node);
                ref.name = getNodeText(cap_node, src);
                references.push_back(ref);
            }
        }
    }

    ts_query_cursor_delete(cursor);

    QoreListNode* local_ranges = new QoreListNode(autoTypeInfo);
    for (const auto& ref : references) {
        bool is_local = false;
        for (const auto& scope : scopes) {
            if (ref.start_byte >= scope.start_byte && ref.end_byte <= scope.end_byte) {
                for (const auto& def : scope.definitions) {
                    if (def == ref.name) {
                        is_local = true;
                        break;
                    }
                }
                if (is_local) {
                    break;
                }
            }
        }
        if (is_local) {
            QoreHashNode* range = new QoreHashNode(autoTypeInfo);
            range->setKeyValue("start_byte", static_cast<int64>(ref.start_byte), xsink);
            range->setKeyValue("end_byte", static_cast<int64>(ref.end_byte), xsink);
            local_ranges->push(range, xsink);
        }
    }

    QoreHashNode* metadata = new QoreHashNode(autoTypeInfo);
    metadata->setKeyValue("local", local_ranges, xsink);
    return metadata;
}

QoreListNode* TreeSitterQuery::execute(TreeSitterNode* node, ExceptionSink* xsink,
                                        const QoreHashNode* metadata) {
    if (!query || !node) {
        return new QoreListNode(autoTypeInfo);
    }

    const std::string& src = node->getSource();

    TSQueryCursor* cursor = ts_query_cursor_new();
    ts_query_cursor_exec(cursor, query, node->getNode());
    applyCursorSettings(cursor);

    QoreListNode* matches = new QoreListNode(autoTypeInfo);
    TSQueryMatch match;

    while (ts_query_cursor_next_match(cursor, &match)) {
        if (!evaluatePredicates(match.pattern_index, match, src, metadata)) {
            continue;
        }

        QoreHashNode* match_hash = new QoreHashNode(autoTypeInfo);
        match_hash->setKeyValue("pattern_index", static_cast<int64>(match.pattern_index), xsink);

        QoreListNode* captures = new QoreListNode(autoTypeInfo);
        for (uint16_t i = 0; i < match.capture_count; i++) {
            const TSQueryCapture& capture = match.captures[i];

            QoreHashNode* capture_hash = new QoreHashNode(autoTypeInfo);
            uint32_t name_len;
            const char* name = ts_query_capture_name_for_id(query, capture.index, &name_len);
            capture_hash->setKeyValue("name", new QoreStringNode(name, name_len, QCS_UTF8), xsink);
            capture_hash->setKeyValue("node", TreeSitterNode::buildNodeInfo(capture.node, src, xsink), xsink);

            QoreHashNode* directives = collectDirectives(match.pattern_index, match, src, xsink);
            if (directives) {
                capture_hash->setKeyValue("directives", directives, xsink);
            }

            captures->push(capture_hash, xsink);
        }
        match_hash->setKeyValue("captures", captures, xsink);
        matches->push(match_hash, xsink);
    }

    ts_query_cursor_delete(cursor);
    return matches;
}

QoreListNode* TreeSitterQuery::captures(TreeSitterNode* node, ExceptionSink* xsink,
                                         const QoreHashNode* metadata) {
    if (!query || !node) {
        return new QoreListNode(autoTypeInfo);
    }

    const std::string& src = node->getSource();

    TSQueryCursor* cursor = ts_query_cursor_new();
    ts_query_cursor_exec(cursor, query, node->getNode());
    applyCursorSettings(cursor);

    QoreListNode* all_captures = new QoreListNode(autoTypeInfo);
    TSQueryMatch match;
    uint32_t capture_index;

    while (ts_query_cursor_next_capture(cursor, &match, &capture_index)) {
        if (!evaluatePredicates(match.pattern_index, match, src, metadata)) {
            ts_query_cursor_remove_match(cursor, match.id);
            continue;
        }

        const TSQueryCapture& capture = match.captures[capture_index];

        QoreHashNode* capture_hash = new QoreHashNode(autoTypeInfo);
        uint32_t name_len;
        const char* name = ts_query_capture_name_for_id(query, capture.index, &name_len);
        capture_hash->setKeyValue("name", new QoreStringNode(name, name_len, QCS_UTF8), xsink);
        capture_hash->setKeyValue("pattern_index", static_cast<int64>(match.pattern_index), xsink);
        capture_hash->setKeyValue("node", TreeSitterNode::buildNodeInfo(capture.node, src, xsink), xsink);

        QoreHashNode* directives = collectDirectives(match.pattern_index, match, src, xsink);
        if (directives) {
            capture_hash->setKeyValue("directives", directives, xsink);
        }

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
    if (!query) {
        return nullptr;
    }
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
    has_byte_range = true;
    byte_range_start = start;
    byte_range_end = end;
}

void TreeSitterQuery::setPointRange(TSPoint start, TSPoint end) {
    has_point_range = true;
    point_range_start = start;
    point_range_end = end;
}

void TreeSitterQuery::setMaxStartDepth(uint32_t depth) {
    has_max_start_depth = true;
    max_start_depth = depth;
}

void TreeSitterQuery::clearByteRange() {
    has_byte_range = false;
}

void TreeSitterQuery::clearPointRange() {
    has_point_range = false;
}
