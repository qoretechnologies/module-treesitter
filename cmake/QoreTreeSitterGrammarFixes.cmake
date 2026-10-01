# Copyright (C) 2026 Qore Technologies, s.r.o.
# SPDX-License-Identifier: MIT
# Fix the pinned grammar sources in the build tree; never edit FetchContent inputs.
function(qore_treesitter_fixed_source language filename source_dir output_var)
    set(_input "${source_dir}/${filename}")
    file(SHA256 "${_input}" _hash)
    file(READ "${_input}" _source)
    if(language STREQUAL "python" AND filename STREQUAL "scanner.c")
        set(_original_hash 6db82134ac2d4c90a1a1475487a625cface02662ebda9b7478cad9c7147e9afe)
        set(_fixed_hash 8668a0650df6dce5ee7a47f8d7dac4f5c30ed9176fe1704880f9cc488bf76268)
        string(REPLACE [==[array_pop(&scanner->delimiters);]==] [==[(void)array_pop(&scanner->delimiters);]==] _source "${_source}")
        string(REPLACE [==[array_pop(&scanner->indents);]==] [==[(void)array_pop(&scanner->indents);]==] _source "${_source}")
    elseif(language STREQUAL "yaml" AND filename STREQUAL "scanner.c")
        set(_original_hash 9cd962f7c7541247cfc7ebf661abaa5d9b642d27de16575c87093496557aae59)
        set(_fixed_hash 1ce3002c1f3d5c911ca6bc4f022a8cae42c65410bb257c2675ed4809d4ffeb68)
        string(REPLACE [==[array_pop(&scanner->ind_len_stk);]==] [==[(void)array_pop(&scanner->ind_len_stk);]==] _source "${_source}")
        string(REPLACE [==[array_pop(&scanner->ind_typ_stk);]==] [==[(void)array_pop(&scanner->ind_typ_stk);]==] _source "${_source}")
        string(REPLACE [==[    bool is_cur_wsp = is_wsp(scanner->cur_chr);
]==] [==[]==] _source "${_source}")
        string(REPLACE [==[            is_cur_wsp = is_lka_wsp;
]==] [==[]==] _source "${_source}")
        # The STOP cases intentionally share the FAIL return after marking the
        # accepted token, or rejecting a verbatim tag without its closing '>'.
        string(REPLACE [==[                mrk_end(scanner, lexer);
            case SCN_FAIL:]==] [==[                mrk_end(scanner, lexer);
                // fall through
            case SCN_FAIL:]==] _source "${_source}")
        string(REPLACE [==[                    mrk_end(scanner, lexer);
                case SCN_FAIL:]==] [==[                    mrk_end(scanner, lexer);
                    // fall through
                case SCN_FAIL:]==] _source "${_source}")
        string(REPLACE [==[                    }
                case SCN_FAIL:]==] [==[                    }
                    // fall through
                case SCN_FAIL:]==] _source "${_source}")
    elseif(language STREQUAL "yaml" AND filename STREQUAL "parser.c")
        set(_original_hash 8a3baaab33fb63cf9a89f97ec61dbb3ab0d4ef69be9f0f229092c79d129617c9)
        set(_fixed_hash 118f869b240f99483ef5ef5efe1d33a488c42cfb6b33faf5e7c340a2f72a5c26)
        string(REPLACE [==[  START_LEXER();
  eof = lexer->eof(lexer);]==] [==[  // All YAML content tokens come from the external scanner. This EOF-only
  // lexer needs no character lookahead or skip state from START_LEXER.
  bool result = false;
  bool eof;
  goto start;
next_state:
  lexer->advance(lexer, false);
start:
  eof = lexer->eof(lexer);]==] _source "${_source}")
    else()
        message(FATAL_ERROR "Unknown private grammar source: ${language}/${filename}")
    endif()
    if(NOT _hash STREQUAL _original_hash)
        message(FATAL_ERROR "Unexpected ${language}/${filename}; review the pinned grammar fix")
    endif()
    string(SHA256 _result_hash "${_source}")
    if(NOT _result_hash STREQUAL _fixed_hash)
        message(FATAL_ERROR "Pinned ${language}/${filename} fix did not match")
    endif()
    set(_output "${CMAKE_CURRENT_BINARY_DIR}/fixed-grammars/${language}/${filename}")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/fixed-grammars/${language}")
    file(WRITE "${_output}.tmp" "${_source}")
    configure_file("${_output}.tmp" "${_output}" COPYONLY)
    set(${output_var} "${_output}" PARENT_SCOPE)
endfunction()
