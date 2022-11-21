Stack Configuration
===================


The *Cloe Stack* describes the entire closed-loop system, including the Cloe
runtime, but not limited to it. It is described by a Cloe Stack configuration,
written in the JSON format.

Each Cloe Stack can be configured through one or more configuration files. Each
configuration file may contain a subset of keys, each of which documents
different aspects of the simulation.

In order to make configuration of various sections orthogonal, there is a
keyword that has special meaning as key in the configuration:

args
    Inside the configuration of any self-contained simulation component that is
    added to the system e.g. via the ``binding`` parameter, the special ``args``
    section contains extra configuration for the binding itself. It is passed
    through as-is, mostly without any pre-processing by the runtime.

    Example::

      simulators:
        - binding: "vtd"
          args:
            rdb_params:
              retry_delay_s: 1.0

Some of the following sections are optional, others are not. The required
sections differ in that they can be required *locally*, in every single
configuration file, or they can be required *globally*, across the entire merged
stack configuration.

See the ``tests/`` directory in the Cloe repository for examples of stack files
that are directly processed by the Cloe runtime.


.. _config-version:

version
-------
Locally required.

The version of the schema used in the document. Currently must be the string
``"4"``. How different schema versions will be supported in the future is
undecided.

This is the only key which is required in each configuration file.


.. _config-engine:

engine
------
Optional.

Configuration settings for the Cloe runtime.

Defaults::

   engine:
     keep_alive: false
     ignore: []
     hooks:
       pre_connect: []
       post_disconnect: []
     output:
       clobber: true
       path: "${CLOE_SIMULATION_UUID}"
       files:
         triggers: "triggers.json"
         statistics: "statistics.json"
         config: "config.json"
     registry_path: "${XDG_DATA_HOME-${HOME}/.local/share}/cloe/registry"
     plugin_path:
       - "/usr/local/lib/cloe"
       - "/usr/lib/cloe"
     plugins:
       allow_clobber: true
       ignore_failure: false
       ignore_missing: false
     polling_interval: 100
     security:
       enable_command_action: false
       enable_include_section: true
       enable_hooks_section: true
       max_include_depth: 64
     triggers:
       ignore_source: false
     watchdog:
       mode: off
       default_timeout: 90000
       state_timeouts:
         CONNECT: 300000
         ABORT: 90000
         STOP: 300000
         DISCONNECT: 600000


.. _config-server:

server
------
Optional.

Configuration settings for the server embedded in the Cloe engine. Disable the
server for better performance. Listen on all addresses by setting
`listen_address` to `0.0.0.0`. Change the prefixes only if you really really
know what you are doing!

Defaults::

    server:
      listen: true
      listen_address: 127.0.0.1
      listen_port: 8080
      listen_threads: 10
      static_prefix: ""
      api_prefix: "/api"


.. _config-include:

include
-------
Optional.

A list of files to include, in order; must be JSON and are directly interpreted
by ``cloe-engine``. Relative paths are interpreted according to the input file.
If the input stack configuration is not a file, or is stdin, then relative
to the current-working-directory of ``cloe-engine``.

Example::

    include:
        - "../common.json"
        - "../default_simulator_vtd.json"
        - "../tests/acc.json"


.. _config-plugins:

plugins
-------
Optional.

A list of plugin paths to load. Note that any plugin that is not located
inside the directories listed in the ``/engine/plugin_path`` configuration or
contained in the ``CLOE_PLUGIN_PATH`` environment variable must be explicitly
specified in the ``plugins`` section.

Example 1::

   plugins:
     - path: "../build/plugins/simulator_vtd.so"
     - path: "../build/plugins/component_noisy_object_sensor.so"
       name: "noisy_object_sensor"

Example 2::

   plugins:
     - path: "../build/plugins"


.. _config-defaults:

defaults
--------
Optional.

A map of defaults for the ``simulators`` and ``controllers`` sections.
These are applied during instantiation of a binding, based on the given name
and the binding name (at the plugin level).

Example::

   defaults:
     simulators:
       - binding: "vtd"
         args:
            label_vehicle: "symbol"
       - name: "never_instantiated"
         args:
            distribution: "normal"


.. _config-simulators:

simulators
----------
Globally required.

Example::

   simulators:
     - binding: "vtd"
       args:
         rdb_params:
           retry_delay_s: 1.0
         scenario: "acc.xml"
         image_generator: false


.. _config-vehicles:

vehicles
--------
Globally required.

A list of vehicles, specifying where the vehicle comes from, and what it's
identifier is. Components of the vehicle can be modified through a map.

Example::

   vehicles:
     - name: "default"
       from:
         simulator: "vtd"
         index: 0
       components:
         "cloe::default_world_sensor":
           binding: "noisy_object_sensor"
           name: "noisy_object_sensor"
           from: "cloe::default_world_sensor"
           args:
             distribution:
               binding: "normal"
               mean: 0.0
               std_deviation: 0.3


.. _config-controllers:

controllers
-----------
Globally required.

A list of controllers and the vehicles they are bound to.

Example::

   controllers:
     - binding: "basic"
       vehicle: "default"
     - binding: "virtue"
       vehicle: "default"


.. _config-triggers:

triggers
--------
Optional.

A list of triggers as defined by :doc:`triggers`.

Example::

  triggers:
    - event: "start"
      action: {
        "name": "bundle",
        "actions": [
          "command=echo 'Start simulation.'",
          "basic/hmi=!enable"
        ]
      }
    - { "event": "next=1",   "action": "basic/hmi=enable" }
    - { "event": "time=5",   "action": "basic/hmi=resume" }
    - { "event": "time=5.5", "action": "basic/hmi=!resume" }
    - label: Push and release basic/hmi=plus
      event: time=6
      action: {
        "name": "insert",
        "triggers": [
          { "event": "next", "action": "basic/hmi=plus" },
          { "event": "next=1", "action": "basic/hmi=!plus" }
        ]
      }


.. _config-simulation:

simulation
----------
Optional.

Several settings that control the way a simulation is run.

abort_on_controller_failure
   Defines whether Cloe shall abort when the controller throws an error.

   Optional. Default is ``true``.

controller_retry_limit
   Number of times a controller is asked for progress in each time step before
   the simulation is aborted. If set to a negative value, the controller can
   retry an unlimited number of times.

   Optional. Default is ``1000``.

controller_retry_sleep
   Time in milliseconds Cloe will wait after an unsuccessful try waiting for
   the controller to progress.

   Note: It is not recommended to set this to 0.

   Optional. Default is ``1`` ms.

model_step_width
  Stepwidth of the Cloe simulation time in nanoseconds.

  Optional. Default is ``20e6`` ns, which is 20 ms.

namespace
  Namespace for simulation events and actions defined by the Cloe library.

  Optional. Default is ``"cloe"``.

Example::

  simulation:
    namespace: "cloe"
    abort_on_controller_failure: true
    controller_retry_limit: 100
    controller_retry_sleep: 5
    model_step_width: 10e6


.. _config-logging:

logging
-------
Optional.

A list of logging configurations, applied in sequence. There are three
parameters each object can take:

name
   Required. The name of the logger, such as ``cloe`` or ``vtd/signs``.
   These are seen in the logging output. The special value ``*`` applies to
   all loggers.

level
   Optional. The new level at which this logger should log. The default
   verbosity is ``info``. Values can be one of: ``fatal``, ``error``, ``warn``,
   ``info``, ``debug``, and ``trace``.

pattern
   Optional. The format that should be used for logging. See the spdlog
   `documentation <https://github.com/gabime/spdlog/wiki/3.-Custom-formatting>`__.

Example::

   logging:
     - name: "vtd/signs"
       level: "error"
