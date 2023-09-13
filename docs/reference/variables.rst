Variable Interpolation
======================

The Cloe engine can interpolate variables of the form::

    ${VARIABLE_NAME}
    ${VARIABLE_NAME-default string}

The default string can also contain a variable.
For example, the registry path is set like so by default::

    ${XDG_DATA_HOME-${HOME}/.local/share}/cloe/registry

There are several special variables that are set by Cloe engine:

``THIS_STACKFILE_DIR``
    Full path to directory containing stack file containing this variable.

``THIS_STACKFILE_FILE``
    Full path to stack file containing this variable.

``CLOE_SIMULATION_UUID``
    If this variable is not

Example stack file::

    {
        "version": "4",
        "engine": {
            "output": {
                 "path": "${THIS_STACKFILE_DIR}/output"
            }
        }
    }

Interpolation occurs before the JSON is parsed, so variables
can be placed anywhere::

    {
        "version": "4",
        ${ENGINE_CONFIG}
    }

However, care should be taken to ensure that the stack file is valid
JSON after interpolation. In the above example, if ``ENGINE_CONFIG`` is
empty, then the JSON is invalid, because of the trailing comma.

This approach does allow us to use variables in places where strings
are not useable, such as for integers or booleans.

Interpolation can be disabled or enabled from the command line,
and the handling of non-existing variables can also be specified:

``--interpolate`` and ``--no-interpolate``
    Interpolate variables of the form ``${XYZ}`` in stack files.
    This is enabled by default.

``--interpolate-undefined``
    Interpolate undefined variables with empty strings.
    This is disabled by default.

See ``cloe-engine --help`` for up-to-date information for your version of
Cloe.
