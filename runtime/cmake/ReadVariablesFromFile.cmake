# This module provides a function to read in variables from the given file and
# stores them in CMake variables of the same name.

function(read_variables_from_file filepath var_prefix)
    file(STRINGS "${filepath}" assignments)
    foreach(line ${assignments})
        string(STRIP ${line} line)
        if(${line} MATCHES "^([a-zA-Z0-9_]+) *= *\"?([^\"]*)\"?$")
            message(STATUS "-> Setting ${var_prefix}${CMAKE_MATCH_1} = \"${CMAKE_MATCH_2}\"")
            set(${var_prefix}${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" PARENT_SCOPE)
        endif()
    endforeach()
endfunction()
