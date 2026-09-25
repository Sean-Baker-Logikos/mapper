# mapper -- application shell. Entry point is main.cpp; the root-level
# mapper.cpp/mapper.h VS template files were retired (see SPECIFICATION.md
# section 5).
qt_add_executable(mapper
    main.cpp
    MainWindow.cpp
    MainWindow.h
)

target_include_directories(mapper PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_definitions(mapper PRIVATE MAPPER_VERSION="${PROJECT_VERSION}")
target_link_libraries(mapper
    PRIVATE mapper_view mapper_io mapper_model mapper_warnings Qt6::Widgets
)

# MainWindow.h declares Q_OBJECT; AUTOMOC is off globally (root CMakeLists.txt).
set_target_properties(mapper PROPERTIES
    AUTOMOC ON
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
