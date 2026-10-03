# Qore tree-sitter Module

Tree-sitter syntax parsing library bindings for the [Qore Programming Language](https://qore.org).

## Description

This module provides bindings to the [tree-sitter](https://tree-sitter.github.io/tree-sitter/) parsing library, enabling fast, incremental syntax parsing for multiple programming languages.

Nodes and cursors keep their underlying syntax tree alive, so chained calls such
as `parser.parse(source).getRootNode().hasError()` are supported. Copied nodes
retain the complete source; resetting a traversal or query cursor also switches
to the new tree's source.

## Supported Grammars

- Python
- Java
- JSON
- YAML
- JavaScript
- Kotlin
- TypeScript
- TSX
- SQL
- Markdown
- Qore

**Note:** The Qore tree-sitter grammar is provided by the `astparser` module built into Qore 3.0 and later.
The `treesitter` module reuses that grammar, maintained in the [Qore repository](https://github.com/qoretechnologies/qore)
at `modules/astparser/grammars/tree-sitter-qore/`, and exposes it as the `qore` parser language.
The build uses the installed grammar's `parser.c` and, when present, `scanner.c` for brace-delimited regular expressions
such as `value =~ m{^abc$}i;`. Install the matching astparser grammar files before rebuilding this module.

## Features

- **Incremental parsing**: Efficiently re-parse after edits with change detection (`hasChanges()`)
- **Tree cursors**: Memory-efficient tree traversal
- **Query API**: Pattern matching for syntax highlighting and code analysis
- **Query predicates**: Automatic evaluation of `#eq?`, `#not-eq?`, `#match?`, `#not-match?`, `#any-of?`, `#not-any-of?`
- **Range-scoped queries**: Limit query execution to byte or point ranges for viewport-only highlighting
- **Multi-language support**: `setIncludedRanges()` for parsing embedded languages (e.g., JS in HTML)
- **Bundled query files**: Access `highlights.scm`, `locals.scm`, `injections.scm`, `folds.scm`, `indents.scm`, and `tags.scm` via `getQuery()`
- **Thread-safe**: All classes are thread-safe
## Building

```bash
mkdir build && cd build
cmake ..
make
make install
```

## Requirements

- Qore 2.0+
- CMake 3.21+
- C++11 compiler
- Optional system tree-sitter runtime >= 0.26.5; otherwise CMake fetches the pinned runtime
- Qore SDK with the installed Qore grammar; SQL uses the committed generated parser

## Example

```qore
%requires treesitter

# Parse Python code
TreeSitterParser parser("python");
TreeSitterTree tree = parser.parse("def hello(): pass");
TreeSitterNode root = tree.getRootNode();

printf("Root type: %s\n", root.getType());
printf("S-expression: %s\n", root.toSexp());
```

## Classes

- **TreeSitterParser**: Parse source code into syntax trees
- **TreeSitterTree**: Represents a parsed syntax tree
- **TreeSitterNode**: A node in the syntax tree
- **TreeSitterCursor**: Efficient tree traversal cursor
- **TreeSitterQuery**: Pattern matching on syntax trees

## License

Native implementation: LGPL 2.1 or later; see [COPYING.LGPL](COPYING.LGPL).
Grammar components retain their upstream licenses. General support files retain
the previously documented MIT alternative.

## Links

- [Qore Programming Language](https://qore.org)
- [tree-sitter](https://tree-sitter.github.io/tree-sitter/)

Set `QORE_TREESITTER_QUERY_DIR` to a directory containing language subdirectories
to use alternate query files. Queries are cached by canonical filename; sandbox
filesystem policy applies to every call, including cached results.

The pinned Python 0.25.0 and YAML 0.7.2 grammars have small, hash-checked fixes
applied to private build-tree copies. Scanner stack pops explicitly discard
unused values; YAML removes an unused whitespace variable and documents its
intentional tag-scanner fallthroughs. Its EOF-only internal lexer omits generic
character-lookahead state while preserving token and callback behavior.
Downloaded grammar sources remain unchanged. Updating either grammar requires
reviewing the source hashes and fixes together.

To validate these fixes against unpacked pinned grammar sources:

```sh
python3 test/test_grammar_fixes.py /path/to/tree-sitter-python-0.25.0 /path/to/tree-sitter-yaml-0.7.2 -v
```

The checks compile every fixed source with warnings as errors, verify immutable
inputs and repeatable outputs, reject missing or unexpected sources, and compare
all 65,536 YAML lexer states under eight EOF/lookahead combinations. The Qore
suite additionally covers indentation, quotes, block scalars, tags, malformed
input and parser recovery.
