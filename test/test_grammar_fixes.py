#!/usr/bin/env python3
# Copyright (C) 2026 Qore Technologies, s.r.o.
# SPDX-License-Identifier: MIT
"""Check private source fixes, immutable inputs, and every YAML lexer state.

Usage: test_grammar_fixes.py PYTHON_GRAMMAR_ROOT YAML_GRAMMAR_ROOT
"""
import hashlib
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

SOURCES = {name: Path(sys.argv.pop(1)).resolve() / 'src' for name in ('python', 'yaml')}
ROOT = Path(__file__).resolve().parents[1]
FILES = (('python', 'scanner.c'), ('yaml', 'scanner.c'), ('yaml', 'parser.c'))


class GrammarFixTests(unittest.TestCase):
    def patch(self, root, language, filename, mode='normal'):
        source = root / 'source with spaces'
        source.mkdir()
        original = (SOURCES[language] / filename).read_bytes()
        if mode == 'changed':
            original += b'\n/* changed upstream source */\n'
        if mode != 'missing':
            (source / filename).write_bytes(original)
        script = root / 'fix.cmake'
        script.write_text(f'''include("{ROOT}/cmake/QoreTreeSitterGrammarFixes.cmake")
qore_treesitter_fixed_source("{'unknown' if mode == 'unknown' else language}" "{filename}" "{source}" actual)
file(WRITE "{root}/result.txt" "${{actual}}")
''')
        result = subprocess.run(['cmake', '-P', str(script)], cwd=root, capture_output=True, text=True)
        if mode != 'missing':
            self.assertEqual(original, (source / filename).read_bytes())
        return result, script

    def test_repeatable_build_tree_fix_and_warning_free_compilation(self):
        for language, filename in FILES:
            with self.subTest(language=language, filename=filename), tempfile.TemporaryDirectory() as tmp:
                root = Path(tmp)
                result, script = self.patch(root, language, filename)
                self.assertEqual(0, result.returncode, result.stdout + result.stderr)
                target = Path((root / 'result.txt').read_text())
                digest = hashlib.sha256(target.read_bytes()).hexdigest()
                stamp = target.stat().st_mtime_ns
                result = subprocess.run(['cmake', '-P', str(script)], cwd=root, capture_output=True, text=True)
                self.assertEqual(0, result.returncode, result.stderr)
                self.assertEqual(digest, hashlib.sha256(target.read_bytes()).hexdigest())
                self.assertEqual(stamp, target.stat().st_mtime_ns)
                result = subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror', '-O2',
                                         '-I', str(SOURCES[language]), '-c', str(target), '-o', str(root / 'test.o')],
                                        capture_output=True, text=True)
                self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_changed_source_is_rejected(self):
        self.assert_rejected('changed', 'Unexpected')

    def test_missing_source_is_rejected(self):
        self.assert_rejected('missing', 'SHA256')

    def test_unknown_fix_is_rejected(self):
        self.assert_rejected('unknown', 'Unknown private grammar source')

    def assert_rejected(self, mode, diagnostic):
        for language, filename in FILES:
            with self.subTest(language=language, filename=filename), tempfile.TemporaryDirectory() as tmp:
                root = Path(tmp)
                result, _ = self.patch(root, language, filename, mode)
                self.assertNotEqual(0, result.returncode)
                self.assertIn(diagnostic, result.stderr)
                self.assertFalse((root / 'fixed-grammars').exists())

    def test_yaml_lexer_preserves_all_states_and_callback_traces(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            result, _ = self.patch(root, 'yaml', 'parser.c')
            self.assertEqual(0, result.returncode, result.stderr)
            original = (SOURCES['yaml'] / 'parser.c').read_text()
            fixed = Path((root / 'result.txt').read_text()).read_text()
            def lexer(source, name):
                start = source.index('static bool ts_lex(')
                end = source.index('\n}\n', start) + 3
                return source[start:end].replace('ts_lex(', name + '(')
            # Compile the original function without warning flags; its known unused
            # macro state is precisely the defect under test. The fixed complete
            # translation unit is compiled with -Wall -Wextra -Werror above.
            test = '#include "tree_sitter/parser.h"\n#include <assert.h>\n#include <string.h>\n'
            test += lexer(original, 'original_lex') + lexer(fixed, 'fixed_lex')
            test += r'''
typedef struct {
    TSLexer lexer;
    bool initial_eof;
    bool after_eof;
    bool advanced;
    char trace[16];
    unsigned count;
} Probe;
static void trace(Probe *p, char event) {
    assert(p->count < sizeof(p->trace));
    p->trace[p->count++] = event;
}
static void advance(TSLexer *lexer, bool skip) {
    Probe *p = (Probe *)lexer;
    trace(p, skip ? 'S' : 'A');
    p->advanced = true;
    lexer->lookahead = 42;
}
static void mark_end(TSLexer *lexer) { trace((Probe *)lexer, 'M'); }
static bool eof(const TSLexer *lexer) {
    Probe *p = (Probe *)lexer;
    trace(p, 'E');
    return p->advanced ? p->after_eof : p->initial_eof;
}
int main(void) {
    for (unsigned state = 0; state <= UINT16_MAX; ++state) {
        for (unsigned flags = 0; flags < 8; ++flags) {
            Probe a = {0};
            a.lexer.lookahead = (flags & 4) ? 0 : 0x10ffff;
            a.lexer.result_symbol = 42;
            a.lexer.advance = advance;
            a.lexer.mark_end = mark_end;
            a.lexer.eof = eof;
            a.initial_eof = (flags & 1) != 0;
            a.after_eof = (flags & 2) != 0;
            Probe b = a;
            assert(original_lex(&a.lexer, (TSStateId)state) == fixed_lex(&b.lexer, (TSStateId)state));
            assert(a.lexer.result_symbol == b.lexer.result_symbol);
            assert(a.lexer.lookahead == b.lexer.lookahead);
            assert(a.advanced == b.advanced);
            assert(a.count == b.count);
            assert(memcmp(a.trace, b.trace, a.count) == 0);
        }
    }
    return 0;
}
'''
            (root / 'equivalence.c').write_text(test)
            subprocess.run(['cc', '-std=c11', '-O2', '-I', str(SOURCES['yaml']), str(root / 'equivalence.c'),
                            '-o', str(root / 'equivalence')], check=True)
            subprocess.run([str(root / 'equivalence')], check=True, timeout=30)


if __name__ == '__main__':
    unittest.main()
