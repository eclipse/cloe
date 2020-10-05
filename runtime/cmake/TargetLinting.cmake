# This module causes compilation to proceed with more static checks and static
# analysis. There are two levels of checks that can be enabled, the normal
# and the extended. The extended level contains more checks, but lengthens
# the compilation process significantly and may also contain more false
# positives.
#
# In general, set_target_linting(your_target) can be used and provides good
# defaults. If TargetLintingExtended is set to a truthy value, then extended
# linting is provided.
#

option(TargetLintingExtended "Perform extended linting if possible" OFF)
option(TargetLintingCppCheck "Apply cppcheck during extended linting" OFF)
option(TargetLintingIwyu     "Apply iwyu during extended linting" OFF)

function(set_target_pedantic target)
    target_compile_options(${target}
      PRIVATE
        -Wall
        -Wextra
        -pedantic
    )
endfunction()

function(try_target_clang_tidy target)
    find_program(clang_tidy_path
        NAMES
            clang-tidy-7.0
            clang-tidy-6.0
            clang-tidy-5.0
            clang-tidy-4.0
            clang-tidy
    )
    if(clang_tidy_path)
        set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY ${clang_tidy_path})
    endif()
endfunction()

function(try_target_cppcheck target)
    find_program(cppcheck_path NAMES cppcheck)
    if(cppcheck_path)
        set_target_properties(${target} PROPERTIES CXX_CPPCHECK ${cppcheck_path})
    endif()
endfunction()

function(try_target_iwyu target)
    find_program(iwyu_path NAMES include-what-you-use iwyu)
    if(iwyu_path)
        set_target_properties(${target} PROPERTIES CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
    endif()
endfunction()

# The following three functions are quick ways to set different levels
# of linting on a target.

function(set_target_linting_safe target)
    try_target_clang_tidy(${target})
endfunction()

function(set_target_linting_extended target)
    target_compile_options(${target}
      PRIVATE
        -Werror
    )
    if(TargetLintingCppCheck)
        try_target_cppcheck(${target})
    endif()
    if(TargetLintingIwyu)
        try_target_iwyu(${target})
    endif()
endfunction()

function(set_target_linting target)
    set_target_pedantic(${target})
    set_target_linting_safe(${target})
    if(TargetLintingExtended)
        set_target_linting_extended(${target})
    endif()
endfunction()
