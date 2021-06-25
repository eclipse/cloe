Reference
=========

This chapter contains reference documentation for various systems in Cloe.
It is assumed that you already know how the systems work and what they are
there for. See the chapters :doc:`overview` and :doc:`usage` if you need more
background knowledge for any of the following topics.

Section :doc:`reference/system-states` provides an overview of the structure of
a simulation loop.

Section :doc:`reference/model-states` provides an overview of when the various
methods of a simulation model are executed.

Section :doc:`reference/config` provides an overview of all the different
parameters that can be written in a Cloe stack file. These configure the
runtime and the orchestration of a simulation.

Section :doc:`reference/watchdog` provides a detailed explanation of the
watchdog feature of the Cloe runtime.

Section :doc:`reference/json-api` provides an overview of all endpoints that
are made available from the Cloe runtime. These can be used to build interfaces
to Cloe or utility tools for simulations.

Section :doc:`reference/triggers` provides an overview of how to use triggers
with Cloe.

Section :doc:`reference/events` provides an overview of all available trigger
events.

Section :doc:`reference/actions` provides an overview of all available trigger
actions.

Section :doc:`reference/plugins` provides an overview of all plugins that are
part of the Cloe distribution.

----

.. toctree::
    reference/system-states
    reference/model-states
    reference/config
    reference/watchdog
    reference/json-api
    reference/triggers
    reference/events
    reference/actions
    reference/plugins
