Plugins
=======

..
    TODO(release) // Generate plugin documentation

    1. Create Conan packages for the entire project locally or in a dev
       container.

    2. Run `docs/reference-gen.sh` to update the plugin documentation.

    Make sure that the VERSION file is set with the version about to be released
    when creating the packages, otherwise the reference documentation will
    contain references to the wrong version.

As much "external" functionality as possible is implemented through plugins.
These provide an orthogonal, *horizontal* extension of the Cloe system. Only
two of these plugins are built-in to Cloe itself, the ``nop`` simulator and
``nop`` controller.

.. rubric:: Simulator Bindings

Each simulation requires at least one simulator binding. The following
simulator bindings are supplied by Cloe. However, to make use of the VTD
or any other external simulator bindings, you will need to have a licensed
installation of these simulators.

.. toctree::
   :maxdepth: 1

   plugins/nop_simulator
   plugins/minimator
   plugins/esmini
   plugins/vtd

.. rubric:: Controller Plugins

Each simulation may have one or more controllers connected to vehicles supplied
by the simulator binding. The following controllers are supplied by Cloe and
have no prerequisites for their use.

.. toctree::
   :maxdepth: 1

   plugins/nop_controller
   plugins/demo_printer
   plugins/demo_stuck
   plugins/gndtruth_extractor
   plugins/basic
   plugins/virtue

.. rubric:: Component Plugins

A vehicle in a simulation is provided by a simulator binding, but can be
further modified using component plugins. The following components are supplied
by Cloe and have no prerequisites for their use.

.. toctree::
   :maxdepth: 1

   plugins/noisy_object_sensor
   plugins/noisy_lane_sensor
   plugins/speedometer
