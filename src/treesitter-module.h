/* -*- mode: c++; indent-tabs-mode: nil -*- */
/** @file treesitter-module.h defines the treesitter module */
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

#ifndef _QORE_TREESITTER_MODULE_H
#define _QORE_TREESITTER_MODULE_H

#include <qore/Qore.h>
#include <tree_sitter/api.h>

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>

// Module version
#define TREESITTER_MODULE_VERSION "1.0.0"

// Forward declarations
class TreeSitterParser;
class TreeSitterTree;
class TreeSitterNode;
class TreeSitterCursor;
class TreeSitterQuery;

// Class IDs for private data
DLLLOCAL extern qore_classid_t CID_TREESITTERPARSER;
DLLLOCAL extern qore_classid_t CID_TREESITTERTREE;
DLLLOCAL extern qore_classid_t CID_TREESITTERNODE;
DLLLOCAL extern qore_classid_t CID_TREESITTERCURSOR;
DLLLOCAL extern qore_classid_t CID_TREESITTERQUERY;

// QoreClass pointers
DLLLOCAL extern QoreClass* QC_TREESITTERPARSER;
DLLLOCAL extern QoreClass* QC_TREESITTERTREE;
DLLLOCAL extern QoreClass* QC_TREESITTERNODE;
DLLLOCAL extern QoreClass* QC_TREESITTERCURSOR;
DLLLOCAL extern QoreClass* QC_TREESITTERQUERY;

// Language registry
class TreeSitterLanguages {
public:
    static const TSLanguage* getLanguage(const char* name);
    static QoreListNode* getLanguageList();
    static bool isLanguageSupported(const char* name);

    //! Get the bundled highlight query for a language
    /** @param name the language name
        @param xsink exception sink for error reporting
        @return the query string, or nullptr if not found
    */
    static QoreStringNode* getHighlightQuery(const char* name, ExceptionSink* xsink);

    //! Get a bundled query file for a language by type
    /** @param name the language name
        @param query_type the query type (e.g., "highlights", "locals", "injections")
        @param xsink exception sink for error reporting
        @return the query string, or nullptr if not found
    */
    static QoreStringNode* getQuery(const char* name, const char* query_type, ExceptionSink* xsink);

private:
    static void initLanguages();
    static std::unordered_map<std::string, const TSLanguage*> languages;
    static std::unordered_map<std::string, std::string> query_cache;
    static std::mutex query_cache_mutex;
    static bool initialized;
};

// Exception helper
DLLLOCAL void raiseTreeSitterException(ExceptionSink* xsink, const char* err, const char* fmt, ...);

#endif // _QORE_TREESITTER_MODULE_H
