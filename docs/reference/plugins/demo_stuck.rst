Demo Stuck
==========

This controller is a controller that can be configured to run very slowly and
get stuck at certain point in the simulation. This is primarily used for system
testing.

The following help can be viewed with ``cloe-engine usage demo_stuck``:

.. runcmd:: cloe-launch exec -P ../conanfile.py -- usage demo_stuck
   :replace: "Path:.*\\//Path: <SOME_PATH>\\/"
   :syntax: yaml

JSON Schema
-----------

.. runcmd:: cloe-launch exec -P ../conanfile.py -- usage --json demo_stuck
   :replace: "\"\\$id\":.*\\//\"\\$id\": \"<SOME_PATH>\\/"
   :syntax: json
