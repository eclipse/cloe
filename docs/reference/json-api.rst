JSON API
========

.. highlight:: json

The JSON API provides a way for external programs to interface with Cloe
dynamically.

.. warning::
    This document is a work-in-progress and describes the target version 1 state
    and is currently not fully implemented.

/version
""""""""
Return the version of the Cloe library and engine.

::

   {
      "version": "0.7.0",
      "version_major": 0,
      "version_minor": 7,
      "version_patch": 0
      "plugin_api": {
         "simulator": "0.1.0",
         "component": "n/a",
         "controller": "0.1.0"
      },
      "package_release": "0beta.20190125.e7687690",
      "package_version": "0.7.0-0beta.20190125.e7687690",
      "timestamp": "2019-01-25",
   }

/uuid
"""""
Return the simulation UUID. This is unique for every simulation run.

::

   "0816ed4c-f6c8-43c5-b14c-dfeda7a1ac4b"

/plugins
""""""""

::

   [
      {
         "is_compatible": true,
         "name": "vtd",
         "path": "/home/captain/workspace/cloe/build/plugins/simulator_vtd.so",
         "type": "simulator",
         "version": "0.1"
      },
      {
         "is_compatible": true,
         "name": "basic",
         "path": "/home/captain/workspace/cloe/build/plugins/controller_basic.so",
         "type": "controller",
         "version": "0.1"
      },
   ]

/configuration
""""""""""""""
Return the input configuration. This is the starting point for the simulation.
If the simulation is run with the same configuration, the result should be same,
provided the rest of the context is unchanged.

?type
   One of ``active``, ``input``, or ``files``.

::

    {
        "logging": [
            {
                "name": "*",
                "level": "info"
            },
            {
                "name": "cloe/config",
                "level": "debug"
            }
        ]
    }

/simulation
"""""""""""
Return the simulation state.

::

    {
        "simulation_cycle": 8957,
        "simulation_time": "45.21s",
        "realtime_factor": 1.0,
        "achievable_realtime_factor": 595.75,
        "common_cycle_width": "20ms",
        "uuid": "0816ed4c-f6c8-43c5-b14c-dfeda7a1ac4b"
    }

/simulation/configure
"""""""""""""""""""""
Change certain simulation parameters.

::

   {
      "error": "expect POST method and JSON body",
      "fields": {
         "realtime_factor": "double? :: 1.0 is realtime, <=0.0 is unlimited",
         "reset_stats": "bool? :: reset all statistics",
         "restart": "bool? :: reset the simulation",
         "target_speed": "double? :: 1.0 is realtime, <=0.0 is unlimited"
      }
   }

/simulation/statistics
""""""""""""""""""""""

::

    {
        "current_timings": {
            "simulator/vtd": "2.489us",
            "controller/basic": "14.759us",
            "webui": "3.42us",
            "cycle": "33.571us",
            "padding": "19.96ms"
        },
        "total_timings_ms": {
            "simulator/vtd": {
                "count": 31438,
                "max": 0.142161,
                "mean": 0.00290878611870984,
                "min": 0.000419,
                "sample_std_deviation": 0.00118338246496986,
                "sample_variance": 1.40039405839814e-06,
                "std_deviation": 0.00118336364392724,
                "variance": 1.40034951376876e-06
            },
            "controller/basic": {
                "count": 31438,
                "max": 0.185976,
                "mean": 0.0195031975952667,
                "min": 0.002701,
                "sample_std_deviation": 0.00509239399841849,
                "sample_variance": 2.59324766351287e-05,
                "std_deviation": 0.00509231300671268,
                "variance": 2.59316517583351e-05
            },
            "cycle": {
                "count": 31437,
                "max": 0.217401,
                "mean": 0.045771169895346,
                "min": 0.005695,
                "sample_std_deviation": 0.0104793543318843,
                "sample_variance": 0.000109816867213182,
                "std_deviation": 0.0104791876582566,
                "variance": 0.000109813373976957
            },
        },
        "restart_count": 0
    }

/triggers/actions
"""""""""""""""""

::

    [
        "CMD", "REALTIME_FACTOR", "RESTART", "STOP"
    ]

/triggers/events
""""""""""""""""

::

    [
        "TIME", "NEXT", "START", "FAILURE", "SUCCESS", "END", "STEP"
    ]

/triggers/queue
"""""""""""""""

::

    {
        "failure": [],
        "start": [],
        "cycle": [],
        "success": [],
        "time": []
    }

/simulators
"""""""""""
::

    [
        "vtd"
    ]

/simulators/{name}
""""""""""""""""""

::

    {
        "name": "vtd",
        "is_connected": true,
        "state": "running",
        "cycle_width": "20ms",
        "num_vehicles": 1,
        "endpoints": [
            "/simulators/vtd/_state",
        ]
    }

/simulators/{name}/vehicles
"""""""""""""""""""""""""""

::

    [
        "default"
    ]

/simulators/{name}/vehicles/{vehicle_id}
""""""""""""""""""""""""""""""""""""""""

::

    {
        "id": 0,
        "name": "default",
        "from": "simulators/vtd",
        "components": [
            "cloe::default_ego_sensor",
            "cloe::default_latlong_actuator",
            "cloe::default_world_sensor",
            "cloe::gndtruth_ego_sensor",
            "cloe::gndtruth_world_sensor"
        ],
        "endpoints": []
    }

/vehicles
"""""""""

::

    [
      "default",
      "fuzzy"
    ]

/vehicles/{name}
""""""""""""""""

::

    {
        "id": 0,
        "name": "default",
        "from": "/simulators/vtd/0",
        "components": [
            "cloe::default_ego_sensor",
            "cloe::default_latlong_actuator",
            "cloe::default_world_sensor",
            "cloe::gndtruth_ego_sensor",
            "cloe::gndtruth_world_sensor"
        ],
        "endpoints": []
    }

/vehicles/{name}/components/{component_name}
""""""""""""""""""""""""""""""""""""""""""""

::

    {
        "sensed_state": {
            "acceleration": {
                "x": 0.0,
                "y": 0.0,
                "z": 0.0
            },
            "angular_velocity": {
                "x": 0.0,
                "y": 0.0,
                "z": 0.0
            },
            "cog_offset": {
                "x": 0.0,
                "y": 0.0,
                "z": 0.0
            },
            "dimensions": {
                "x": 0.0,
                "y": 0.0,
                "z": 0.0
            },
            "id": -1,
            "pose": {
                "error": "TODO(ben): not implemented"
            },
            "type": "unknown",
            "velocity": {
                "x": 0.0,
                "y": 0.0,
                "z": 0.0
            },
            "velocity_norm": 0.0
        },
        "wheel_steering_angle": 1.40360650811855e-316
    }


/controllers
""""""""""""

::

    [
        "basic",
    ]

/controllers/{name}
"""""""""""""""""""

::

    {
        "name": "basic",
        "is_connected": true,
        "state": "running",
        "cycle_width": "20ms",
        "vehicle": "fuzzy",
        "endpoints": []
    }

/controllers/{name}/ui
""""""""""""""""""""""
Return UI specification.
