/**
 * @file Qore grammar for tree-sitter
 * @author Qore Technologies
 * @license LGPL-2.1
 * @see {@link https://qore.org|Qore Programming Language}
 */

/// <reference types="tree-sitter-cli/dsl" />
// @ts-check

const PREC = {
  // Operator precedence (based on Qore's parser)
  COMMA: 1,
  ASSIGN: 2,
  TERNARY: 3,
  NULL_COALESCE: 4,
  LOGICAL_OR: 5,
  LOGICAL_AND: 6,
  BITWISE_OR: 7,
  BITWISE_XOR: 8,
  BITWISE_AND: 9,
  EQUALITY: 10,
  COMPARISON: 11,
  SHIFT: 12,
  ADD: 13,
  MULTIPLY: 14,
  UNARY: 15,
  POSTFIX: 16,
  CALL: 17,
  MEMBER: 18,
};

module.exports = grammar({
  name: 'qore',

  extras: $ => [
    $.comment,
    $.line_comment,
    /\s/,
  ],

  // External scanner not yet implemented
  // externals: $ => [
  //   $.regex_pattern,
  // ],

  // Note: word token is used for keyword extraction, must be terminal
  // word: $ => $.identifier,

  conflicts: $ => [
    [$.hash_literal, $.block],
    [$.variable_declarator, $.primary_expression],
    [$.module_name, $.scoped_identifier],
    [$.parenthesized_expression, $.list_literal],
    [$.function_declaration, $.closure_expression],
    [$.argument_list, $.parameter_list],
    [$.parameter, $.primary_expression],
  ],

  rules: {
    // Entry point
    source_file: $ => repeat($._top_level_item),

    _top_level_item: $ => choice(
      $.parse_directive,
      $.namespace_declaration,
      $.class_declaration,
      $.function_declaration,
      $.constant_declaration,
      $.global_variable_declaration,
      $.hashdecl_declaration,
      $.typedef_declaration,
      $.enum_declaration,
      $._statement,
    ),

    // ==================== Parse Directives ====================
    parse_directive: $ => seq(
      '%',
      choice(
        // Style directives
        'new-style',
        'old-style',
        // Type checking
        'require-types',
        'strict-types',
        'require-prototypes',
        // Variable handling
        'require-our',
        'assume-local',
        'assume-global',
        'allow-bare-refs',
        // Boolean evaluation
        'perl-bool-eval',
        'strict-bool-eval',
        // Debugging
        'enable-debug',
        'disable-debug',
        // Module directives
        seq('requires', $.module_name),
        seq('try-module', $.module_name),
        // Other common directives
        'strict-args',
        'no-global-vars',
        'no-child-restrictions',
        'no-typedef',
        'no-enum',
        'no-transient',
        'lockdown',
        'exec-class',
        'modern',
        'enable-all-warnings',
        'push-parse-options',
        'pop-parse-options',
        // Conditional parsing directives
        seq('ifdef', $.identifier),
        seq('ifndef', $.identifier),
        seq('if', '(', $._expression, ')'),
        seq('elif', '(', $._expression, ')'),
        'else',
        'endif',
      ),
      optional($.newline),
    ),

    module_name: $ => choice(
      $.identifier,
      $.scoped_identifier,
    ),

    // ==================== Namespace ====================
    namespace_declaration: $ => seq(
      optional($.modifiers),
      'namespace',
      field('name', choice($.identifier, $.scoped_identifier)),
      choice(
        seq('{', repeat($._namespace_item), '}'),
        ';',
      ),
    ),

    _namespace_item: $ => choice(
      $.namespace_declaration,
      $.class_declaration,
      $.function_declaration,
      $.constant_declaration,
      $.global_variable_declaration,
      $.hashdecl_declaration,
      $.typedef_declaration,
      $.enum_declaration,
    ),

    // ==================== Class ====================
    class_declaration: $ => seq(
      optional($.modifiers),
      'class',
      field('name', $.identifier),
      optional($.superclass_list),
      '{',
      repeat($._class_item),
      '}',
    ),

    superclass_list: $ => seq(
      'inherits',
      commaSep1($.superclass),
    ),

    superclass: $ => seq(
      optional($.access_modifier),
      $.scoped_identifier,
    ),

    _class_item: $ => choice(
      $.member_declaration,
      $.method_declaration,
      $.constructor_declaration,
      $.destructor_declaration,
      $.copy_method,
      $.constant_declaration,
      $.member_group,
    ),

    member_group: $ => seq(
      $.access_modifier,
      '{',
      repeat($.member_declaration),
      '}',
    ),

    member_declaration: $ => seq(
      optional($.modifiers),
      field('type', optional($.type)),
      field('name', $.identifier),
      optional(seq('=', field('default', $._expression))),
      ';',
    ),

    method_declaration: $ => seq(
      optional($.modifiers),
      optional(field('return_type', $.type)),
      field('name', $.identifier),
      $.parameter_list,
      choice(
        $.block,
        ';',
      ),
    ),

    constructor_declaration: $ => seq(
      optional($.modifiers),
      'constructor',
      $.parameter_list,
      optional($.base_class_constructor_calls),
      choice(
        $.block,
        ';',
      ),
    ),

    destructor_declaration: $ => seq(
      optional($.modifiers),
      'destructor',
      '(',
      ')',
      choice(
        $.block,
        ';',
      ),
    ),

    copy_method: $ => seq(
      optional($.modifiers),
      'copy',
      '(',
      ')',
      choice(
        $.block,
        ';',
      ),
    ),

    base_class_constructor_calls: $ => seq(
      ':',
      commaSep1($.base_class_constructor_call),
    ),

    base_class_constructor_call: $ => seq(
      $.scoped_identifier,
      $.argument_list,
    ),

    // ==================== Function ====================
    function_declaration: $ => seq(
      optional($.modifiers),
      optional(field('return_type', $.type)),
      choice('sub', $.identifier),
      field('name', optional($.identifier)),
      $.parameter_list,
      choice(
        $.block,
        ';',
      ),
    ),

    parameter_list: $ => seq(
      '(',
      optional(commaSep($.parameter)),
      ')',
    ),

    parameter: $ => seq(
      optional($.modifiers),
      optional(field('type', $.type)),
      field('name', $.identifier),
      optional(seq('=', field('default', $._expression))),
    ),

    // ==================== Constants and Variables ====================
    constant_declaration: $ => seq(
      optional($.modifiers),
      'const',
      field('name', $.identifier),
      '=',
      field('value', $._expression),
      ';',
    ),

    global_variable_declaration: $ => seq(
      choice('our', 'my'),
      optional(field('type', $.type)),
      commaSep1($.variable_declarator),
      ';',
    ),

    variable_declarator: $ => choice(
      seq(
        field('name', $.variable_name),
        optional(seq('=', field('value', $._expression))),
      ),
      // Object construction: identifier(args) or just identifier
      seq(
        field('name', $.identifier),
        optional(choice(
          seq('=', field('value', $._expression)),
          $.argument_list,  // Constructor arguments
        )),
      ),
    ),

    // ==================== Hashdecl ====================
    hashdecl_declaration: $ => seq(
      optional($.modifiers),
      'hashdecl',
      field('name', $.identifier),
      optional($.superclass_list),
      '{',
      repeat($.hashdecl_member),
      '}',
    ),

    hashdecl_member: $ => seq(
      optional(field('type', $.type)),
      field('name', $.identifier),
      optional(seq('=', field('default', $._expression))),
      ';',
    ),

    // ==================== Typedef ====================
    typedef_declaration: $ => seq(
      optional($.modifiers),
      'typedef',
      field('type', $.type),
      field('name', $.identifier),
      ';',
    ),

    // ==================== Enum ====================
    enum_declaration: $ => seq(
      optional($.modifiers),
      'enum',
      field('name', $.identifier),
      optional(seq(':', field('base_type', $.type))),
      '{',
      optional(commaSep1($.enum_member)),
      optional(','),  // trailing comma allowed
      '}',
    ),

    enum_member: $ => seq(
      field('name', $.identifier),
      optional(seq('=', field('value', $._expression))),
    ),

    // ==================== Statements ====================
    _statement: $ => choice(
      $.expression_statement,
      $.block,
      $.if_statement,
      $.while_statement,
      $.do_while_statement,
      $.for_statement,
      $.foreach_statement,
      $.switch_statement,
      $.try_statement,
      $.return_statement,
      $.throw_statement,
      $.break_statement,
      $.continue_statement,
      $.on_exit_statement,
      $.context_statement,
      $.summarize_statement,
      $.local_variable_declaration,
    ),

    expression_statement: $ => seq(
      $._expression,
      ';',
    ),

    block: $ => seq(
      '{',
      repeat($._statement),
      '}',
    ),

    if_statement: $ => prec.right(seq(
      'if',
      '(',
      field('condition', $._expression),
      ')',
      field('consequence', $._statement),
      optional(seq(
        'else',
        field('alternative', $._statement),
      )),
    )),

    while_statement: $ => seq(
      'while',
      '(',
      field('condition', $._expression),
      ')',
      field('body', $._statement),
    ),

    do_while_statement: $ => seq(
      'do',
      field('body', $._statement),
      'while',
      '(',
      field('condition', $._expression),
      ')',
      ';',
    ),

    for_statement: $ => seq(
      'for',
      '(',
      field('init', optional($._expression)),
      ';',
      field('condition', optional($._expression)),
      ';',
      field('update', optional($._expression)),
      ')',
      field('body', $._statement),
    ),

    foreach_statement: $ => seq(
      'foreach',
      field('variable', $.identifier),
      'in',
      '(',
      field('iterable', $._expression),
      ')',
      field('body', $._statement),
    ),

    switch_statement: $ => seq(
      'switch',
      '(',
      field('value', $._expression),
      ')',
      '{',
      repeat($.switch_case),
      optional($.default_case),
      '}',
    ),

    switch_case: $ => seq(
      'case',
      field('value', $._expression),
      ':',
      repeat($._statement),
    ),

    default_case: $ => seq(
      'default',
      ':',
      repeat($._statement),
    ),

    try_statement: $ => seq(
      'try',
      field('body', $.block),
      repeat1($.catch_clause),
    ),

    catch_clause: $ => seq(
      'catch',
      '(',
      optional(field('type', $.type)),
      field('parameter', $.identifier),
      ')',
      field('body', $.block),
    ),

    return_statement: $ => seq(
      'return',
      optional($._expression),
      ';',
    ),

    throw_statement: $ => seq(
      'throw',
      $._expression,
      ';',
    ),

    break_statement: $ => seq('break', ';'),
    continue_statement: $ => seq('continue', ';'),

    on_exit_statement: $ => seq(
      'on_exit',
      $.block,
    ),

    context_statement: $ => seq(
      'context',
      optional(field('name', $.identifier)),
      '(',
      field('expression', $._expression),
      ')',
      optional($.context_modifiers),
      field('body', $._statement),
    ),

    context_modifiers: $ => repeat1(choice(
      $.where_clause,
      $.sortby_clause,
    )),

    where_clause: $ => seq('where', '(', $._expression, ')'),
    sortby_clause: $ => seq(choice('sortBy', 'sortDescendingBy'), '(', $._expression, ')'),

    summarize_statement: $ => seq(
      'summarize',
      '(',
      field('expression', $._expression),
      ')',
      'by',
      '(',
      commaSep1($._expression),
      ')',
      field('body', $._statement),
    ),

    local_variable_declaration: $ => seq(
      optional(field('type', $.type)),
      commaSep1($.variable_declarator),
      ';',
    ),

    // ==================== Expressions ====================
    _expression: $ => choice(
      $.assignment_expression,
      $.ternary_expression,
      $.binary_expression,
      $.unary_expression,
      $.postfix_expression,
      $.primary_expression,
    ),

    assignment_expression: $ => prec.right(PREC.ASSIGN, seq(
      field('left', $._expression),
      field('operator', choice(
        '=', '+=', '-=', '*=', '/=', '%=',
        '&=', '|=', '^=', '<<=', '>>=',
        '??=', ':=',
      )),
      field('right', $._expression),
    )),

    ternary_expression: $ => prec.right(PREC.TERNARY, seq(
      field('condition', $._expression),
      '?',
      field('consequence', $._expression),
      ':',
      field('alternative', $._expression),
    )),

    binary_expression: $ => choice(
      // Null coalescing
      prec.left(PREC.NULL_COALESCE, seq($._expression, choice('??', '?*'), $._expression)),
      // Logical
      prec.left(PREC.LOGICAL_OR, seq($._expression, choice('||', 'or'), $._expression)),
      prec.left(PREC.LOGICAL_AND, seq($._expression, choice('&&', 'and'), $._expression)),
      // Bitwise
      prec.left(PREC.BITWISE_OR, seq($._expression, '|', $._expression)),
      prec.left(PREC.BITWISE_XOR, seq($._expression, '^', $._expression)),
      prec.left(PREC.BITWISE_AND, seq($._expression, '&', $._expression)),
      // Equality
      prec.left(PREC.EQUALITY, seq($._expression, choice('==', '!=', '===', '!==', '=~', '!~'), $._expression)),
      // Comparison
      prec.left(PREC.COMPARISON, seq($._expression, choice('<', '>', '<=', '>=', '<=>'), $._expression)),
      prec.left(PREC.COMPARISON, seq($._expression, 'instanceof', $._expression)),
      // Shift
      prec.left(PREC.SHIFT, seq($._expression, choice('<<', '>>'), $._expression)),
      // Arithmetic
      prec.left(PREC.ADD, seq($._expression, choice('+', '-'), $._expression)),
      prec.left(PREC.MULTIPLY, seq($._expression, choice('*', '/', '%'), $._expression)),
      // Range
      prec.left(PREC.ADD, seq($._expression, '..', $._expression)),
    ),

    unary_expression: $ => prec.right(PREC.UNARY, seq(
      field('operator', choice(
        '!', 'not', '~', '-', '+', '\\',
        '++', '--',
        'background',
        'delete', 'remove',
        'exists', 'elements', 'keys',
        'shift', 'pop',
        'chomp', 'trim',
        'new',
      )),
      field('operand', $._expression),
    )),

    postfix_expression: $ => prec.left(PREC.POSTFIX, seq(
      field('operand', $._expression),
      field('operator', choice('++', '--')),
    )),

    primary_expression: $ => choice(
      $.identifier,
      $.variable_name,
      $.scoped_identifier,
      $.literal,
      $.string,
      $.list_literal,
      $.hash_literal,
      $.closure_expression,
      $.call_expression,
      $.member_expression,
      $.index_expression,
      $.cast_expression,
      $.parenthesized_expression,
      $.implicit_argument,
      $.context_reference,
      $.regex,
      // Higher-order functions
      $.map_expression,
      $.select_expression,
      $.foldl_expression,
      $.foldr_expression,
    ),

    call_expression: $ => prec(PREC.CALL, seq(
      field('function', choice($.identifier, $.scoped_identifier, $.member_expression)),
      $.argument_list,
    )),

    argument_list: $ => seq(
      '(',
      optional(commaSep($._expression)),
      ')',
    ),

    member_expression: $ => prec.left(PREC.MEMBER, seq(
      field('object', $._expression),
      '.',
      field('member', $.identifier),
    )),

    index_expression: $ => prec.left(PREC.MEMBER, seq(
      field('object', $._expression),
      '[',
      field('index', $._expression),
      ']',
    )),

    cast_expression: $ => seq(
      '<',
      field('type', $.type),
      '>',
      field('value', $._expression),
    ),

    parenthesized_expression: $ => seq('(', $._expression, ')'),

    closure_expression: $ => seq(
      optional(field('return_type', $.type)),
      'sub',
      $.parameter_list,
      $.block,
    ),

    // Higher-order functions
    map_expression: $ => seq(
      'map',
      field('expression', $._expression),
      ',',
      field('list', $._expression),
    ),

    select_expression: $ => seq(
      'select',
      field('expression', $._expression),
      ',',
      field('list', $._expression),
    ),

    foldl_expression: $ => seq(
      'foldl',
      field('expression', $._expression),
      ',',
      field('list', $._expression),
    ),

    foldr_expression: $ => seq(
      'foldr',
      field('expression', $._expression),
      ',',
      field('list', $._expression),
    ),

    implicit_argument: $ => /\$\d+/,

    context_reference: $ => seq('%', $.identifier),

    // ==================== Literals ====================
    literal: $ => choice(
      $.integer,
      $.float,
      $.number,
      $.boolean,
      $.null,
      $.nothing,
      $.date,
      $.binary,
    ),

    integer: $ => token(choice(
      /[0-9]+/,
      /0x[0-9a-fA-F]+/,
      /0o[0-7]+/,
      /0b[01]+/,
    )),

    float: $ => token(/[0-9]+\.[0-9]+([eE][+-]?[0-9]+)?/),

    number: $ => token(/[0-9]+(\.[0-9]+)?n/),

    boolean: $ => choice('True', 'False'),

    null: $ => 'NULL',
    nothing: $ => 'NOTHING',

    date: $ => token(seq(
      /[0-9]{4}-[0-9]{2}-[0-9]{2}/,
      optional(seq(
        /[T ]/,
        /[0-9]{2}:[0-9]{2}:[0-9]{2}/,
        optional(/\.[0-9]+/),
      )),
    )),

    binary: $ => token(/<[0-9a-fA-F]*>/),

    // ==================== Strings ====================
    string: $ => choice(
      $.single_quoted_string,
      $.double_quoted_string,
    ),

    single_quoted_string: $ => seq(
      "'",
      repeat(choice(
        $.escape_sequence,
        /[^'\\]+/,
      )),
      "'",
    ),

    double_quoted_string: $ => seq(
      '"',
      repeat(choice(
        $.escape_sequence,
        $.string_interpolation,
        /[^"\\$]+/,
      )),
      '"',
    ),

    escape_sequence: $ => token.immediate(seq(
      '\\',
      choice(
        /[nrtbfv0\\'"]/,
        /x[0-9a-fA-F]{2}/,
        /u[0-9a-fA-F]{4}/,
      ),
    )),

    string_interpolation: $ => seq(
      '$',
      choice(
        $.identifier,
        seq('{', $._expression, '}'),
        seq('(', $._expression, ')'),
      ),
    ),

    // ==================== Collections ====================
    list_literal: $ => seq(
      '(',
      optional(commaSep1($._expression)),
      ')',
    ),

    hash_literal: $ => seq(
      '{',
      optional(commaSep($.hash_entry)),
      '}',
    ),

    hash_entry: $ => seq(
      field('key', choice($.string, $.identifier)),
      ':',
      field('value', $._expression),
    ),

    // ==================== Regex ====================
    regex: $ => choice(
      $.regex_literal,
      $.regex_subst,
      $.regex_trans,
    ),

    regex_literal: $ => seq(
      '/',
      $.regex_pattern,
      '/',
      optional($.regex_flags),
    ),

    regex_subst: $ => seq(
      's/',
      $.regex_pattern,
      '/',
      $.regex_replacement,
      '/',
      optional($.regex_flags),
    ),

    regex_trans: $ => seq(
      'tr/',
      $.regex_pattern,
      '/',
      $.regex_replacement,
      '/',
    ),

    regex_pattern: $ => /[^\/\n]*/,
    regex_replacement: $ => /[^\/\n]*/,
    regex_flags: $ => /[gimxsun]+/,

    // ==================== Types ====================
    type: $ => choice(
      $.simple_type,
      $.complex_type,
      $.nullable_type,
    ),

    simple_type: $ => choice(
      'int',
      'float',
      'number',
      'bool',
      'string',
      'date',
      'binary',
      'hash',
      'list',
      'object',
      'code',
      'reference',
      'nothing',
      'any',
      'auto',
      'data',
      'softint',
      'softfloat',
      'softnumber',
      'softbool',
      'softstring',
      'softdate',
      'softlist',
      'timeout',
      $.scoped_identifier,
    ),

    complex_type: $ => choice(
      // hash<type>, list<type>, softlist<type>, enum<type>
      seq(
        choice('hash', 'list', 'softlist', 'enum'),
        '<',
        $.type,
        '>',
      ),
      // union<type1, type2, ...>
      seq(
        'union',
        '<',
        commaSep1($.type),
        '>',
      ),
      // code<return_type(param_types...)> or code<return_type()>
      seq(
        'code',
        '<',
        $.type,  // return type
        '(',
        optional($.code_param_types),
        ')',
        '>',
      ),
    ),

    // Parameter types for code<> signature, supporting varargs
    code_param_types: $ => choice(
      // Just varargs: code<int(...)>
      '...',
      // Types optionally followed by varargs: code<int(string, ...)>
      seq(
        commaSep1($.type),
        optional(seq(',', '...')),
      ),
    ),

    nullable_type: $ => seq('*', $.type),

    // ==================== Modifiers ====================
    modifiers: $ => repeat1($.modifier),

    modifier: $ => choice(
      $.access_modifier,
      'abstract',
      'final',
      'static',
      'synchronized',
      'deprecated',
      'transient',
    ),

    access_modifier: $ => choice(
      'public',
      'private',
      'private:internal',
      'private:hierarchy',
    ),

    // ==================== Identifiers ====================
    identifier: $ => /[a-zA-Z_][a-zA-Z0-9_]*/,

    variable_name: $ => seq('$', /[a-zA-Z_][a-zA-Z0-9_]*/),

    scoped_identifier: $ => prec.left(seq(
      optional('::'),
      $.identifier,
      repeat1(seq('::', $.identifier)),
    )),

    // ==================== Comments ====================
    comment: $ => token(seq(
      '/*',
      /[^*]*\*+([^/*][^*]*\*+)*/,
      '/',
    )),

    line_comment: $ => token(seq('#', /.*/)),

    newline: $ => /\r?\n/,
  },
});

/**
 * Creates a rule for comma-separated items with at least one item.
 */
function commaSep1(rule) {
  return seq(rule, repeat(seq(',', rule)));
}

/**
 * Creates a rule for comma-separated items (zero or more).
 */
function commaSep(rule) {
  return optional(commaSep1(rule));
}
