.. reference-gen: virtue

Virtue Controller
=================

The :program:`virtue` controller plugin checks that the simulation does not
exhibit irrationality or invalid behavior. There are a few `Checker` classes
in the `virtue.cpp` implementation that are of interest:

RationalityChecker
   Checks that the ego object does not change size.
   Checks that the normed velocity is not negative.

SafetyChecker
   Checks that reported acceleration does not exceed 20 m/s.
   Checks that acceleration derived from velocity deltas does not exceed 20 m/s.
   Checks that simulation step increases by one in each check pass.

MissingLaneBoundariesChecker
   Checks that there are no missing lane boundaries, given the configured
   components.

Triggers
--------

As with vehicles, plugin trigger event names are dependent on the given name of
a plugin in a simulation. This is by default the name of the plugin itself,
which is used in this documentation.

<virtue>/failure
""""""""""""""""
Triggers when the virtue controller detects a violation of requirements.

No arguments are accepted.
Inline short-form is implicitly supported.

Defaults
--------

The following help can be viewed with :command:`cloe-engine usage virtue`:

.. literalinclude:: virtue.yaml
   :language: yaml

JSON Schema
-----------

The following can be viewed with :command:`cloe-engine usage --json virtue`:

.. literalinclude:: virtue_schema.json
   :language: json
