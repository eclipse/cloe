Demo Printer
============

The following help can be viewed with ``cloe-engine usage demo_printer``:

.. runcmd:: cloe-launch exec -P ../conanfile.py -- usage demo_printer
   :replace: "Path:.*\\//Path: <SOME_PATH>\\/"
   :syntax: yaml

JSON Schema
-----------

.. runcmd:: cloe-launch exec -P ../conanfile.py --  usage --json demo_printer
   :replace: "\"\\$id\":.*\\//\"\\$id\": \"<SOME_PATH>\\/"
   :syntax: json
