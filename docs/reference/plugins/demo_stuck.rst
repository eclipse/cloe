Demo Stuck
==========

This controller is a controller that can be configured to run very slowly and
get stuck at certain point in the simulation. This is primarily used for system
testing.

The following help can be viewed with ``cloe-engine usage demo_stuck``:

.. runcmd:: ../build/cloe-engine usage ../build/plugins/controller_demo_stuck.so
   :syntax: yaml

JSON Schema
-----------

.. runcmd:: ../build/cloe-engine usage --json ../build/plugins/controller_demo_stuck.so
   :syntax: json
