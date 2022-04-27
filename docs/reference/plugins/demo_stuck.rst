.. reference-gen: demo_stuck

Demo-Stuck Controller
=====================

The :program:`demo_stuck` controller plugin is a controller that can be
configured to run very slowly and get stuck at certain point in the simulation.
This is primarily used for system testing.

Examples
--------

You can see the plugin in action in the system test at
``tests/test_engine_stuck_controller.json``, among other tests.

.. literalinclude:: ../../../tests/test_engine_stuck_controller.json
   :language: json

Defaults
--------

The following help can be viewed with :command:`cloe-engine usage demo_stuck`:

.. literalinclude:: demo_stuck.yaml
   :language: yaml

JSON Schema
-----------

The following can be viewed with :command:`cloe-engine usage --json demo_stuck`:

.. literalinclude:: demo_stuck_schema.json
   :language: json
