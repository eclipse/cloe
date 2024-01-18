find_package(pybind11 CONFIG REQUIRED)
pybind11_add_module(pycloe bindings.cpp)
target_link_libraries(pycloe PUBLIC cloe::stacklib cloe::enginelib)
# todo this should be exposed by engine or so
target_include_directories(pycloe PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../../engine/src)
