# mapper_view -- Qt rendering layer.
add_library(mapper_view STATIC
    MapCanvas.cpp
    MapCanvas.h
)

target_include_directories(mapper_view PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(mapper_view
    PUBLIC  mapper_model Qt6::Widgets
    PRIVATE mapper_warnings
)

set_target_properties(mapper_view PROPERTIES AUTOMOC ON)
