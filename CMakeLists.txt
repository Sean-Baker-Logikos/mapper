cmake_minimum_required(VERSION 3.21)

project(mapper
    VERSION 0.1.0
    DESCRIPTION "Desktop viewer for ESRI Shapefiles"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MAPPER_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(MAPPER_BUILD_TESTS        "Build unit tests"                  ON)

# Warning configuration, applied to mapper's own targets only.
# Vendored third-party code (third_party/) is deliberately exempt -- see
# DEVELOPMENT.md section 7 on the 1996-era GCTP sources.
add_library(mapper_warnings INTERFACE)
if(MSVC)
    target_compile_options(mapper_warnings INTERFACE /W4 /permissive- /utf-8)
    if(MAPPER_WARNINGS_AS_ERRORS)
        target_compile_options(mapper_warnings INTERFACE /WX)
    endif()
else()
    target_compile_options(mapper_warnings INTERFACE -Wall -Wextra -Wpedantic)
    if(MAPPER_WARNINGS_AS_ERRORS)
        target_compile_options(mapper_warnings INTERFACE -Werror)
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

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
