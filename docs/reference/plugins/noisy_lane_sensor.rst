.. reference-gen: noisy_lane_sensor

Noisy-Lane-Sensor Component
===========================

The :program:`noisy_lane_sensor` component plugin applies noise to lane data.

Triggers
--------

.. todo:: Describe the triggers ``noisy_lane_sensor`` makes available.

Examples
--------

The ``tests/test_vtd_smoketest.json`` test makes use of this plugin. First, the
plugin is defined in ``tests/config_vtd_infinite.json``:

.. literalinclude:: ../../../tests/config_vtd_infinite.json
   :language: json

Then it is activated in ``tests/test_vtd_smoketest.json``

.. literalinclude:: ../../../tests/test_vtd_smoketest.json
   :language: json

Defaults
--------

The following help can be viewed with :command:`cloe-engine usage noisy_lane_sensor`:

.. literalinclude:: noisy_lane_sensor.yaml
   :language: yaml

JSON Schema
-----------

The following can be viewed with :command:`cloe-engine usage --json noisy_lane_sensor`:

.. literalinclude:: noisy_lane_sensor_schema.json
   :language: json
