CDR 4 - Triggers
================

A closed loop simulation operates with one or more participants in a scenario.
The purpose of a simulation is to investigate the effects of events on behavior
models and functions. In order to steer a simulation in the desired use-cases,
it is often necessary to introduce and react to events in the simulation.
We call such an event or reaction a *trigger*.

.. note::

   This section concerns itself with the design rationale of triggers in Cloe.
   If you want to know how to use triggers with Cloe, see the
   :doc:`trigger reference <../reference/triggers>`.

The design of the trigger mechanism in Cloe defines how models can *register*
events that can occur as well as actions that can be taken in response to an
event. These two parts can be viewed more generically as *read-only data* and
*functions with side-effects*. In it's most primitive form, a trigger has the
form::

   if (CONDITION) {
      do ACTION(s)
   }

In every cycle of the simulation, the condition is checked and if it applies,
then the action is executed and the trigger removed. For certain event types,
such as simulation time, it is possible to structure the triggers such that the
runtime cost of checking the conditions is minimized [1]_. Other triggers may
be so complex that they must be treated as black-box *hooks*.

The term *trigger* shall be understood as some interaction with the simulation
that takes the specific two-part form of (event, action). The term *hook* shall
be understood as a trigger with an event that always is triggered and does not
result in the trigger being removed from the set of available triggers.

Questions that should be considered in the design process are:

- How do models register readily available read-only data?
  For example,

  * internal state (state machine state, feature booleans, activation state, etc.)
  * vehicle attributes (speed, steering torque, etc.)
  * simulation state (cycle number, execution speed, abortion, etc.)
  * user intervention (button click, etc.)

- How do models register computationally expensive data?
  For example,

  * time-to-collision
  * collision detection

  Such data should ideally not be calculated more than once per cycle.

- How do models register available actions?
  For example,

  * simulation parameters (speed, pause, abort, etc.)
  * controller interface (enable, increase speed, resume, etc.)
  * vehicle attributes (open door, set blinker, brake pressed, etc.)
  * driver actions (steering wheel override; similar to vehicle)
  * system (run command, log message, etc.)
  * traffic (non-ego vehicle cut-in, braking to full-stop, etc.)
  * assertions

- How can a trigger depend on more than one data item?
  For example,

  * ``if AEB active and abs(steering torque) > 20 nm``

- Can triggers be processed in parallel?

- How do triggers indicate that they should not be removed even if they
  are applied?

Requirements
------------

The following requirements have been collected through an informal process
of information gathering and brainstorming.

1. Triggers must be reconstructible.

   The entirety of a simulation must be reproducible.
   Every interaction with the simulation, be it a trigger or
   a user-interaction, must be reconstructible by design.
   This leads to the following derived requirements:

   a) Triggers must be traceable.

      It should be clear why a trigger is active and why the trigger condition
      is fulfilled.

   b) Triggers must be reproducible.

      The same input conditions to a simulation should result in the same
      output, including the set of triggers.

   c) Triggers include all dynamic interaction.

      Every dynamic interaction that affects the simulation must take the form
      of a trigger.

   d) Triggers must be serializable and deserializable into a portable format.

      The set of triggers active in a simulation should be exportable to a
      portable format in a file or string. Any blob of data conforming to the
      portable format should importable into a running simulation or inserted
      at the start of a simulation.

2. Triggers should be able to reflect temporal and logical constraints.

   This is necessary for the description of complex interactions and
   relationships. For example, the following logical constraints may need to be
   modelled:

   *A => B*
      Trigger *B* may only be activated if trigger *A* is executed.

   *Ca xor Cb*
      Trigger condition is fulfilled if only one of sub-condition *Ca* or
      *Cb* is fulfilled, but not both.

   Temporal constraints are a subclass of logical constraints -- those that
   pertain to a trigger in the dimension of simulated time. One very artificial
   example could be: "Fail if *A* triggers and the simulated time is less than
   10 seconds."

3. Triggers must be human-readable.

   A canonical textual representation of a sequence of triggers should be
   available. A human should be able to read a log of executed triggers and
   understand precisely what happened.

4. Triggers must be parametrizable by general-purpose programming languages.

   A common use-case is to vary parameters of a base simulation scenario in
   order to discover critical scenario instances. The parameterization of a
   set of triggers should be possible without an in-depth understanding of the
   implementation of triggers.

   This is effectively given by requirement 3.

5. Triggers should be performant.

   Trigger conditions are likely to be evaluated every cycle of a simulation,
   some of them possibly more than once. The introduction of triggers in a
   simulation should in general not slow the simulation by more than a small
   constant factor.

6. Triggers should be user-friendly.

   Barring the introduction of a graphic user-interface to allow testers to
   create comprehensive trigger sequences and scenarios, the programming of
   them should be as easy as possible.

   An implementation of this requirement may provide multiple ways of creating
   triggers in order to support varying degrees of complexity:
   Common use-cases should be simple, and rare use-cases should be possible.

Proposal
--------

.. highlight:: json

The following design of triggers attempts to fulfill the stated requirements in
a sustainable way:

#. Each trigger can be described by the following attributes:

   label
      Descriptive (string).
      A human-readable string describing what the trigger does.

   event
      Descriptive (object).
      A key uniquely identifying a condition that can be evaluated by the
      simulation as either true or false. The condition may have parameters.

   action(s)
      Descriptive (list of objects).
      A key uniquely identifying an action that can be executed by the
      simulation. The action may have parameters.

   persistent
      Descriptive (boolean).
      A boolean value that indicates whether the trigger should be removed
      from the list of active triggers after the condition has been fulfilled
      and the action executed.

   source
      Attribute (string).
      The source of a specific trigger instance. Possibilities are:

      - absolute path
      - trigger name
      - IP address
      - model (i.e., controller or simulator)

   insertion time / since
      Attribute (number).
      The simulation time a specific trigger instance is registered with the
      simulation. This is the point in the simulation *since* which the trigger
      is active.

#. Each trigger is described in JSON with the canonical form::

      {
         "label": "print message and exit",
         "since": 0,
         "source": "/path/to/file",
         "event": {
            "name": "time",
            "time": 1.0
         },
         "actions": [
            {
               "name": "cmd",
               "cmd": "echo 'Hello World!'"
            },
            {
               "name": "exit",
               "code": 0
            }
         ],
         "persistent": false
      }

   If only one action is configured, the single object may replace the list
   construct.

   Only the fields ``label``, ``event``, and ``actions`` can be specified by
   a user.

#. A trigger may be expressed in JSON with a short form::

      {
         "event": "time=1.0",
         "action": "cmd=echo 'Hello World!'"
      }

   The value of the ``event`` or ``action`` key may be a string which contains
   an identifier optionally followed by an ``=`` and some text (that is, it
   approximately matches the regular expression ``^[a-zA-Z0-9_]+(=.+)?$``)::

      "key=value"

   This is interpreted in the same manner as the following object would be::

      {
         "name": "key",
         "key": "value"
      }

   For the action key, a list of strings is interpreted analogously.
   The short form is loosely modeled after the approach taken by `Ansible`_.
   This syntax may be expanded in the future, but should remain
   backwards-compatible.

#. A trigger action may register a new trigger.

   This allows triggers that consist of separate event-action pairs to be
   treated as a single trigger, from the perspective of the user.

   For example: A common use-case in simulations is for a tester, representing
   the driver of a vehicle, to push some buttons in some sequences. A button
   press consists of pushing down at a certain time, and letting go a while
   later. This could be represented by two triggers::

      [
         {
            "event": "time=15.0",
            "action": {
               "name": "basic/hmi",
               "plus": true
            }
         },
         {
            "event": "time=15.5",
            "action": {
               "name": "basic/hmi",
               "plus": false
            }
         }
      ]

   However, this is error-prone, due to most people thinking of pushing
   a button as a single action. The tester is likely to fully forget to
   "un-push" the button or set the time wrong (such as identical to the start
   time). A single trigger that could insert one or two triggers for a later
   point could allow the user to only need to program the following::

      {
         "event": "time=15.0",
         "action": {
            "name": "button",
            "button": "basic/hmi/plus"
         }
      }

   A short-form may be made available to make it even easier::

      {
         "event": "time=15.0",
         "action": "button=basic/hmi/plus"
      }

#. The trigger manager can be configured to disallow triggers with a particular
   source.

   After recording a simulation with a set of triggers, it is clear which
   triggers were originally programmed, and which originated from a trigger
   action or a user.

   Generated triggers *must* be ignored, else there would be duplicate
   triggers, since the same trigger would be generated again.

   Disallowing user interaction can be used to guarantee the fidelity of a test
   without needing to inspect the trigger log after the simulation.

#. A trigger may be run asynchronously on the cache if they accept a latency
   of one cycle.

   This lessens the performance effect complex triggers can have on
   a simulation by allowing the trigger condition to run in parallel with
   the simulation.

#. User interaction is realized through the dynamic insertion of triggers.

   Buttons in the Cloe UI contain a JSON data blob which is then posted to
   a single endpoint, say ``/api/input``::

      {
         "event": "next",
         "action": "button=basic/hmi/plus"
      }

   The special ``next`` event is evaluated in the next simulation cycle.

   This design has some very significant advantageous consequences:

   a) Requests from the UI are never blocked for more than a minuscule amount
      of time.
   b) The triggers that are dynamically inserted into the simulation are
      traceable and reproducible, and can be shown to the user in real time.
      (For example, a list of active and inactive triggers can be shown in
      the UI itself.)
   c) Multiple triggers in a single simulation cycle can be prevented, if
      necessary or useful.

#. Complex trigger sequences are realized through the use of `Wren`_.

   More on this later.

----

.. rubric:: Footnotes
.. [1]
   With a priority queue, the cost of checking whether any action should be
   executed has a cost of *O(1)*, because the triggers are sorted by activation
   time.

.. _Ansible: https://www.ansible.com
.. _Wren: http://wren.io
