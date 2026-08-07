# mapper_model -- plain C++ geometry types. MUST NOT link Qt.
add_library(mapper_model STATIC
    Envelope.cpp
    Envelope.h
    Geometry.h
    Layer.h
)

target_include_directories(mapper_model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(mapper_model PRIVATE mapper_warnings)
