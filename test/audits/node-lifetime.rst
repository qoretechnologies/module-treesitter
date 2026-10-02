Tree-sitter node and cursor lifetime review
==========================================

Copyright 2026 Qore Technologies, s.r.o.

Root cause: TSNode borrows its TSTree, but native node wrappers retained only
source text. Releasing a temporary Qore tree left the node pointing at freed
memory. Final Leap RPM qualification reproduced the crash in ts_node_has_error.
Nodes now retain a shared reference to the native tree owner; traversal and
query cursors retain that owner as well. Node copies preserve the complete
source; cursor reset updates its owner, and copied cursors retain independent
positions. Child/result construction releases partial output on exceptions or
cancellation. Traversal cursor operations are serialized with a mutex.

Validation: qore-packaging/results/{target}-treesitter-lifetime-final-tests-3.json
and corresponding logs for Fedora, Leap and EL10. All 99 cases / 371 assertions
pass both normally and under Valgrind. QORE_PCRE2_NO_JIT=1 uses the documented
interpreter mode for Valgrind; no memory suppressions are used. Earlier native
qualification also passed with normal PCRE2 JIT. All three documentation builds
pass. Existing malformed-input regressions remain enabled.

.. list-table:: Full audit checklist
   :header-rows: 1

   * - Check
     - Status
     - Evidence

   * - 1. Entry exists in doxygen/lang/120_modules.dox.tmpl (for modules in the Qore repo; N/A for external module repos)
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 2. Entry exists in doxygen/lang/900_release_notes.dox.tmpl (for modules in the Qore repo; external modules have release notes in their .qm)
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 3. qore_user_module() or qore_external_user_module() call in CMakeLists.txt
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 4. Module added to QMOD list in CMakeLists.txt
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 5. .qm file has @section <lowercasemodname>intro as first doc section — must be all lowercase (e.g., avrodataproviderintro, not AvroDataProviderintro)
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 6. %modern in .qm file — no redundant %new-style, %require-types, %strict-args, %enable-all-warnings
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 7. No parse directives (%requires, %modern, %new-style) in separated .qc files (check OUTSIDE of @code blocks only)
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 8. No %include usage (deprecated for modules)
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 9. Copyright 2026 on all new files
     - Pass
     - All new files have 2026 notices; modified native files already carry 2026.

   * - 10. Directory layout: .qm inside qlib/<ModuleName>/ directory (not at qlib/<ModuleName>.qm for multi-file modules)
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 11. No second .qm for the same module at qlib/<ModuleName>.qm
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 12. ns=Qore::XX matches the QoreNamespace constructor path
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 13. %modern directive present
     - Pass
     - The expanded treesitter.qtest uses %modern.

   * - 14. Executable permission set (chmod +x)
     - Pass
     - treesitter.qtest retains its executable bit.

   * - 15. Uses %prepend-module-path  before %requires for in-repo modules (Qore and Qore modules only; not Qorus)
     - Pass
     - The native build path precedes module requirements; qualification explicitly preloads the freshly built binary.

   * - 16. External module dependencies use %try-module — except modules delivered with the project itself (Qore ex: DataProvider, ConnectionProvider, QUnit, etc.) which use hard %requires
     - N/A
     - The tree-sitter binary belongs to this repository; QUnit and FsUtil are core source modules.

   * - 17. No filesystem operations (fopen, open, creat, unlink, remove, rename, mkdir, rmdir, stat, chmod) without sandbox checks
     - N/A
     - The ownership changes perform no filesystem operations.

   * - 18. No network operations (connect, bind, socket, getaddrinfo, gethostbyname) without sandbox checks
     - N/A
     - The ownership changes perform no network operations.

   * - 19. If filesystem/network ops exist, verify QoreSandboxManagerHelper usage
     - N/A
     - No new native filesystem or network operations.

   * - 20. No File::, Dir::, Socket::, HTTPClient:: usage without justification
     - N/A
     - New Qore tests use no filesystem/network operations.

   * - 21. All for/while loops that could iterate >100 times have qore_check_cancel() checks
     - Pass
     - Child enumeration and query/capture loops check cancellation every 100 iterations.

   * - 22. Uses qore_check_cancel() (NOT deprecated qore_check_io_interrupt())
     - Pass
     - Uses qore_check_cancel; no deprecated interrupt API added.

   * - 23. Check frequency: every 100 iterations for tight loops, every 10 for expensive iterations
     - Pass
     - Checks at iteration zero and each 100th bounded iteration.

   * - 24. No blocking operations without cancellation support
     - Pass
     - Native tree operations introduce no blocking I/O. Cursor state is protected by a scoped mutex.

   * - 25. Every action has display_name, short_desc (plain text, <80 chars), desc (markdown)
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 26. Every action has options populated via getActionOptionFromFields() — without this, the action shows an empty, unusable form
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 27. Every action has output_type set to a typed data type constant (e.g., MyResponseDataType) — not omitted
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 28. DPAT_API actions: provider has "supports_request": True and implements doRequestImpl()
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 29. DPAT_FIND actions: every option exists in SearchOptions, getRecordTypeImpl() returns *hash<string, AbstractDataField>
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 30. Scheme-based apps (with "scheme" in registerApp): actions use "path" and do NOT use "cls" — having both scheme and cls causes a runtime error
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 31. Single-key hash slices use trailing comma: Fields{"key",} (without trailing comma, Fields{"key"} returns the value, not a hash)
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 32. Typed data type classes exist for request and response types — inherit HashDataType, have const Fields hash, call addQoreFields(Fields) in constructor, export public constant at bottom (e.g., public const MyDataType = new MyDataType();)
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 33. Request/input types use public Fields (enables ClassName::Fields in action registration)
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 34. Response/output types use private Fields
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 35. Each field in data types has display_name, type, and desc (markdown-formatted)
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 36. Input fields have example_value where useful (string fields, endpoint URIs, SQL queries, etc.)
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 37. Fields with finite allowed values use allowed_values with AllowedValueInfo containing both value and display_name (Title Case, human-readable) — never bare values, never described only in text
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 38. Password/secret fields have "sensitive": True
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 39. groups uses AppGroup enum values from qlib/DataProvider/AppGroup.qc
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 40. App logo stored as separate file, loaded at module level in Priv namespace
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 41. App desc uses markdown: bullet list of capabilities, links to project website, business-language explanation of value
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 42. display_name is user-friendly ("Apache Avro" not "avro")
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 43. short_desc is plain text, under 80 chars, single sentence — no markdown
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 44. desc uses markdown: backticks for code/field refs ( field_name ,  True ,  pdf ), \n\n for paragraphs, -  bullet lists for enumerations, bold for caveats
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 45. Descriptions use plain business language relating to common challenges — not just technical "what" but "why" and "when to use"
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 46. No bare True/False/NOTHING — must be backtick-wrapped in desc
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 47. No bare field/option names in prose — must use backticks
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 48. Long descriptions (>500 chars) use bold section headers and bullet lists
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 49. Factory registration in Qore repo: every factory name registered in qlib/DataProvider/DataProvider.qc → FactoryMap (without this, module loads but doesn't appear in Qorus apps)
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 50. getRecordTypeImpl() signature: must be private *hash<string, AbstractDataField> getRecordTypeImpl(*hash<auto> search_options) — NOT returning *AbstractDataProviderType
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 51. Dependency JARs committed (for JNI modules): JAR files in qlib/*/jar/ may be gitignored — use git add -f to ensure they're tracked, otherwise CI compilation fails
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 52. JAR install rules in CMakeLists.txt for all dependency JARs
     - N/A
     - No new module, QPP class, DataProvider registration, separated module structure or Java dependency.

   * - 53. No workarounds: No TODOs, FIXMEs, stubs, or partially-implemented features
     - Pass
     - Fixes borrowed-tree ownership directly. No timing changes, disabled cases, or retained global owners.

   * - 54. Exception safety: C++ uses ReferenceHolder for Qore allocations, std::unique_ptr for C++ allocations, *xsink checked after every fallible operation
     - Pass
     - shared_ptr balances the native tree ref even if control-block allocation fails. Child enumeration, Qore result containers and cursor copies use RAII; each fallible result insertion is checked.

   * - 55. Thread safety: All mutable shared state protected by std::lock_guard<std::mutex> or documented as immutable-after-construction
     - Pass
     - Node owners are immutable; traversal cursor state is guarded by its mutex. Query cursor ownership changes use its existing mutex. Concurrent cursor copy/reset tests pass.

   * - 56. Type safety: Strongly-typed code<return(args)> instead of untyped code; static_cast instead of C casts; typed hashdecls for results; enums where appropriate
     - Pass
     - Tree ownership uses a typed shared_ptr; no untyped ownership or new C casts. Tests use typed nodes, lists and capture hashes.

   * - 57. Performance: No O(n²) where O(n) is possible; no unnecessary copies; coordinate descent uses incremental residuals not full matrix multiply
     - Pass
     - Descendant nodes and cursors share the native tree and source rather than copying the full source per node. Enumeration remains linear.

   * - 58. Error handling: All inputs validated (dimensions, empty data, unfitted models); C++ I/O handles EAGAIN/EINTR if applicable
     - Pass
     - Result insertion and cancellation failures release partial outputs. Empty lists, 201 children, deleted/temporary parents and concurrent resets are covered.

   * - 59. Documentation: Doxygen @param, @return, @throw on all public methods; @par Example with realistic business scenarios; @note for important caveats
     - Pass
     - README, class documentation and release notes describe retained trees, complete copied-node source, and cursor reset behavior.

   * - 60. QPP flags: [flags=CONSTANT] on methods that never throw; [flags=RET_VALUE_ONLY] on methods that throw but have no side effects
     - Pass
     - Child enumeration uses RET_VALUE_ONLY; cursor operations retain their state-changing flags.

   * - 61. Security: No user-controlled format strings; no buffer overflows; bounds checking on array indices; no credentials in code
     - Pass
     - No credentials, paths, format strings or new unchecked offsets are introduced. Retained ownership prevents dangling TSNode tree pointers.

   * - 62. Correctness: Algorithms verified against reference implementations; edge cases tested (empty data, single sample, all-zero features)
     - Pass
     - 99 cases / 371 assertions pass normally and under Valgrind on Fedora 44, Leap 16 and EL10. Zero errors, lost allocations or suppressions; documentation builds also pass.
