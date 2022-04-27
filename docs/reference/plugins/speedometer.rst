.. reference-gen: speedometer

Speedometer Component
=====================

.. todo:: Describe the purpose of the speedometer plugin.

Triggers
--------

As with vehicles, plugin trigger event names are dependent on the given name of
a plugin in a simulation. This is by default the name of the plugin itself,
which is used in this documentation.

<vehicle>/kmph
""""""""""""""
Triggers when the specified comparison with the vehicle speed evaluates to
true.

==============  ==========  ==============  ==================================
Parameter       Required    Type            Description
==============  ==========  ==============  ==================================
``is``          yes         string          | Comparison operator and numeric constant,
                                            | where operator is one of:
                                            | ``==``, ``!=``, ``<``, ``<=``, ``>``, and ``>=``.
                                            | Spaces surrounding the operator are ignored.
==============  ==========  ==============  ==================================

Inline short-form is supported as the content of ``is``.

Examples::

   [
     { "event": { "name": "default/kmph", "is": "==0" },
       "action": { "name": "insert", "triggers": [
         {"event": "next", "action": "log=info:Full-stop achieved."},
         {"event": "next", "action": "basic/hmi=enable"}
       ]}
     },
     { "label": "fail if vehicle speed exceeds 100 km/h",
       "event": "default/kmph=>100.0", "action": "fail" }
   ]



Defaults
--------

The following help can be viewed with :command:`cloe-engine usage speedometer`

.. literalinclude:: speedometer.yaml
   :language: yaml

JSON Schema
-----------

The following schema can be viewed with :command:`cloe-engine usage --json speedometer`.

.. literalinclude:: speedometer_schema.json
   :language: json
