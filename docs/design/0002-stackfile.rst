CDR 3 - Stackfiles
==================

.. highlight:: json

A Cloe *stack* (or *stackfile*) is a configuration document written in `JSON`_
which describes how Cloe should run a simulation.

.. note::

   This section concerns itself with the design rationale of parts of the stack
   in Cloe. If you want to know how to create a stack file, see the
   :doc:`stack reference <../reference/config>`.

In the past, it was difficult to define how a simulation was to be run.
This would be a combination of starting several services in a specific order
and passing the correct arguments to each of these. There was no single way
to specify a reproducible simulation configuration. This led to several
inconveniences:

- Automated setup of a simulation was only possible through hard-coding special
  scenarios into a startup script.
- The flavors were hard-coded in a big if-else chain in the startup script.
- Only a single simulator binding was supported.
- Execution was only possible on the localhost.
- Existence of many prerequisites -- files and dependencies -- was assumed
  rather than guaranteed or checked.
- Specification of what should be run was accomplished through passing an
  enormous amount of parameters to the correct commands. Despite this, the
  possibilities of what could be achieved were still strongly limited.

The term *stack* derives from `Docker`_, since the problem that Cloe is trying
to solve is a similar one in style and scale, specific to the simulation of
ADAS functions. A Cloe stack is a comprehensive description of an entire
simulation run [1]_. This can act as the single source of configuration, while
supporting include and merge semantics for modularity.

A stackfile consists of several sections, each of which describes a different
part of the simulation framework. The titles in this section are `JSON
pointers`_ to structures in the stack configuration.


Stack Sections
--------------

/vehicles
~~~~~~~~~

Currently, the Cloe engine can read basic descriptions of a vehicle: where it
originates and how it should be named. This shall be expanded to include more
detailed configuration of a vehicle through its components.

An example configuration where components are described is::

   [
      {
         "name": "default",
         "from": {
            "index": 0,
            "binding": "vtd"
         },
         "components": {
            "default_world_sensor": {
               "require": true
            },
            "default_actuator": {
               "require": true,
               "assert": "latlong_actuator"
            },
            "default_ego_sensor": {
               "require": true
            }
         }
      },
      {
         "name": "fuzzy",
         "from": "default",
         "components": [
            {
               "name": "default_world_sensor",
               "component": "gauss_filter",
               "replace": "default_world_sensor",
               "args": {
                  "mean": 1.0,
                  "std_dev": 0.5
               }
            }
         ]
      }
   ]


----

.. rubric:: Footnotes
.. [1]
   It is plausible that the Cloe stack file might be extended to describe not
   just one simulation run, but a whole series of simulation runs.

.. _Docker: https://docs.docker.com/get-started/part5/
.. _JSON: https://json.org
.. _YAML: https://yaml.org
.. _JSON pointers: https://tools.ietf.org/html/rfc6901
