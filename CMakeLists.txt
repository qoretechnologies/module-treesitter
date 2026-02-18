cmake_minimum_required(VERSION 3.14)

project(qore-treesitter-module VERSION 1.0.0)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

set(PROJECT_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12.0")
    cmake_policy(SET CMP0074 NEW)

    if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.18.0")
        cmake_policy(SET CMP0053 NEW)
    endif()
endif()

cmake_policy(SET CMP0042 NEW)

# Allow using FetchContent_Populate for source-only dependencies
# (avoids conflicting targets from grammar CMakeLists.txt files)
if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.30.0")
    cmake_policy(SET CMP0169 OLD)
endif()

if (UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

find_package(Qore 2.0 REQUIRED)

# Check for C++11.
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# Build with PIC for shared library use
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)

# Fetch tree-sitter core library (only need sources, not CMake targets)
FetchContent_Declare(
    tree-sitter
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter.git
    GIT_TAG v0.26.3
)
FetchContent_GetProperties(tree-sitter)
if(NOT tree-sitter_POPULATED)
    FetchContent_Populate(tree-sitter)
endif()

# Fetch language grammars (only need sources, not their CMake targets)
FetchContent_Declare(
    tree-sitter-python
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-python.git
    GIT_TAG v0.23.6
)
FetchContent_GetProperties(tree-sitter-python)
if(NOT tree-sitter-python_POPULATED)
    FetchContent_Populate(tree-sitter-python)
endif()

FetchContent_Declare(
    tree-sitter-java
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-java.git
    GIT_TAG v0.23.5
)
FetchContent_GetProperties(tree-sitter-java)
if(NOT tree-sitter-java_POPULATED)
    FetchContent_Populate(tree-sitter-java)
endif()

FetchContent_Declare(
    tree-sitter-json
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-json.git
    GIT_TAG v0.24.8
)
FetchContent_GetProperties(tree-sitter-json)
if(NOT tree-sitter-json_POPULATED)
    FetchContent_Populate(tree-sitter-json)
endif()

FetchContent_Declare(
    tree-sitter-yaml
    GIT_REPOSITORY https://github.com/tree-sitter-grammars/tree-sitter-yaml.git
    GIT_TAG v0.7.2
)
FetchContent_GetProperties(tree-sitter-yaml)
if(NOT tree-sitter-yaml_POPULATED)
    FetchContent_Populate(tree-sitter-yaml)
endif()

FetchContent_Declare(
    tree-sitter-javascript
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-javascript.git
    GIT_TAG v0.23.1
)
FetchContent_GetProperties(tree-sitter-javascript)
if(NOT tree-sitter-javascript_POPULATED)
    FetchContent_Populate(tree-sitter-javascript)
endif()

FetchContent_Declare(
    tree-sitter-kotlin
    GIT_REPOSITORY https://github.com/fwcd/tree-sitter-kotlin.git
    GIT_TAG 0.3.8
)
FetchContent_GetProperties(tree-sitter-kotlin)
if(NOT tree-sitter-kotlin_POPULATED)
    FetchContent_Populate(tree-sitter-kotlin)
endif()

FetchContent_Declare(
    tree-sitter-typescript
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-typescript.git
    GIT_TAG v0.23.2
)
FetchContent_GetProperties(tree-sitter-typescript)
if(NOT tree-sitter-typescript_POPULATED)
    FetchContent_Populate(tree-sitter-typescript)
endif()

# Find the Qore grammar installed by astparser
find_path(TREE_SITTER_QORE_SRC_DIR
    NAMES parser.c
    PATHS ${CMAKE_INSTALL_PREFIX}/share/qore/treesitter/tree-sitter-qore/src
          /usr/share/qore/treesitter/tree-sitter-qore/src
          /usr/local/share/qore/treesitter/tree-sitter-qore/src
    NO_DEFAULT_PATH
)
if(NOT TREE_SITTER_QORE_SRC_DIR)
    message(FATAL_ERROR "Qore tree-sitter grammar not found. Install the astparser module first.")
endif()
message(STATUS "Found Qore tree-sitter grammar: ${TREE_SITTER_QORE_SRC_DIR}")

# Build tree-sitter as a static library
add_library(tree-sitter-lib STATIC
    ${tree-sitter_SOURCE_DIR}/lib/src/lib.c
)
target_include_directories(tree-sitter-lib PUBLIC
    ${tree-sitter_SOURCE_DIR}/lib/include
    ${tree-sitter_SOURCE_DIR}/lib/src
)

# Build grammar parsers
add_library(tree-sitter-grammars STATIC
    ${tree-sitter-python_SOURCE_DIR}/src/parser.c
    ${tree-sitter-python_SOURCE_DIR}/src/scanner.c
    ${tree-sitter-java_SOURCE_DIR}/src/parser.c
    ${tree-sitter-json_SOURCE_DIR}/src/parser.c
    ${tree-sitter-yaml_SOURCE_DIR}/src/parser.c
    ${tree-sitter-yaml_SOURCE_DIR}/src/scanner.c
    ${tree-sitter-javascript_SOURCE_DIR}/src/parser.c
    ${tree-sitter-javascript_SOURCE_DIR}/src/scanner.c
    ${tree-sitter-kotlin_SOURCE_DIR}/src/parser.c
    ${tree-sitter-kotlin_SOURCE_DIR}/src/scanner.c
    ${tree-sitter-typescript_SOURCE_DIR}/typescript/src/parser.c
    ${tree-sitter-typescript_SOURCE_DIR}/typescript/src/scanner.c
    ${tree-sitter-typescript_SOURCE_DIR}/tsx/src/parser.c
    ${tree-sitter-typescript_SOURCE_DIR}/tsx/src/scanner.c
    ${TREE_SITTER_QORE_SRC_DIR}/parser.c
)
target_include_directories(tree-sitter-grammars PUBLIC
    ${tree-sitter_SOURCE_DIR}/lib/include
    ${tree-sitter-python_SOURCE_DIR}/src
    ${tree-sitter-java_SOURCE_DIR}/src
    ${tree-sitter-json_SOURCE_DIR}/src
    ${tree-sitter-yaml_SOURCE_DIR}/src
    ${tree-sitter-javascript_SOURCE_DIR}/src
    ${tree-sitter-kotlin_SOURCE_DIR}/src
    ${tree-sitter-typescript_SOURCE_DIR}/typescript/src
    ${tree-sitter-typescript_SOURCE_DIR}/tsx/src
    ${TREE_SITTER_QORE_SRC_DIR}
)

set(QPP_SRC
    src/QC_TreeSitterParser.qpp
    src/QC_TreeSitterTree.qpp
    src/QC_TreeSitterNode.qpp
    src/QC_TreeSitterCursor.qpp
    src/QC_TreeSitterQuery.qpp
)

set(CPP_SRC
    src/treesitter-module.cpp
    src/TreeSitterParser.cpp
    src/TreeSitterTree.cpp
    src/TreeSitterNode.cpp
    src/TreeSitterCursor.cpp
    src/TreeSitterQuery.cpp
    src/TreeSitterLanguages.cpp
)

qore_wrap_qpp_value(QPP_SOURCES METALIST QPP_META_SRC ${QPP_SRC})

foreach (it ${QPP_SOURCES})
    GET_FILENAME_COMPONENT(_outfile ${it} NAME_WE)
    set(QPP_DOX ${QPP_DOX} ${CMAKE_CURRENT_BINARY_DIR}/${_outfile}.dox.h)
endforeach()

set(module_name "treesitter")

set(QORE_DOX_TMPL_SRC
  docs/mainpage.dox.tmpl
)

add_library(${module_name} MODULE ${CPP_SRC} ${QPP_SOURCES})

include_directories(${CMAKE_SOURCE_DIR}/src)
target_include_directories(${module_name} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    ${tree-sitter_SOURCE_DIR}/lib/include
)

add_custom_target(QORE_INC_FILES DEPENDS ${QORE_INC_SRC})
add_dependencies(${module_name} QORE_INC_FILES)

target_link_libraries(${module_name} tree-sitter-lib tree-sitter-grammars ${QORE_LIBRARY})

set(MODULE_DOX_INPUT ${CMAKE_CURRENT_BINARY_DIR}/mainpage.dox ${QPP_DOX})
string(REPLACE ";" " " MODULE_DOX_INPUT "${MODULE_DOX_INPUT}")

if (DEFINED ENV{DOXYGEN_EXECUTABLE})
    set(DOXYGEN_EXECUTABLE $ENV{DOXYGEN_EXECUTABLE})
endif()

set(DOXYGEN_IMAGE_PATH ${CMAKE_SOURCE_DIR}/docs/images CACHE STRING "Doxygen image path" FORCE)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/images)

set(MODULE_DOX_INPUT ${CMAKE_BINARY_DIR})
qore_external_binary_module(${module_name} ${PROJECT_VERSION})

qore_dist(${PROJECT_VERSION})

# install qpp metadata files
if (COMMAND qore_install_qpp_metadata)
    qore_install_qpp_metadata(${module_name} ${QPP_META_SRC})
else()
    install(FILES ${QPP_META_SRC}
            DESTINATION ${CMAKE_INSTALL_DATADIR}/qore/metadata)
endif()

qore_config_info()

if (DOXYGEN_FOUND)
    qore_wrap_dox(QORE_DOX_SRC ${QORE_DOX_TMPL_SRC})
    add_custom_target(QORE_MOD_DOX_FILES DEPENDS ${QORE_DOX_SRC})
    add_dependencies(docs-module QORE_MOD_DOX_FILES)
endif()
