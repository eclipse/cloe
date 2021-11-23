Creating a Stackfile
====================

We've been re-using stackfiles that came with Cloe, but these won't cover any
new use-cases. In this section we'll introduce you to the fundamentals of
creating a new stackfile to configure a simulation.

Cloe relies on :doc:`stack files <../reference/config>` as a single-source
specification for what a simulation consists of. Before running even a very
basic simulation, we need one of these:

.. code-block:: json

   {
      "version": "4",
      "simulators": [
         {
            "binding": "nop"
         }
      ],
      "vehicles": [
         {
            "name": "ego",
            "from": {
               "simulator": "nop",
               "index": 0
            }
         }
      ],
      "controllers": [
         {
            "binding": "basic",
            "vehicle": "ego"
         }
      ]
   }

Wow, Quite a chunk! But trust me, by accepting a little complexity overhead, we
will save ourselves a lot of trouble later! So let's break this down section
by section:

version
   By specifying exactly which version of the Cloe stack specification we are
   using, we communicate in very clear terms how the configuration file should
   be interpreted.

simulators
   We need to specify at least one simulator to use. This is where we will get
   our vehicles from initially. We use the *nop* simulator binding, which does
   very little, but is also builtin, and is useful for testing purposes.
   Since we don't give it a name, it gets the name *nop* by default.

vehicles
   We must make a vehicle available for our controller. We do this by giving it
   a unique name, in this case *ego*, and specifying exactly where the vehicle
   comes from. We only have one simulator binding, *nop*, so we'll use that,
   and we'll use the very first vehicle it has. Since we are software
   developers, we start with the index 0.

controllers
   Since a vehicle by itself is fairly boring (at least for simulation
   purposes), we want to add a controller to the vehicle. In the simulation
   framework, one vehicle can have many controllers assigned to it, but each
   controller can only be assigned to a single vehicle. If a controller is not
   assigned to a vehicle, it is considered disabled, since it's fully isolated
   from the simulated world. We choose the *basic* controller. This is not
   builtin, but it's provided by Cloe for demo purposes, so it has no
   dependencies. Since we don't give it a name, it gets the default name
   *basic*. We then set the vehicle to use the vehicle we previously specified
   and which we named *ego*.

.. note::
   If you are using the ``cloe-launch`` tool, the paths of all required plugins
   will be added to the ``CLOE_PLUGIN_PATH`` environment variable and the basic
   controller plugin will automatically be loaded. If you need to specify the
   plugin path explicitly, refer to the 'plugins' section in the
   :doc:`stack file reference documentation <../reference/config>`.

Save the stack file somewhere, as ``config_nop_basic.json``. This is our
single-source, comprehensive description of a simulation. Next we can run
the simulation!
