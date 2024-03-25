cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

set(CLOE_FIND_PACKAGES ON CACHE BOOL "Call find_package() for cloe packages")
set(CLOE_ENGINE_VERSION ${CLOE_PROJECT_VERSION})
if(CLOE_FIND_PACKAGES)
    find_package(fable REQUIRED)
    find_package(cloe-runtime REQUIRED)
    find_package(cloe-models REQUIRED)
    find_package(cloe-stacklib REQUIRED)
endif()
find_package(Boost REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
# Library libstack ---------------------------------------------------
add_library(cloe-simulation
        src/simulation.cpp
        src/simulation_context.cpp
        src/registrar.cpp
        src/utility/command.cpp
)
add_library(cloe::simulation ALIAS cloe-simulation)
set_target_properties(cloe-simulation PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)
target_compile_definitions(cloe-simulation
        PUBLIC
        CLOE_ENGINE_VERSION="${CLOE_ENGINE_VERSION}"
        PROJECT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)
target_include_directories(cloe-simulation
        PUBLIC
        include
)
target_link_libraries(cloe-simulation
        PUBLIC
        cloe::stacklib
        cloe::models
        cloe::runtime
        fable::fable
        boost::boost
        Threads::Threads
)


option(CLOE_ENGINE_WITH_SERVER "Enable integrated server component?" ON)
if(CLOE_ENGINE_WITH_SERVER)
    message(STATUS "-> Enable server component")
    if(CLOE_FIND_PACKAGES)
        find_package(cloe-oak REQUIRED)
    endif()
    target_sources(cloe-simulation PRIVATE src/server.cpp)
    target_link_libraries(cloe-simulation PRIVATE cloe::oak)
    target_compile_definitions(cloe-simulation PUBLIC CLOE_ENGINE_WITH_SERVER=1)
else()
    message(STATUS "-> Disable server component")
    target_sources(cloe-simulation PRIVATE src/server_mock.cpp)
    target_compile_definitions(cloe-simulation PUBLIC CLOE_ENGINE_WITH_SERVER=0)
endif()

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS cloe-simulation DESTINATION ${CMAKE_INSTALL_LIBDIR})
