Basic
=====

The following help can be viewed with ``cloe-engine usage basic``:

.. runcmd:: cloe-launch exec -P ../conanfile.py -- usage basic
   :replace: "Path:.*\\//Path: <SOME_PATH>\\/"
   :syntax: yaml

JSON Schema
-----------

.. runcmd:: cloe-launch exec -P ../conanfile.py -- usage --json basic
   :replace: "\"\\$id\":.*\\//\"\\$id\": \"<SOME_PATH>\\/"
   :syntax: json
