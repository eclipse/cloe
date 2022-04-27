Built-in Events
===============

.. highlight:: json

The following events are built-in and are always available.
The events and arguments are case-sensitive.

.. note::

   In some of the examples, fictitious events and actions are used. These are
   prefixed with ``plugin/``.

time
""""
Triggers at the specified time in the simulation.

==============  ==========  ==============  ==================================
Parameter       Required    Type            Description
==============  ==========  ==============  ==================================
``time``        yes         number          Seconds in simulation time.
==============  ==========  ==============  ==================================

Inline short-form is supported as the content of ``time``.

Examples::

   [
     { "event": { "name": "time", "time": 10.5 }, "action": "stop" },
     { "event": "time=10.5", "action": "stop" }
   ]

next
""""
Triggers in the next simulation cycle, except if the simulation is paused.
On insertion, the current simulation time is used to insert a special time
trigger. The value of the parameter ``time`` is added to this time. Therefore,
it is possible to insert "relative" time triggers.

==============  ==========  ==============  ==================================
Parameter       Required    Type            Description
==============  ==========  ==============  ==================================
``time``        yes         number          Additional seconds in simulation time.
==============  ==========  ==============  ==================================

Inline short-form is supported as the content of ``time``.

Examples::

   [
     { "event": { "name": "next" }, "action": {"name": "basic/hmi", "plus": true } },
     { "event": { "name": "next", "time": 0.5 }, "action": {"name": "basic/hmi", "plus": false } },
     { "event": "next=30", "action": "stop" }
   ]

start
"""""
Triggers when the simulation is started.

No arguments are accepted.
Inline short-form is implicitly supported.

stop
""""
Triggers when the simulation is stopped, regardless whether it is a success or
failure.

No arguments are accepted.
Inline short-form is implicitly supported.

.. note::
   When a simulation is aborted by the user, no triggers are called!

success
"""""""
Triggers when the simulation is successfully stopped.

No arguments are accepted.
Inline short-form is implicitly supported.

failure
"""""""
Triggers when the simulation is a failure (but stopped).

No arguments are accepted.
Inline short-form is implicitly supported.

.. note::
   This is not the same as the abortion of a simulation. When a simulation is
   aborted by the user, no triggers are called!

pause
"""""
Triggers when the simulation is paused.

No arguments are accepted.
Inline short-form is implicitly supported.

resume
""""""
Triggers when the simulation is resumed after being paused.

No arguments are accepted.
Inline short-form is implicitly supported.
