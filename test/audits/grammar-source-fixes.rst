Private grammar source-fix review
=================================

Copyright 2026 Qore Technologies, s.r.o.

Scope: CMake, the pinned Python/YAML private source fixes, regression tests,
README and release notes. The upstream archives remain unchanged.

Validation on Fedora's packaged CPU SDK: Debug native build, 93 Qore cases /
336 assertions, five helper checks, strict API reference and full-suite Valgrind
all pass. Valgrind reports zero errors, no definite/indirect/possible lost blocks,
and no suppressions. It uses the upstream PCRE2 Valgrind-instrumented library
with JIT enabled. Logs are in qore-packaging/results/treesitter-functional-3b.log,
treesitter-grammar-fixes-host-1.log and treesitter-valgrind-3.log.

The helper compares every 16-bit YAML lexer state with eight EOF/lookahead
combinations, including EOF changes after advance, token results and exact
callback traces. Scanner regressions cover quotes, dedentation, scalars, tags,
empty/comment-only input, malformed input and reuse after errors.

.. list-table:: Full audit checklist
   :header-rows: 1

   * - Check
     - Status
     - Evidence

   * - 1. Entry exists in doxygen/lang/120_modules.dox.tmpl (for modules in the Qore repo; N/A for external module repos)
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 2. Entry exists in doxygen/lang/900_release_notes.dox.tmpl (for modules in the Qore repo; external modules have release notes in their .qm)
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 3. qore_user_module() or qore_external_user_module() call in CMakeLists.txt
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 4. Module added to QMOD list in CMakeLists.txt
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 5. .qm file has @section <lowercasemodname>intro as first doc section — must be all lowercase (e.g., avrodataproviderintro, not AvroDataProviderintro)
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 6. %modern in .qm file — no redundant %new-style, %require-types, %strict-args, %enable-all-warnings
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 7. No parse directives (%requires, %modern, %new-style) in separated .qc files (check OUTSIDE of @code blocks only)
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 8. No %include usage (deprecated for modules)
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 9. Copyright 2026 on all new files
     - Pass
     - All new files carry 2026 copyright notices.

   * - 10. Directory layout: .qm inside qlib/<ModuleName>/ directory (not at qlib/<ModuleName>.qm for multi-file modules)
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 11. No second .qm for the same module at qlib/<ModuleName>.qm
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 12. ns=Qore::XX matches the QoreNamespace constructor path
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 13. %modern directive present
     - Pass
     - The changed Qore suite retains %modern.

   * - 14. Executable permission set (chmod +x)
     - Pass
     - The Qore suite and Python test have executable mode.

   * - 15. Uses %prepend-module-path  before %requires for in-repo modules (Qore and Qore modules only; not Qorus)
     - Pass
     - The suite prepends its native build directory; qualification explicitly preloads the exact debug qmod.

   * - 16. External module dependencies use %try-module — except modules delivered with the project itself (Qore ex: DataProvider, ConnectionProvider, QUnit, etc.) which use hard %requires
     - Pass
     - No new imports; treesitter is the module under test and QUnit/FsUtil come from Qore.

   * - 17. No filesystem operations (fopen, open, creat, unlink, remove, rename, mkdir, rmdir, stat, chmod) without sandbox checks
     - Pass
     - The native grammar edits perform no filesystem operations; CMake writes only private build copies.

   * - 18. No network operations (connect, bind, socket, getaddrinfo, gethostbyname) without sandbox checks
     - Pass
     - No native network operations added.

   * - 19. If filesystem/network ops exist, verify QoreSandboxManagerHelper usage
     - N/A
     - No native filesystem or network operation added.

   * - 20. No File::, Dir::, Socket::, HTTPClient:: usage without justification
     - Pass
     - New Qore test cases parse in-memory strings only.

   * - 21. All for/while loops that could iterate >100 times have qore_check_cancel() checks
     - Pass
     - The EOF lexer has at most one transition; no runtime loop added. Exhaustive state iteration exists only in a finite native test.

   * - 22. Uses qore_check_cancel() (NOT deprecated qore_check_io_interrupt())
     - N/A
     - No cancellation check changed or added.

   * - 23. Check frequency: every 100 iterations for tight loops, every 10 for expensive iterations
     - N/A
     - No unbounded runtime loop added.

   * - 24. No blocking operations without cancellation support
     - Pass
     - Native edits add no blocking operations; subprocess equivalence test has a deadline.

   * - 25. Every action has display_name, short_desc (plain text, <80 chars), desc (markdown)
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 26. Every action has options populated via getActionOptionFromFields() — without this, the action shows an empty, unusable form
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 27. Every action has output_type set to a typed data type constant (e.g., MyResponseDataType) — not omitted
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 28. DPAT_API actions: provider has "supports_request": True and implements doRequestImpl()
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 29. DPAT_FIND actions: every option exists in SearchOptions, getRecordTypeImpl() returns *hash<string, AbstractDataField>
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 30. Scheme-based apps (with "scheme" in registerApp): actions use "path" and do NOT use "cls" — having both scheme and cls causes a runtime error
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 31. Single-key hash slices use trailing comma: Fields{"key",} (without trailing comma, Fields{"key"} returns the value, not a hash)
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 32. Typed data type classes exist for request and response types — inherit HashDataType, have const Fields hash, call addQoreFields(Fields) in constructor, export public constant at bottom (e.g., public const MyDataType = new MyDataType();)
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 33. Request/input types use public Fields (enables ClassName::Fields in action registration)
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 34. Response/output types use private Fields
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 35. Each field in data types has display_name, type, and desc (markdown-formatted)
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 36. Input fields have example_value where useful (string fields, endpoint URIs, SQL queries, etc.)
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 37. Fields with finite allowed values use allowed_values with AllowedValueInfo containing both value and display_name (Title Case, human-readable) — never bare values, never described only in text
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 38. Password/secret fields have "sensitive": True
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 39. groups uses AppGroup enum values from qlib/DataProvider/AppGroup.qc
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 40. App logo stored as separate file, loaded at module level in Priv namespace
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 41. App desc uses markdown: bullet list of capabilities, links to project website, business-language explanation of value
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 42. display_name is user-friendly ("Apache Avro" not "avro")
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 43. short_desc is plain text, under 80 chars, single sentence — no markdown
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 44. desc uses markdown: backticks for code/field refs ( field_name ,  True ,  pdf ), \n\n for paragraphs, -  bullet lists for enumerations, bold for caveats
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 45. Descriptions use plain business language relating to common challenges — not just technical "what" but "why" and "when to use"
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 46. No bare True/False/NOTHING — must be backtick-wrapped in desc
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 47. No bare field/option names in prose — must use backticks
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 48. Long descriptions (>500 chars) use bold section headers and bullet lists
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 49. Factory registration in Qore repo: every factory name registered in qlib/DataProvider/DataProvider.qc → FactoryMap (without this, module loads but doesn't appear in Qorus apps)
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 50. getRecordTypeImpl() signature: must be private *hash<string, AbstractDataField> getRecordTypeImpl(*hash<auto> search_options) — NOT returning *AbstractDataProviderType
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 51. Dependency JARs committed (for JNI modules): JAR files in qlib/*/jar/ may be gitignored — use git add -f to ensure they're tracked, otherwise CI compilation fails
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 52. JAR install rules in CMakeLists.txt for all dependency JARs
     - N/A
     - No new Qore module/class, DataProvider declaration, registration or Java dependency in this change.

   * - 53. No workarounds: No TODOs, FIXMEs, stubs, or partially-implemented features
     - Pass
     - Each diagnostic is fixed at its source: unused pop results, dead whitespace state, intentional fallthrough comments and unnecessary generic lexer state. No warning flags are disabled in the production build.

   * - 54. Exception safety: C++ uses ReferenceHolder for Qore allocations, std::unique_ptr for C++ allocations, *xsink checked after every fallible operation
     - Pass
     - Native edits add no ownership or allocations. Test temporary directories use context managers; failed source validation writes no replacement.

   * - 55. Thread safety: All mutable shared state protected by std::lock_guard<std::mutex> or documented as immutable-after-construction
     - Pass
     - No shared mutable runtime state added; scanner ownership is unchanged.

   * - 56. Type safety: Strongly-typed code<return(args)> instead of untyped code; static_cast instead of C casts; typed hashdecls for results; enums where appropriate
     - Pass
     - Qore tests use typed strings and lists; lexer retains TSLexer/TSStateId/bool. C test uses C casts for an embedded-first-member probe.

   * - 57. Performance: No O(n²) where O(n) is possible; no unnecessary copies; coordinate descent uses incremental residuals not full matrix multiply
     - Pass
     - Build copies only three affected files; runtime removes unused work without new allocations.

   * - 58. Error handling: All inputs validated (dimensions, empty data, unfitted models); C++ I/O handles EAGAIN/EINTR if applicable
     - Pass
     - Hash checks reject changed inputs; missing/unknown sources and repeat configuration are tested.

   * - 59. Documentation: Doxygen @param, @return, @throw on all public methods; @par Example with realistic business scenarios; @note for important caveats
     - Pass
     - README and release notes explain the fixes and give the standalone test command; strict Doxygen passes.

   * - 60. QPP flags: [flags=CONSTANT] on methods that never throw; [flags=RET_VALUE_ONLY] on methods that throw but have no side effects
     - N/A
     - No QPP signature or flag changed.

   * - 61. Security: No user-controlled format strings; no buffer overflows; bounds checking on array indices; no credentials in code
     - Pass
     - No credentials or formatting of untrusted text added; equivalence trace checks its bounds.

   * - 62. Correctness: Algorithms verified against reference implementations; edge cases tested (empty data, single sample, all-zero features)
     - Pass
     - 93 cases/336 assertions pass normally and under Valgrind with zero errors and no lost blocks. Five helper tests include 524288 original/fixed lexer comparisons and -Wall -Wextra -Werror compilation.
