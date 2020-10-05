cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(incbin LANGUAGES C)

include(GNUInstallDirs)
include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS NO_OUTPUT_DIRS)

add_library(incbin INTERFACE)
add_library(incbin::incbin ALIAS incbin)
target_include_directories(incbin INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

option(INCBIN_HEADER_ONLY "Do not build the incbin executable." ON)
if(NOT INCBIN_HEADER_ONLY)
    add_executable(incbin-gen incbin.c)
    set_target_properties(incbin-gen PROPERTIES
        OUTPUT_NAME "incbin"
    )

    install(TARGETS incbin-gen
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

# Installation
install(FILES incbin.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(FILES README.md UNLICENSE
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)
install(TARGETS incbin EXPORT incbin-config)
install(EXPORT incbin-config
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/incbin
)
export(TARGETS incbin
    NAMESPACE incbin::
    FILE incbin-config.cmake
)
