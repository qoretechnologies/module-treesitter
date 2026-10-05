# Copyright (C) 2026 Qore Technologies, s.r.o.
# SPDX-License-Identifier: MIT
# Use the pinned source epoch for RPM headers and installed file timestamps.
%global source_date_epoch_from_changelog 1
%global use_source_date_epoch_as_buildtime 1
%if v"%{rpmversion}" >= v"4.20"
%global build_mtime_policy clamp_to_source_date_epoch
%else
%global clamp_mtime_to_source_date_epoch 1
%endif
%bcond_without tests
%bcond_without docs
Name: qore-treesitter-module
Version: 1.0.0
Release: 2%{?dist}
Summary: Incremental syntax parsing and queries for Qore
License: LGPL-2.1-or-later AND LGPL-2.1-only AND MIT
URL: https://github.com/qoretechnologies/module-treesitter
Source0: https://api.opensuse.org/public/source/home:davidnichols:qore:testing/%{name}/%{name}-%{version}.tar.xz
Source1: https://api.opensuse.org/public/source/home:davidnichols:qore:testing/%{name}/tree-sitter-python-0.25.0.tar.xz
Provides: bundled(tree-sitter-python) = 0.25.0
Source2: https://api.opensuse.org/public/source/home:davidnichols:qore:testing/%{name}/tree-sitter-java-0.23.5.tar.xz
Provides: bundled(tree-sitter-java) = 0.23.5
Source3: https://api.opensuse.org/public/source/home:davidnichols:qore:testing/%{name}/tree-sitter-json-0.24.8.tar.xz
Provides: bundled(tree-sitter-json) = 0.24.8
Source4: https://api.opensuse.org/public/source/home:davidnichols:qore:testing/%{name}/tree-sitter-yaml-0.7.2.tar.xz
Provides: bundled(tree-sitter-yaml) = 0.7.2
Source5: https://api.opensuse.org/public/source/home:davidnichols:qore:testing/%{name}/tree-sitter-javascript-0.25.0.tar.xz
Provides: bundled(tree-sitter-javascript) = 0.25.0
Source6: https://api.opensuse.org/public/source/home:davidnichols:qore:testing/%{name}/tree-sitter-kotlin-0.3.8.tar.xz
Provides: bundled(tree-sitter-kotlin) = 0.3.8
Source7: https://api.opensuse.org/public/source/home:davidnichols:qore:testing/%{name}/tree-sitter-typescript-0.23.2.tar.xz
Provides: bundled(tree-sitter-typescript) = 0.23.2
Source8: https://api.opensuse.org/public/source/home:davidnichols:qore:testing/%{name}/tree-sitter-sql-0.3.11.tar.xz
Provides: bundled(tree-sitter-sql) = 0.3.11
Source9: https://api.opensuse.org/public/source/home:davidnichols:qore:testing/%{name}/tree-sitter-markdown-0.5.3.tar.xz
Provides: bundled(tree-sitter-markdown) = 0.5.3
BuildRequires: cmake >= 3.21
BuildRequires: make
BuildRequires: gcc-c++
BuildRequires: pkgconfig(tree-sitter) >= 0.26.5
BuildRequires: qore-devel >= 3.0.0~
BuildRequires: qore-rpm-macros >= 3.0.0~
%if %{with tests}
BuildRequires: python3
%endif
%if %{with docs}
BuildRequires: doxygen
%endif
%if 0%{?suse_version}
BuildRequires: util-linux
%else
BuildRequires: util-linux-core
%endif

%description
Incremental parsing, syntax trees, cursors and structural queries for
Qore, Python, Java, JSON, YAML, JavaScript, Kotlin, TypeScript, TSX, SQL and
Markdown. Includes installed query resources and compiler metadata. Uses the
system tree-sitter runtime and pinned private grammars with hidden symbols.

%if %{with docs}
%package doc
Summary: Tree-sitter module API reference
BuildArch: noarch
%description doc
API reference and examples for syntax parsing and querying with Qore.
%endif

%prep
%autosetup
%setup -T -D -a 1
%setup -T -D -a 2
%setup -T -D -a 3
%setup -T -D -a 4
%setup -T -D -a 5
%setup -T -D -a 6
%setup -T -D -a 7
%setup -T -D -a 8
%setup -T -D -a 9
mkdir -p grammar-licenses
install -Dm644 tree-sitter-python-0.25.0/LICENSE grammar-licenses/tree-sitter-python/LICENSE
install -Dm644 tree-sitter-java-0.23.5/LICENSE grammar-licenses/tree-sitter-java/LICENSE
install -Dm644 tree-sitter-json-0.24.8/LICENSE grammar-licenses/tree-sitter-json/LICENSE
install -Dm644 tree-sitter-yaml-0.7.2/LICENSE grammar-licenses/tree-sitter-yaml/LICENSE
install -Dm644 tree-sitter-javascript-0.25.0/LICENSE grammar-licenses/tree-sitter-javascript/LICENSE
install -Dm644 tree-sitter-kotlin-0.3.8/LICENSE grammar-licenses/tree-sitter-kotlin/LICENSE
install -Dm644 tree-sitter-typescript-0.23.2/LICENSE grammar-licenses/tree-sitter-typescript/LICENSE
install -Dm644 tree-sitter-sql-0.3.11/LICENSE grammar-licenses/tree-sitter-sql/LICENSE
install -Dm644 tree-sitter-markdown-0.5.3/LICENSE grammar-licenses/tree-sitter-markdown/LICENSE
# The SDK supplies the Qore grammar: retain its source RPM identity and hashes
# alongside the module so its corresponding source can be located precisely.
rpm -q --qf '%%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}\n%%{SOURCERPM}\n' qore-devel > qore-grammar-build-source.txt
sha256sum %{_datadir}/qore/treesitter/tree-sitter-qore/grammar.js \
  %{_datadir}/qore/treesitter/tree-sitter-qore/src/parser.c \
  %{_datadir}/qore/treesitter/tree-sitter-qore/src/scanner.c >> qore-grammar-build-source.txt

%build
%{?set_build_flags}
. %{_rpmconfigdir}/qore/module-env.sh
qore_set_source_prefix_maps "%{qore_debug_source_dir}"
cmake -S . -B build -G 'Unix Makefiles' \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS_RELEASE=-DNDEBUG -DCMAKE_CXX_FLAGS_RELEASE=-DNDEBUG \
  -DCMAKE_INSTALL_PREFIX=%{_prefix} \
  -DCMAKE_SKIP_RPATH=ON -DCMAKE_IGNORE_PREFIX_PATH=/usr/local \
  -DQore_DIR=%{_libdir}/cmake/Qore -DQORE_EXECUTABLE=/usr/bin/qore \
  -DQORE_QPP_EXECUTABLE=/usr/bin/qpp -DQORE_GENERATE_JAVA_BINDINGS=OFF \
  -DQORE_USE_SYSTEM_TREE_SITTER=ON -DFETCHCONTENT_FULLY_DISCONNECTED=ON \
  -DTREE_SITTER_QORE_SRC_DIR=%{_datadir}/qore/treesitter/tree-sitter-qore/src \
  -DFETCHCONTENT_SOURCE_DIR_TREE-SITTER-PYTHON:PATH=$PWD/tree-sitter-python-0.25.0 \
  -DFETCHCONTENT_SOURCE_DIR_TREE-SITTER-JAVA:PATH=$PWD/tree-sitter-java-0.23.5 \
  -DFETCHCONTENT_SOURCE_DIR_TREE-SITTER-JSON:PATH=$PWD/tree-sitter-json-0.24.8 \
  -DFETCHCONTENT_SOURCE_DIR_TREE-SITTER-YAML:PATH=$PWD/tree-sitter-yaml-0.7.2 \
  -DFETCHCONTENT_SOURCE_DIR_TREE-SITTER-JAVASCRIPT:PATH=$PWD/tree-sitter-javascript-0.25.0 \
  -DFETCHCONTENT_SOURCE_DIR_TREE-SITTER-KOTLIN:PATH=$PWD/tree-sitter-kotlin-0.3.8 \
  -DFETCHCONTENT_SOURCE_DIR_TREE-SITTER-TYPESCRIPT:PATH=$PWD/tree-sitter-typescript-0.23.2 \
  -DFETCHCONTENT_SOURCE_DIR_TREE-SITTER-SQL:PATH=$PWD/tree-sitter-sql-0.3.11 \
  -DFETCHCONTENT_SOURCE_DIR_TREE-SITTER-MARKDOWN:PATH=$PWD/tree-sitter-markdown-0.5.3 \
  -DCMAKE_DISABLE_FIND_PACKAGE_Doxygen=%{!?with_docs:ON}%{?with_docs:OFF}
grep -qx 'QORE_TREE_SITTER_FOUND:INTERNAL=1' build/CMakeCache.txt
%if %{with docs}
printf '\nWARN_AS_ERROR = FAIL_ON_WARNINGS\n' >> build/Doxyfile
%endif
cmake --build build -- %{?_smp_mflags}
%if %{with docs}
cmake --build build --target docs -- %{?_smp_mflags}
%endif

%install
DESTDIR=%{buildroot} cmake --install build
chmod 755 %{buildroot}%{_libdir}/qore-modules/treesitter-api-*.qmod
install -d %{buildroot}%{_licensedir}/%{name}
cp -a COPYING.LGPL COPYING.GPL grammar-licenses %{buildroot}%{_licensedir}/%{name}/
hardlink -t -O %{buildroot}%{_licensedir}/%{name}
%if %{with docs}
install -d %{buildroot}%{_docdir}/%{name}-doc
cp -a build/docs/treesitter/html %{buildroot}%{_docdir}/%{name}-doc/
hardlink -t -O %{buildroot}%{_docdir}/%{name}-doc
%endif

%check
%if %{with tests}
diff -r grammar-licenses %{buildroot}%{_licensedir}/%{name}/grammar-licenses
cmp COPYING.LGPL %{buildroot}%{_licensedir}/%{name}/COPYING.LGPL
cmp COPYING.GPL %{buildroot}%{_licensedir}/%{name}/COPYING.GPL
test %{buildroot}%{_licensedir}/%{name}/grammar-licenses/tree-sitter-json/LICENSE -ef \
  %{buildroot}%{_licensedir}/%{name}/grammar-licenses/tree-sitter-javascript/LICENSE
python3 -B -W error test/test_grammar_fixes.py tree-sitter-python-0.25.0 tree-sitter-yaml-0.7.2 -v
. %{_rpmconfigdir}/qore/module-env.sh
QORE_TREESITTER_QUERY_DIR=$PWD/build/queries /usr/bin/qore -b --enable-debug \
  -l "$PWD/build/treesitter-api-$(/usr/bin/qore --latest-module-api).qmod" test/treesitter.qtest -v
%endif

%files
%license %{_licensedir}/%{name}/
%doc README.md rpm/README.rst qore-grammar-build-source.txt
%{_libdir}/qore-modules/treesitter-api-*.qmod
%dir %{_datadir}/qore/metadata/treesitter
%{_datadir}/qore/metadata/treesitter/*.meta.json
%{_datadir}/qore/treesitter/queries/
%if %{with docs}
%files doc
%license COPYING.LGPL COPYING.GPL
%doc %{_docdir}/%{name}-doc/
%endif

%changelog
* Mon Oct 05 2026 David Nichols <david@qore.org> - 1.0.0-2
- Preserve every grammar license path and byte while hard-linking identical notices.
- Point repacked sources to their public OBS downloads.

* Fri Oct 02 2026 David Nichols <david@qore.org> - 1.0.0-1
- Package native parsers, compiler metadata and query resources for all languages.
- Retain pinned grammar licenses and Qore SDK grammar source provenance.
- Require offline tests, distribution dependencies and strict API references.
