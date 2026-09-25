# CMakeLists.txt -- top-level build for mapper (KAN-11).
#
# Toolchain: MSVC (VS2022, cl.exe) + Ninja, driven by CMakePresets.json.
# The Qt 6 prefix is pinned in CMakePresets.json (CMAKE_PREFIX_PATH), not here,
# so a developer or CI agent can override it without editing this file.
# The prefix MUST be an MSVC Qt kit (msvc2022_64). A mingw_64 kit exports
# GCC import libraries that link.exe cannot consume.

cmake_minimum_required(VERSION 3.21)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(mapper
    VERSION 0.1.0
    DESCRIPTION "Desktop viewer for ESRI Shapefiles"
    LANGUAGES CXX
)

# Project-wide language level. Required explicitly: mapper_model and mapper_io
# link no Qt target, so they inherit no cxx_std_17 INTERFACE requirement and
# would otherwise compile at the MSVC default (C++14), rejecting nested
# namespace definitions, [[nodiscard]] and <optional>.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MAPPER_BUILD_TESTS "Build the Catch2 unit tests" ON)

set(Qt6_DIR "C:/Qt/6.11.2/msvc2022_64/lib/cmake/Qt6")
find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

# qt_standard_project_setup() turns CMAKE_AUTOMOC on for every target in the
# tree, including mapper_model and mapper_io, which SPECIFICATION.md section 5
# forbids from depending on Qt. Confine AUTOMOC to the Qt targets, which enable
# it per target.
set(CMAKE_AUTOMOC OFF)

# Shared warning configuration. Every target links this; it carries flags only,
# no sources and no link dependencies.
add_library(mapper_warnings INTERFACE)
if (MSVC)
    target_compile_options(mapper_warnings INTERFACE /W4 /permissive- /EHsc)
else()
    target_compile_options(mapper_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()

# Dependency rule (SPECIFICATION.md section 5): arrows point downward only.
# model/ and io/ are plain C++ and MUST NOT link Qt. Only view/ and app/ may.
add_subdirectory(model)
add_subdirectory(io)
add_subdirectory(view)
add_subdirectory(app)

if(MAPPER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
