RPM packaging
=============

Copyright 2026 Qore Technologies, s.r.o.

The portable recipe packages the native module, eleven language parsers,
compiler metadata and installed upstream query files. HTML documentation is a
separate package. The distribution tree-sitter runtime is required; nine grammar
components use the same pinned sources and retained paths as Debian packaging.
No network access, npm or grammar generation occurs during the RPM build.

Prepare a pinned source bundle from qore-packaging::

    python3 tools/packaging.py prepare --repo ../module-treesitter --ref COMMIT \
      --name qore-treesitter-module --version 1.0.0 \
      --spec qore-treesitter-module.spec --vendor-manifest rpm/vendor-sources.json \
      --cache cache --output work/treesitter-source
    python3 tools/build-local.py --source work/treesitter-source \
      --image TARGET_SDK_IMAGE --output results/treesitter-build --jobs 4

Private grammar symbols remain hidden so they cannot interpose Qore's own
astparser. The Qore grammar comes from the installed qore-devel SDK; its source
RPM identity and input hashes are recorded in qore-grammar-build-source.txt.
Retain that exact Qore source RPM together with this module's source RPM.
The Qore grammar is copyright 2026 Qore Technologies, s.r.o., licensed under
LGPL 2.1 (the scanner permits later versions); COPYING.LGPL and COPYING.GPL
provide the license texts. Each MIT grammar retains its original notice in
its own directory under grammar-licenses.

Repository qualification uses the default test/documentation options, then
runs the complete suite outside the checkout with QORE_TREESITTER_QUERY_DIR
unset against installed packages. It also compiles debian/tests/compiler's
named-argument parser/query probe with qcc, checks the installed query inventory
against the pinned sources, and verifies runtime installation without the SDK.

The default check phase also validates the hash-checked Python/YAML source fixes
and their unchanged lexer behavior. See README.md for the standalone command.

Installed runtime example (from the extracted source)::

    test_dir=$(mktemp -d /tmp/qore-treesitter-installed.XXXXXX)
    cp test/treesitter.qtest "$test_dir/"
    (
      unset QORE_TREESITTER_QUERY_DIR QORE_MODULE_DIR QORE_MODULE_DIR_ONLY LD_LIBRARY_PATH LD_PRELOAD
      export QORE_MODULE_DIR=$(/usr/bin/qore --module-path) QORE_MODULE_DIR_ONLY=1
      cd "$test_dir"
      /usr/bin/qore -b --enable-debug treesitter.qtest -v
    )

In an SDK image, run the installed compiler example with
``AUTOPKGTEST_TMP=$(mktemp -d) sh debian/tests/compiler``. Both examples were
qualified for Fedora 44, Leap 16.0 and EL10 against candidate 3. The runtime
images contained neither qore-devel nor gcc/gcc-c++; RPM verification passed.
Final canonical builds use the final qualified Qore SDK and remain a release gate.
