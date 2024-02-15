set(CLOE_FIND_PACKAGES ON CACHE BOOL "Call find_package() for cloe packages")
if(CLOE_FIND_PACKAGES)
    find_package(fable REQUIRED)
    find_package(cloe-runtime REQUIRED)
    find_package(cloe-models REQUIRED)
    find_package(cloe-stacklib REQUIRED)
endif()
find_package(Boost REQUIRED)
find_package(CLI11 REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
# Library libstack ---------------------------------------------------
add_library(cloe-stacklib
        src/stack.cpp
        src/stack_factory.cpp
        src/plugin.cpp

        # Built-in plugins:
        src/plugins/nop_controller.cpp
        src/plugins/nop_simulator.cpp
)
add_library(cloe::stacklib ALIAS cloe-stacklib)
set_target_properties(cloe-stacklib PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        OUTPUT_NAME stack
)
target_include_directories(cloe-stacklib
        PUBLIC
        include
)
target_link_libraries(cloe-stacklib
        PUBLIC
        cloe::runtime
        cloe::models
        fable::fable
        Boost::headers
        Threads::Threads
        ${CMAKE_DL_LIBS}
)

include(CTest)
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    include(GoogleTest)

    add_executable(test-stacklib
            test/stack_test.cpp
            test/stack_component_test.cpp
    )
    set_target_properties(test-stacklib PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-stacklib
            GTest::gtest
            GTest::gtest_main
            Boost::boost
            cloe::models
            cloe::stacklib
    )
    gtest_add_tests(TARGET test-stacklib)
endif()
