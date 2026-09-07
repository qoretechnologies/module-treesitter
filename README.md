# Qore tree-sitter Module

Tree-sitter syntax parsing library bindings for the [Qore Programming Language](https://qore.org).

## Description

This module provides bindings to the [tree-sitter](https://tree-sitter.github.io/tree-sitter/) parsing library, enabling fast, incremental syntax parsing for multiple programming languages.

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

**Note:** The Qore tree-sitter grammar is maintained in the [Qore repository](https://github.com/qoretechnologies/qore) as part of the `astparser` module (`modules/astparser/grammars/tree-sitter-qore/`).
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
- CMake 3.14+
- C++11 compiler
- tree-sitter CLI (`npm install -g tree-sitter-cli`) - needed at build time for SQL grammar generation

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

LGPL 2.1 or MIT - see [LICENSE](LICENSE) for details.

## Links

- [Qore Programming Language](https://qore.org)
- [tree-sitter](https://tree-sitter.github.io/tree-sitter/)
