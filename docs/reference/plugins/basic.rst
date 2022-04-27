.. reference-gen: basic

Basic Controller
================

The :program:`basic` controller plugin provides limited :term:`ACC`,
:term:`AEB`, and :term:`LKA` functionality. It is used for internal integration
testing and as an example of how you can write a controller plugin.

Triggers
--------

The following actions are provided by the plugin. Plugin trigger action names
are dependent on the given name of a plugin in a simulation. This is by default
the name of the plugin itself; this default name is used in this documentation.

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

Example Tests
-------------

You can see an example configuration in the ``tests/controller_basic.json``
file, which is included by almost all smoketests run.

.. literalinclude:: ../../../tests/controller_basic.json
   :language: json

Defaults
--------

The following help can be viewed with :command:`cloe-engine usage basic`:

.. literalinclude:: basic.yaml
   :language: yaml

JSON Schema
-----------

The following can be viewed with :command:`cloe-engine usage --json basic`:

.. literalinclude:: basic_schema.json
   :language: json
