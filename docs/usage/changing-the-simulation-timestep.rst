Changing the Simulation Timestep
================================

By default, the virtual time between each timestep is 20 milliseconds. This is
not hardcoded though, Cloe lets you set this at nanosecond granularity. You can
refer to the ``tests/option_timestep_5.json`` configuration for changing the
timestep to 5 ms:

.. literalinclude:: ../../tests/option_timestep_5.json
   :language: json
