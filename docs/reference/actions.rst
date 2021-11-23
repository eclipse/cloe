Available Actions
=================

.. highlight:: json

The actions and arguments are case-sensitive.

.. note::
   In some of the examples, fictitious events and actions are used. These are
   prefixed with ``plugin/``.


Built-in Actions
----------------

The following actions are built-in and are always available.

pause
"""""
Pause a running simulation, which can only be resumed with the following
trigger::

   { "event": "pause", "action": "resume" }

This will pause the simulation even if specified from the very start.

No arguments are accepted.
Inline short-form is implicitly supported.

Examples::

   [
     { "event": "time=60", "action": "pause" },
     { "event": "start", "action": "pause" }
   ]

.. note::
   Without the embedded web server enabled, the pause action would render the
   simulation unable to escape it, as the resume action must somehow be
   inserted from outside. To counter-act this, the web server will be
   temporarily started for the duration of the pause state if it is nominally
   disabled.

resume
""""""
Resume a paused simulation. Note that this only makes sense for ``pause`` event.

No arguments are accepted.
Inline short-form is implicitly supported.

Examples::

   [
     { "event": "pause", "action": "resume" }
   ]

stop
""""
Stop the simulation as neither success nor failure.

No arguments are accepted.
Inline short-form is implicitly supported.

Examples::

   [
     { "event": "time=60", "action": "stop" }
   ]

succeed
"""""""
Stop the simulation as a success.

No arguments are accepted.
Inline short-form is implicitly supported.

Examples::

   [
     { "event": "time=60", "action": "succeed" }
   ]

fail
""""
Stop the simulation as a failure. Note that this does not abort the simulation
immediately, but does cause the simulation to end.

No arguments are accepted.
Inline short-form is implicitly supported.

Examples::

   [
     { "event": "time=60", "action": "stop" }
   ]

keep_alive
""""""""""
Pause the simulation after the stop and fail events instead of tearing down
the simulation. This occurs after the aforementioned events but before the
stop event. The simulation can be induced to complete its tear-down by
sending a stop or fail trigger with the pause event.

No arguments are accepted.
Inline short-form is implicitly supported.

Examples::

   [
     { "event": "next", "action": "keep_alive" },
     { "event": "pause", "action": "stop" }
   ]

reset
"""""
Resets the simulation.

.. warning::
    The only reason why this action is provided is for testing purposes.
    It is currently an unstable feature with undefined behavior; for example:
    all triggers that have run up until this point may have been deleted.
    Or if a controller does not support the action, the simulation may crash.

No arguments are accepted.
Inline short-form is implicitly supported.

Examples::

   [
     { "event": "time=60", "action": "reset" }
   ]

realtime_factor
"""""""""""""""
Sets the target simulation speed.

==============  ==========  ==============  ==================================
Parameter       Required    Type            Description
==============  ==========  ==============  ==================================
``factor``      yes         number          | < 0 indicates unlimited speed,
                                            | = 0 indicates paused state,
                                            | > 0 indicates realtime factor,
                                            | = 1 is realtime speed.
==============  ==========  ==============  ==================================

Inline short-form is supported as the content of ``factor``.

Examples::

   [
     { "event": "start", "action": { "name": "realtime_factor", "factor": -1 } },
     { "event": "start", "action": "realtime_factor=0.2" }
   ]

.. note::
   This action is unstable and may be renamed in future releases. Additionally,
   the behavior around pause and resume states may diverge or be changed.


reset_statistics
""""""""""""""""
Reset the simulation statistics. This can be useful if an initial period of
simulation should be ignored.

No arguments are accepted.
Inline short-form is implicitly supported.

.. note::
   This action is experimental and may be removed in future releases.

log
"""
Log messages with a given severity.

==============  ==========  ==============  ==================================
Parameter       Required    Type            Description
==============  ==========  ==============  ==================================
``level``       no          string          | logging level to use, one of:
                                            | - ``trace``
                                            | - ``debug``
                                            | - ``info`` (default)
                                            | - ``warn``, ``warning``
                                            | - ``err``, ``error``
                                            | - ``fatal``, ``critical``
                                            | - ``off``, ``disabled``
``msg``         yes         string          message to log
==============  ==========  ==============  ==================================

Inline short-form is supported as an option level followed by a colon, then
an optional space followed by the message.

Examples::

   [
     { "event": "stop", "action": { "name": "log", "msg": "Simulation ended." } },
     { "event": "fail", "action": "log=critical:Failure will not be tolerated!" }
   ]

command
"""""""
Run arbitrary system commands as interpreted by the ``/bin/sh`` shell.

==============  ==========  ==============  ==================================
Parameter       Required    Type            Description
==============  ==========  ==============  ==================================
``command``     yes         string          command to execute in shell
==============  ==========  ==============  ==================================

Inline short-form is supported as the content of ``command``.

Commands run with this action can become quite complex and even take advantage
of environment variables (such as ``$PPID`` for the process ID of the Cloe
runtime) and other shell functions such as piping commands together or running
commands in the background (with ``&``). See the ``test`` directory in the
Cloe repository for several examples.

.. warning::
    This can represent a security risk and is therefore only enabled when
    ``/engine/security/enable_command_action`` is set to true. Otherwise the
    action only logs what would have been executed.

Example 1::

   [
     { "event": "stop", "action": { "name": "command", "command": "notify-send 'Simulation ended.'" } },
     { "event": "stop", "action": "command=notify-send 'Simulation ended.'" }
   ]

Example 2::

   [
    {"event": "time=45", "action": "pause"},
    {
      "label": "Insert resume trigger via curl to test the pause-resume behavior.",
      "event": "pause",
      "action": {
        "name": "command",
        "command": "echo '{\"event\": \"pause\", \"action\": \"resume\"}' | curl -d @- http://localhost:23456/api/triggers/input"
      }
    }
   ]

Example 3::

  {
    "version": "4",
    "include": [
      "config_nop_smoketest.json"
    ],
    "engine": {
       "keep_alive": true,
       "security": {
         "enable_command_action": true
       }
    },
    "triggers": [
       {
         "event": "stop",
         "action": {
         "name": "command",
         "command": "sleep 1 && kill -s INT $$PPID &"
         }
       }
     ]
   }

bundle
""""""
Wrap one or more actions into a single action. This can be used to run more
than one action together with a single event.

==============  ==========  ==============  ==================================
Parameter       Required    Type            Description
==============  ==========  ==============  ==================================
``actions``     yes         array           action objects or strings (inline
                                            short-form)
==============  ==========  ==============  ==================================

Inline short-form is **not** supported.

Examples::

   [
     {
       "event": "plugin/collision",
       "action": {
         "name": "bundle",
         "actions": [
           "command=notify-send 'Simulation failed!'",
           "fail"
         ]
       }
     }
   ]

insert
""""""
Insert one or more triggers into the simulation. This can be used to only
activate a trigger when some event has occurred.

==============  ==========  ==============  ==================================
Parameter       Required    Type            Description
==============  ==========  ==============  ==================================
``triggers``    yes         array           trigger objects
==============  ==========  ==============  ==================================

Inline short-form is **not** supported.

Example::

   [
     {
       "event": "plugin/aeb_activation",
       "action": {
         "name": "insert",
         "triggers": [
           { "event": "next", "action": { "name": "plugin/steering_torque", "left": 50.0 } }
           { "event": "future=0.05", "action": {
             "name": "insert",
             "triggers": [
               { "event": "plugin/aeb_activation", "action": "fail" }
             ]
           }}
         ]
       }
     }
   ]

push_release
""""""""""""
Push *buttons* down (by setting to ``true``) for a specified *duration*,
followed by releasing (by setting to ``false``. This eliminates the tedium
of creating multiple actions.

==============  ==========  ==============  ==================================
Parameter       Required    Type            Description
==============  ==========  ==============  ==================================
``action``      yes         string          action name
``duration``    yes         number          number of seconds to hold
``buttons``     yes         array           button names to set
==============  ==========  ==============  ==================================

Inline short-form is **not** supported.

Examples::

   [
     { "event": "time=10.0", "action": { "name": "basic/hmi", "plus": true } },
     { "event": "time=10.5", "action": { "name": "basic/hmi", "plus": false } },

     { "event": "time=15.0",
       "action": {
         "name": "push_release",
         "action": "basic/hmi",
         "duration": 0.25,
         "buttons": [ "plus" ]
       }
     }
   ]


Plugin Actions
--------------

The following actions are provided by plugins that are distributed with Cloe.
Plugin trigger action names are dependent on the given name of a plugin in a
simulation. This is by default the name of the plugin itself; this default
name is used in this documentation.

<basic>/hmi
"""""""""""
The basic controller model makes an action available to manipulate its HMI.
The name of the action is prefixed by the given name of the plugin.

==============  ==========  ==============  =====================================================
Parameter       Required    Type            Description
==============  ==========  ==============  =====================================================
``enable``      no          bool            | Switch to enable or disable
                                            | controller's ACC function.
``cancel``      no          bool            | Push button to cancel ACC function.
``resume``      no          bool            | Push button to resume ACC function.
``plus``        no          bool            | | Push button to increase speed.
                                            | Short press: increases to next 10 km/h.
                                            | Long press: increases to next 5 km/h every 500 ms.
``minus``       no          bool            | Push button to decrease speed.
                                            | Short press: decrease to next 10 km/h.
                                            | Long press: decrease to next 5 km/h every 500 ms.
``distance``    no          bool            | Push button to cycle distance profiles.
==============  ==========  ==============  =====================================================

Inline short-form is supported as a comma-separated list of fields to be
pressed, optionally prefixed with ``!`` to indicate release:

.. code:: text

    basic/hmi=!enable
    basic/hmi=plus,minus
    basic/hmi=!plus,!minus

The push-button interface is tricky to interface with correctly, since usually
the effect triggers on release (like clicking a button with a mouse).
Additionally, effect is often different if the button is held pressed for
a certain period of time. The result is that pushing down on the button
(by setting the field to ``true``) must be accompanied by releasing the button
(by setting the field to ``false``).

If a field is not specified, it is ignored.

.. warning::
    If a field is set and unset very quickly, it is therefore possible that
    both register for the same frame, in which case button presses would
    cancel each other out.

    For this reason, it is recommended to keep the simulation at realtime
    speeds when dynamically inserting this kind of event.

Examples::

   [
     { "event": "time=5.0", "action": { "name": "basic/hmi", "enable": true } },
     {
       "label": "Resume with current speed",
       "event": "time=10.0",
       "action": { "name": "basic/hmi", "resume": true }
     },
     { "event": "time=10.5", "action": "basic/hmi=!resume" },
     { "event": "time=15.0",
       "action": {
         "name": "push_release",
         "action": "basic/hmi",
         "duration": 0.25,
         "buttons": [ "plus" ]
       }
     }
   ]
