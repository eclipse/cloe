.. reference-gen: gndtruth_extractor

Groundtruth Extractor
=====================

The :program:`gndtruth_extractor` controller plugin doesn't actually control
the vehicle in anyway. Instead it is used as a utility to dump specified
vehicle components to file.

================  =============
File Type         Output Type
================  =============
JSON              "json"
JSON.gz           "json.gz"
MessagePack       "msgpack"
MessagePack.gz    "msgpack.gz"
================  =============

.. note::
   The filename is taken literally, i.e., no file-extensions are added
   automatically.

Example
-------

See the following file used in the tests: ``plugins/vtd/tests/controller_gndtruth.json``:

.. literalinclude:: ../../../plugins/vtd/tests/controller_gndtruth.json
   :language: json

.. highlight:: json

Given the following configuration::

   {
     "components": [
       "cloe::gndtruth_lane_sensor"
     ]
   }

Then this would be the output if JSON is used::

   {
     "components": {
       "cloe::gndtruth_lane_sensor": {
         "etc": "..."
       }
     }
   }

The content of the output depends on the YAML and the respective simulation
tool and therefore is not part of this documentation.

Defaults
--------

The following help can be viewed with :command:`cloe-engine usage gndtruth_extractor`:

.. literalinclude:: gndtruth_extractor.yaml
   :language: yaml

JSON Schema
-----------

The following can be viewed with :command:`cloe-engine usage --json gndtruth_extractor`:

.. literalinclude:: gndtruth_extractor_schema.json
   :language: json
