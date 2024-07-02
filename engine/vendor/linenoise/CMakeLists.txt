cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(linenoise LANGUAGES C)

include(GNUInstallDirs)

add_library(linenoise
    linenoise.c
    linenoise.h
)
add_library(linenoise::linenoise ALIAS linenoise)
target_include_directories(linenoise
    PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
install(TARGETS linenoise
    EXPORT linenoiseTargets
    LIBRARY
    ARCHIVE
    RUNTIME
)
install(FILES linenoise.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

include(CTest)
if(BUILD_TESTING)
    add_executable(linenoise-example
        example.c
    )
    target_link_libraries(linenoise-example
        PRIVATE
        linenoise
    )
endif()
