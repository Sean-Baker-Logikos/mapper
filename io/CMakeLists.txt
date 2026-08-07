# mapper_io -- shapefile parsing. Plain C++, headless-testable. MUST NOT link Qt.
add_library(mapper_io STATIC
    Endian.cpp
    Endian.h
)

target_include_directories(mapper_io PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(mapper_io
    PUBLIC  mapper_model
    PRIVATE mapper_warnings
)
