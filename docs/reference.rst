Reference
=========

This chapter contains reference documentation for various systems in Cloe.
It is assumed that you already know how the systems work and what they are
there for. See the chapters :doc:`overview` and :doc:`usage` if you need more
background knowledge for any of the following topics.

`Doxygen <api/classes.html>`_
   provides C++ API documentation generated by Doxygen.

:doc:`reference/system-states`
   provides an overview of the structure of a simulation loop.

:doc:`reference/model-states`
   provides an overview of when the various methods of a simulation model are
   executed.

:doc:`reference/config`
   provides an overview of all the different parameters that can be written in
   a Cloe stack file. These configure the runtime and the orchestration of
   a simulation.

:doc:`reference/hooks`
   provides an explanation of how to use pre-connect and post-disconnect hooks.

:doc:`reference/variables`
   provides an explanation of variable interpolation in input stack files.

:doc:`reference/watchdog`
   provides a detailed explanation of the watchdog feature of the Cloe runtime.

:doc:`reference/json-api`
   provides an overview of all endpoints that are made available from the Cloe
   runtime. These can be used to build interfaces to Cloe or utility tools for
   simulations.

:doc:`reference/triggers`
   provides an overview of how to use triggers with Cloe.

:doc:`reference/events`
   provides an overview of all available trigger events.

:doc:`reference/actions`
   provides an overview of all available trigger actions.

:doc:`reference/plugins`
   provides an overview of all plugins that are part of the Cloe distribution.

:doc:`reference/lua-initialization`
   provides an overview of how the engine processes a Lua file.

.. toctree::
   :hidden:

   Doxygen <api/classes.html#http://>
   reference/system-states
   reference/model-states
   reference/config
   reference/hooks
   reference/variables
   reference/watchdog
   reference/json-api
   reference/triggers
   reference/events
   reference/actions
   reference/plugins
   reference/lua-initialization
