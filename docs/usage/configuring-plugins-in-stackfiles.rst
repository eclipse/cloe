Configuring Plugins in Stackfiles
=================================

All plugins that are part of a simulation can provide configuration through
stackfiles. This section will show you how you can find out which configuration
is possible for each plugin.

Try the following command to see which plugins are available based on your
provided conanfile, such as the example ``tests/conanfile_default.py``::

    $ cloe-launch exec tests/conanfile_default.py -- usage

    [...]

    ---

    Available simulators:
    minimator [<SOME_PATH>/lib/cloe/simulator_minimator.so]
    nop       [builtin://simulator/nop]

    Available controllers:
    basic              [<SOME_PATH>/lib/cloe/controller_basic.so]
    gndtruth_extractor [<SOME_PATH>/lib/cloe/controller_gndtruth_extractor.so]
    demo_printer       [<SOME_PATH>/lib/cloe/controller_demo_printer.so]
    demo_stuck         [<SOME_PATH>/lib/cloe/controller_demo_stuck.so]
    virtue             [<SOME_PATH>/lib/cloe/controller_virtue.so]
    nop                [builtin://controller/nop]

    Available components:
    noisy_lane_sensor   [<SOME_PATH>/lib/cloe/component_noisy_lane_sensor.so]
    noisy_object_sensor [<SOME_PATH>/lib/cloe/component_noisy_object_sensor.so]
    speedometer         [<SOME_PATH>/lib/cloe/component_speedometer.so]

To see the configuration options of the controller ``basic``, do::

    cloe-launch exec tests/conanfile_default.py -- usage basic

.. literalinclude:: ../reference/plugins/basic.yaml
   :language: yaml
