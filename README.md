# Qore tree-sitter Module

Tree-sitter syntax parsing library bindings for the [Qore Programming Language](https://qore.org).

## Description

This module provides bindings to the [tree-sitter](https://tree-sitter.github.io/tree-sitter/) parsing library, enabling fast, incremental syntax parsing for multiple programming languages.

## Supported Languages

- Python
- Java
- JSON
- YAML
- JavaScript
- Kotlin
- TypeScript
- TSX
- Qore

## Features

- **Incremental parsing**: Efficiently re-parse after edits
- **Tree cursors**: Memory-efficient tree traversal
- **Query API**: Pattern matching for syntax highlighting and code analysis
- **Thread-safe**: All classes are thread-safe
- **Grammar parity**: The bundled Qore grammar is intended to mirror the Qore parser to support language servers

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

# Parse Qore code
TreeSitterParser qoreParser("qore");
TreeSitterTree qoreTree = qoreParser.parse("class Test { constructor() {} }");
printf("Qore root: %s\n", qoreTree.getRootNode().getType());
```

## Classes

- **TreeSitterParser**: Parse source code into syntax trees
- **TreeSitterTree**: Represents a parsed syntax tree
- **TreeSitterNode**: A node in the syntax tree
- **TreeSitterCursor**: Efficient tree traversal cursor
- **TreeSitterQuery**: Pattern matching on syntax trees

## License

LGPL 2.1 - see [LICENSE](LICENSE) for details.

## Links

- [Qore Programming Language](https://qore.org)
- [tree-sitter](https://tree-sitter.github.io/tree-sitter/)
