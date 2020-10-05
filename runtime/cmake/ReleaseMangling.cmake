# This module helps transform a package release of 0 to an appropriate nightly
# value.
#

find_program(git_path NAMES git)

function(_git_repo_hash branch hash)
    execute_process(
        COMMAND ${git_path} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE git_branch
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(${branch} ${git_branch} PARENT_SCOPE)
    execute_process(
        COMMAND ${git_path} log -1 --format=%h
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE git_hash
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(${hash} ${git_hash} PARENT_SCOPE)
endfunction()

function(_get_timestamp output_var)
    execute_process(
        COMMAND date +%Y%m%d
        OUTPUT_VARIABLE timestamp
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(${output_var} ${timestamp} PARENT_SCOPE)
endfunction()

function(mangle_package_release release default)
    if(${release})
        # Do nothing
    elseif(${default})
        set(${release} ${${default}} PARENT_SCOPE)
    elseif(git_path AND EXISTS ${PROJECT_SOURCE_DIR}/.git)
        _git_repo_hash(git_branch git_hash)
        _get_timestamp(timestamp)
        set(${release} "0beta.${timestamp}.${git_hash}" PARENT_SCOPE)
    elseif(DEFINED ENV{GIT_COMMIT_HASH})
        _get_timestamp(timestamp)
        set(${release} "0beta.${timestamp}.$ENV{GIT_COMMIT_HASH}" PARENT_SCOPE)
    else()
        _get_timestamp(timestamp)
        set(${release} "0beta.${timestamp}" PARENT_SCOPE)
    endif()
endfunction()
