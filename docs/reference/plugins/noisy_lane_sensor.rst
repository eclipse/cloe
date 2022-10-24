.. reference-gen: noisy_lane_sensor

Noisy-Lane-Sensor Component
===========================

The :program:`noisy_lane_sensor` component plugin applies noise to lane data.

Triggers
--------

.. todo:: Describe the triggers ``noisy_lane_sensor`` makes available.

Examples
--------

The ``optional/vtd/tests/test_vtd_smoketest.json`` test makes use of this plugin.
First, the plugin is defined in ``optional/vtd/tests/config_vtd_infinite.json``:

.. literalinclude:: ../../../optional/vtd/tests/config_vtd_infinite.json
   :language: json

Then it is activated in ``optional/vtd/tests/test_vtd_smoketest.json``

.. literalinclude:: ../../../optional/vtd/tests/test_vtd_smoketest.json
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
