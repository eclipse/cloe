Runtime Watchdog
================

.. highlight:: yaml

.. note:: This feature is available since Cloe runtime version 0.16.

The Cloe runtime has the ability to activate a watchdog that will act when
a simulation state exceeds a configured timeout. It can be configured in the
stack file in the ``engine`` section and has the following defaults::

   engine:
     watchdog:
       mode: off
       default_timeout: 90000
       state_timeouts:
         CONNECT: 300000
         ABORT: 90000
         STOP: 300000
         DISCONNECT: 600000


``/engine/watchdog/mode``
-------------------------

The following modes are available:

``off``
   The watchdog is disabled (the default).

``log``
   When a timeout occurs, the watchdog logs a *critical* message, but does
   nothing else::

      Watchdog timeout of 90000 ms exceeded for state: X

   This can be useful if the log messages are continuously monitored, as the
   orchestrator may be better suited to provide customizable reactions.

``abort``
   When a timeout occurs, the watchdog logs a message and then pushes an ABORT
   interrupt. This will result in an orderly shutdown but will not work if the
   state that caused the timeout never returns.

``kill``
   When a timeout occurs, the program is killed. None of the plugins will be
   given the opportunity to clean up, so this may result in output files that
   are only partially written or processes that are still running in the
   background.


``/engine/watchdog/default_timeout``
------------------------------------

The default timeout is used for each state unless a state-specific timeout is
set in ``/engine/watchdog/state_timeouts``. This value is specified in
milliseconds, with zero indicating no timeout.

.. note:: The default timeout should generally be at least as long as the
   polling interval (set in ``/engine/polling_interval``), otherwise the watchdog
   will trigger during normal operation.

``/engine/watchdog/state_timeouts``
-----------------------------------

Not all states need the same time, in particular, the CONNECT and DISCONNECT
states may require I/O operations that can take an order of magnitude more time
than other states in the simulation.

Each state can therefore be given a state-specific timeout. This can be either
``null`` to use the default timeout, or a number of milliseconds. The following
case-sensitive states are available:

- CONNECT
- START
- STEP_BEGIN
- STEP_SIMULATORS
- STEP_CONTROLLERS
- STEP_END
- PAUSE
- RESUME
- SUCCESS
- FAIL
- ABORT
- STOP
- RESET
- KEEP_ALIVE
- DISCONNECT

See :doc:`system-states` for more information on the simulation states.
