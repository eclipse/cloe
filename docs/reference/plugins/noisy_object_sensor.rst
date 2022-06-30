.. reference-gen: noisy_object_sensor

Noisy-Object-Sensor Component
=============================

The :program:`noisy_object_sensor` component plugin applies noise to object
data.

Triggers
--------

.. todo:: Describe the triggers ``noisy_object_sensor`` makes available.

Examples
--------

The ``plugins/vtd/tests/test_vtd_smoketest.json`` test makes use of this plugin. First, the
plugin is defined in ``plugins/vtd/tests/config_vtd_infinite.json``:

.. literalinclude:: ../../../plugins/vtd/tests/config_vtd_infinite.json
   :language: json

Then it is activated in ``plugins/vtd/tests/test_vtd_smoketest.json``

.. literalinclude:: ../../../plugins/vtd/tests/test_vtd_smoketest.json
   :language: json

Defaults
--------

The following help can be viewed with :command:`cloe-engine usage noisy_object_sensor`:

.. literalinclude:: noisy_object_sensor.yaml
   :language: yaml

JSON Schema
-----------

The following can be viewed with :command:`cloe-engine usage --json noisy_object_sensor`:

.. literalinclude:: noisy_object_sensor_schema.json
   :language: json
