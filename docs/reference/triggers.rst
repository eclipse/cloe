Triggers
========

.. highlight:: json

Cloe provides a way to trigger actions based on simulation state.

.. note::

   This section contains a basic overview of how to use triggers.
   To gain a perspective on the design considerations of triggers,
   see the :doc:`trigger design documentation <../design/0003-triggers>`.

A trigger is defined by a JSON object that specifies a trigger event and
a trigger action (and optionally, a label)::

   {
     "label": "What this trigger is all about",
     "event": { "name": "start" },
     "action": { "name": "stop" },
     "source": "filesystem",
     "optional": false,
     "conceal": false,
     "sticky": false,
   }

Each event and action can be chosen from a pre-defined set of
:doc:`available events <events>` and :doc:`available actions <actions>`.
This *must* be specified as the name of the event or action.
Each such event or action may optionally read from further fields in the
JSON object, such as the ``time`` event, which reads a number from the
``time`` field::

   { "name": "time", "time": 45.2 }

Or like the ``command`` action, which reads a string from the ``command``
field::

   { "name": "command", "command": "echo 'Hello World!'" }

.. note::
   All these names and fields are case-sensitive. In general, all names and
   fields should be in ``lower_snake_case``.

The following fields are available:

``label`` (optional)
    A freeform description about what this trigger is about.
    This is basically a comment. It's not required.

``event`` (required)
    Event configuration, as described in this document and
    :doc:`available events <events>`.

``action`` (required)
    Action configuration, as described in this document and
    :doc:`available actions <actions>`.

``optional`` (optional)
    Do not abort the simulation if the event or action does not exist.
    This can be helpful if you want to reusue a trigger list with
    different configurations.

``sticky`` (optional)
    Do not remove the trigger from the event queue after it has run.

``conceal`` (optional)
    Hide trigger in history, if the trigger can not affect the outcome
    of the simulation. This is determined through the action.
    For example, in a software simulation, the realtime factor has no
    effect on the simulation, and can be modified from the web UI with
    a slider. These inserted triggers have ``conceal`` set so they are
    not exported in the trigger history.

``source`` (descriptive)
    This field is set by Cloe and can be seen in the trigger history file.
    It specifies where the trigger originated from. The following values
    are possible:

    ``filesystem``
        Triggers that originate from the filesystem, such as stack files.

    ``network``
        Triggers that originate from the network API, such as JSON data.

    ``model``
        Triggers that originate from models, such as a simulator binding.

    ``trigger``
        Triggers that originate from triggers themselves, such as the
        ``insert`` action.

    ``instance``
        Triggers that are "reinserted" because they are sticky.

    It is useful when re-using a trigger history to filter out certain
    triggers. For example, triggers that will anyway be inserted by
    other parts of the simulation should not be re-inserted from the
    file.

``at`` (descriptive)
    This field is set by Cloe and can be seen in the trigger history file.
    It specifies the time at which the trigger was executed.

``since`` (descriptive)
    This field is set by Cloe and can be seen in the trigger history file.
    It specifies the time from which the trigger was inserted into the queue.
    If 0, then it was inserted at simulation start. This is especially relevant
    for triggers that used the ``next`` event.


Inline Format
-------------

Because this can become arduous quickly, a short-hand *inline* syntax is
supported for certain selected events and actions. In this form, instead of
a JSON object, we have a string containing the name and at most a single
argument::

   [
     { "event": "start", "action": "stop" },
     { "event": "time=45.2", "action": "command=echo 'Hello World!'" }
   ]


Usage with Cloe
---------------

Cloe can read individual triggers from the triggers section in the
:doc:`stack file <config>`. These triggers can also be in a file by themselves,
but this still must adhere to the Cloe stack-file requirements, such as::

   {
      "version": "4",
      "triggers": [
         { "event": "start", "action": "realtime_factor=-1" },
         { "event": "time=60", "action": "stop" }
      ]
   }


JSON vs YAML
""""""""""""

Since JSON is a subset of YAML, you may also use YAML in conjunction with a
converter or wrapper, which can lead to a much more readable triggers section.
Compare the following trigger representations. In JSON::

   {
     "triggers": [
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
   }

In YAML:

.. code-block:: yaml

   triggers:
   # When AEB activates:
   - event: plugin/aeb_activation
     action:
       name: insert
       triggers:

       # 1. Rip the steering wheel to the left, and
       - event: next
         action:
           name: plugin/steering_torque
           left: 50.0

       # 2. Fail if after 500ms the AEB is still activated
       - event: future=0.05
         action:
           name: insert
           triggers:
           - event: plugin/aeb_activation
             action: fail

Interactive Triggers
""""""""""""""""""""

Dynamic interactions with the simulation are achieved through the trigger
interface. This occurs under-the-hood and is transparent to the user. The
significant advantage it brings is that the order of interactions is
well-defined and traceable.

Trigger History
"""""""""""""""

In order to make the triggers that interact with a simulation traceable and
apparent, each trigger that is activated is saved in a special trigger history
container. This can be saved in a file to reproduce the simulation in exactly
the same way. There are four possibilities available for saving the trigger
history, in no particular order:

#. Insert a trigger::

      {
        "event": "finish",
        "action": "command=wget http://localhost:8080/api/triggers/history -O /tmp/trigger_history.json"
      }

#. Set ``/engine/keep_alive`` to ``true`` and fetch the history via the JSON
   API once the simulation is complete.

#. Set ``/engine/output/files/triggers`` to a file that the trigger output should
   be stored in.

#. Use the Cloe UI.
