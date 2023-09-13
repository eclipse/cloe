Hook Execution
==============

When running simulations, you may want to run programs before or after the
simulation execution itself. Triggers currently only offer events during
a simulation, since many actions are only available after simulation agents are
connected and the simulation has started.

The Cloe engine can execute hooks at two different times:

- pre-connect
- post-disconnect

These can be used in the ``engine`` section of the stack file:

- ``/engine/hooks/pre_connect`` as an array of command actions
- ``/engine/hooks/post_disconnect`` as an array of command actions

For a description of the ``command`` action, see :doc:`actions`.

Here is an example bootstrap stack file, which is included in the list of
input stack files when we want to automatically start and stop the VTD simulator::

  {
    "version": "4",
    "engine": {
      "hooks": {
        "pre_connect": [
          { "path": "${VTD_LAUNCH}", "args": [ "stop" ] },
          {
            "path": "${VTD_LAUNCH}",
            "args": [
              "-l", "/tmp/vtd.log",
              "start",
              "--vtd-setup-cloe", "Cloe.noGUInoIG",
              "--vtd-project", "${THIS_STACKFILE_DIR}/../contrib/projects/cloe_tests"
            ]
          }
        ],
        "post_disconnect": [
          { "path": "${VTD_LAUNCH}", "args": ["stop"] }
        ]
      }
    }
  }

Note that we can use environment variables as well as special variables
``THIS_STACKFILE_DIR`` and ``THIS_STACKFILE_FILE``, unless you have
interpolation disabled. See :doc:`variables` for more information on this.

Hooks can be disabled from the command line (or via an environment variable):

``--no-hooks``
    Disable execution of hooks.

``--secure``, ``-s``, ``CLOE_SECURE_MODE=1``
    Enable secure mode, which disables hooks, disables interpolation, and
    prevents loading files from the system.

See ``cloe-engine --help`` for up-to-date information for your version of
Cloe.
