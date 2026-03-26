/* -*- mode: c++; indent-tabs-mode: nil -*- */
/** @file TreeSitterLookaheadIterator.cpp TreeSitterLookaheadIterator implementation */
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

#include "TreeSitterLookaheadIterator.h"

TreeSitterLookaheadIterator::TreeSitterLookaheadIterator(const char* language, TSStateId state,
                                                           ExceptionSink* xsink)
    : iterator(nullptr) {
    const TSLanguage* lang = TreeSitterLanguages::getLanguage(language);
    if (!lang) {
        raiseTreeSitterException(xsink, "TREESITTER-LANGUAGE-ERROR",
            "Unknown language: '%s'", language);
        return;
    }

    iterator = ts_lookahead_iterator_new(lang, state);
    if (!iterator) {
        raiseTreeSitterException(xsink, "TREESITTER-LOOKAHEAD-ERROR",
            "Failed to create lookahead iterator for state %u", state);
    }
}

TreeSitterLookaheadIterator::~TreeSitterLookaheadIterator() {
    if (iterator) {
        ts_lookahead_iterator_delete(iterator);
    }
}

bool TreeSitterLookaheadIterator::reset(const char* language, TSStateId state, ExceptionSink* xsink) {
    if (!iterator) {
        return false;
    }
    const TSLanguage* lang = TreeSitterLanguages::getLanguage(language);
    if (!lang) {
        raiseTreeSitterException(xsink, "TREESITTER-LANGUAGE-ERROR",
            "Unknown language: '%s'", language);
        return false;
    }
    std::lock_guard<std::mutex> lock(mutex);
    return ts_lookahead_iterator_reset(iterator, lang, state);
}

bool TreeSitterLookaheadIterator::resetState(TSStateId state) {
    if (!iterator) {
        return false;
    }
    std::lock_guard<std::mutex> lock(mutex);
    return ts_lookahead_iterator_reset_state(iterator, state);
}

bool TreeSitterLookaheadIterator::next() {
    if (!iterator) {
        return false;
    }
    std::lock_guard<std::mutex> lock(mutex);
    return ts_lookahead_iterator_next(iterator);
}

TSSymbol TreeSitterLookaheadIterator::getCurrentSymbol() const {
    if (!iterator) {
        return 0;
    }
    std::lock_guard<std::mutex> lock(mutex);
    return ts_lookahead_iterator_current_symbol(iterator);
}

const char* TreeSitterLookaheadIterator::getCurrentSymbolName() const {
    if (!iterator) {
        return "";
    }
    std::lock_guard<std::mutex> lock(mutex);
    return ts_lookahead_iterator_current_symbol_name(iterator);
}
