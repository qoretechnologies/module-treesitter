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

**Note:** The Qore tree-sitter grammar is maintained in the [Qore repository](https://github.com/qoretechnologies/qore) as part of the `astparser` module (`modules/astparser/grammars/tree-sitter-qore/`).

## Features

- **Incremental parsing**: Efficiently re-parse after edits
- **Tree cursors**: Memory-efficient tree traversal
- **Query API**: Pattern matching for syntax highlighting and code analysis
- **Thread-safe**: All classes are thread-safe
## Building

```bash
mkdir build && cd build
cmake ..
make
make install
```

## Requirements

- Qore 1.0+
- CMake 3.15+
- C++17 compiler

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
