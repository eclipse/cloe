Running a Simulation
====================

.. highlight:: console

The basic step to running a simulation is to instruct cloe-engine to ``run``
one or more stackfiles::

   $ cloe-engine run tests/config_minimator_smoketest.json

If you want to see a little more logging output, you set the default logging
level to something a little more verbose, like ``debug``::

   $ cloe-engine -l debug run tests/config_nop_smoketest.json
   II 13:54:51.582 [cloe] Include conf /home/captain/.config/cloe/config.json
   DD 13:54:51.582 [cloe] Skip /usr/local/lib/cloe
   DD 13:54:51.582 [cloe] Skip /usr/lib/cloe
   II 13:54:51.583 [cloe] Include conf tests/config_nop_infinite.json
   II 13:54:51.583 [cloe] Include conf tests/controller_virtue.json
   DD 13:54:51.584 [cloe] Insert plugin tests/../build/plugins/controller_virtue.so
   DD 13:54:51.584 [cloe] Load plugin /home/captain/workspace/cloe/build/plugins/controller_virtue.so
   II 13:54:51.585 [cloe] Include conf tests/controller_basic.json
   DD 13:54:51.585 [cloe] Insert plugin tests/../build/plugins/controller_basic.so
   DD 13:54:51.585 [cloe] Load plugin /home/captain/workspace/cloe/build/plugins/controller_basic.so
   DD 13:54:51.591 [cloe] Insert plugin tests/../build/plugins/component_noisy_object_sensor.so
   DD 13:54:51.591 [cloe] Load plugin /home/captain/workspace/cloe/build/plugins/component_noisy_object_sensor.so
   II 13:54:51.599 [cloe] Initializing simulation...
   DD 13:54:51.599 [cloe] Register static endpoint:   /
   DD 13:54:51.599 [cloe] Register static endpoint:   /index.html
   ...
   II 13:54:51.765 [cloe] Stop simulation.
   DD 13:54:51.765 [cloe] Writing file: /home/captain/.local/share/cloe/registry/208dddb2-a429-4b22-80b9-cc6be68053b9/result.json
   DD 13:54:51.769 [cloe] Writing file: /home/captain/.local/share/cloe/registry/208dddb2-a429-4b22-80b9-cc6be68053b9/config.json
   DD 13:54:51.771 [cloe] Writing file: /home/captain/.local/share/cloe/registry/208dddb2-a429-4b22-80b9-cc6be68053b9/triggers.json
   II 13:54:51.773 [cloe] Wrote 3 output files.
   {
     "elapsed": "165.226612ms",
     "outcome": "success",
     "simulation": {
       "achievable_realtime_factor": 418.3225266680611,
       "eta": {
         "ms": 60000,
         "str": "60s"
       },
       "realtime_factor": -1.0,
       "step": 3001,
       "step_width": "20ms",
       "time": {
         "ms": 60000,
         "str": "60s"
       }
     },
     ...
     "uuid": "208dddb2-a429-4b22-80b9-cc6be68053b9"
   }

.. note::
   Whenever we include a ``...`` in the output, that means that we snip the
   output at that point.)

The output here we snipped twice, since it was so much. At this point though,
the simulation will happily go through the motions and continue simulating in
all eternity.

View the Web Server
-------------------

In the simulation endpoint, you might have noticed the line::

   II 18:07:25.783 [cloe/server] Listening at: http://127.0.0.1:8080

This is the address that Cloe's internal server is listening to.
Point your browser to that address (or Ctrl+click on it) in order to inspect
the API!

You should see a small page open up with the Cloe logo, a welcome text, some
important links, and the JSON API. The Cloe runtime exposes various internal
data via a JSON API. This makes it possible to provide other -- perhaps more
convenient or specialized -- interfaces to a simulation. You can have a look
at some of the links, and you can explore the JSON API right from your browser.

Exit the Simulation
-------------------

Whenever you are ready, you can press Ctrl+C to exit the simulation.
(There are also other ways to exit a simulation, we'll see more about those in
about shutting down the simulation and some statistics concerning the
simulation.

Congratulations! You installed Cloe, verified that it was working in its most
basic form, ran the simulation,
checked out the internal web server, and terminated Cloe.
Reward yourself with a snack!
