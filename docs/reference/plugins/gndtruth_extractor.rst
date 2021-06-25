Groundtruth Extractor
=====================

The following help can be viewed with ``cloe-engine usage gndtruth_extractor``:

.. runcmd:: ../build/cloe-engine usage ../build/plugins/controller_gndtruth_extractor.so
   :syntax: yaml

+----------------+--------------+
| File-Type      | output_type  |
+================+==============+
| JSON           | "json"       |
+----------------+--------------+
| JSON.gz        | "json.gz"    |
+----------------+--------------+
| MessagePack    | "msgpack"    |
+----------------+--------------+
| MessagePack.gz | "msgpack.gz" |
+----------------+--------------+

Note: The filename is taken literally. I.e. no file-extensions are added automatically.

JSON Schema
-----------

.. runcmd:: ../build/cloe-engine usage --json ../build/plugins/controller_gndtruth_extractor.so
   :syntax: json

Example
-------

.. highlight:: json

Given the following configuration::

   {
     "components": [
       "cloe::gndtruth_lane_sensor"
     ]
   }

Then this would be the output if JSON is used:::

  {
    "components": {
      "cloe::gndtruth_lane_sensor": {
        "etc": "..."
      }
    }
  }

The content of the output depends on the YAML and the respective simulation
tool and therefore is not part of this documentation.
