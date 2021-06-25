Virtue
======

The following help can be viewed with ``cloe-engine usage virtue``:

.. runcmd:: cloe-launch exec -P ../conanfile.py -- usage virtue
   :replace: "Path:.*\\//Path: <SOME_PATH>\\/"
   :syntax: yaml

JSON Schema
-----------

.. runcmd:: cloe-launch exec -P ../conanfile.py -- usage --json virtue
   :replace: "\"\\$id\":.*\\//\"\\$id\": \"<SOME_PATH>\\/"
   :syntax: json
