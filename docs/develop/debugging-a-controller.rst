Debugging a Controller
======================

Consider the below illustration of the standard communication pattern between
Cloe and a controller::

                               "controller" to debug
                         _________________________________
                        /                                 \

   ┌────────────┐
   │            │       ┌──────────┐         ┌────────────┐
   │    Cloe    │ <---> │  Plugin  │ <-----> │ Controller │
   │            │       └──────────┘         └────────────┘
   └────────────┘           (1)                   (2)

   \_______________________________/
             runtime process

As a developer of a controller, you will find yourself in the situation that
you want to debug both the *controller* (2) and the controller *plugin* (1),
though not necessarily at the same time. From a Cloe perspective, these are two
very separate use-cases.

.. note::
   Debugging while the :doc:`watchdog <../reference/watchdog>` is active will
   usually cause the watchdog to activate, so make sure you don't have it
   enabled if you want to debug without interruptions.

Debugging the Plugin (1)
------------------------

The controller plugin is loaded as a dynamic library (an ``.so`` file). This
integrates it into the Cloe engine and thus can be debugged within the engine
executable. You can simply set your breakpoints for the not-yet loaded plugin.

This can be done, for example in Visual Studio Code or on the command line with
GDB:

.. asciinema:: debugging-with-gdb.asc
   :preload: 1
   :idle-time-limit: 1

This kind of debugging usually works without any problems.

Debugging the Controller (2)
----------------------------

Occasionally, the controller itself is also part of the plugin, as is the
case with the example *basic* controller. In that case, the process of
debugging is the same as when debugging the plugin (1).

Normally though, the controller is in a separate process, outside of the Cloe
runtime engine. It may even be an entire set of processes or coupled to
different hardware. You should be able to debug your controller with whatever
mechanisms are normally used.

There is just one caveat that *may* apply. If the communication between the
plugin (1) and the controller (2) is asynchronous to the extent that the
controller plugin is called multiple times in one time step, we need to make
sure that Cloe doesn't think the controller died. This can be done disabling
the ``controller_retry_limit`` using the following configuration values in your
stack file:

.. code-block:: json

   {
      "version": "4",
      "simulation": {
         "controller_retry_limit": -1,
         "controller_retry_sleep": 10
      }
   }

The ``/simulation/controller_retry_limit`` value is a positive integer by
default, which limits the controller plugin to retry a process step that many
times. If you are debugging the controller (2) and the plugin (1) loops and
loops while waiting for controller (2) to respond, you can reach this limit
quite quickly. In that case, the correct action is to disable the retry limit.

For more information, see the documentation on the :ref:`simulation section<config-simulation>`.
