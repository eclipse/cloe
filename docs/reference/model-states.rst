Model States
============

The following image shows when the various methods of a Cloe simulation model
(like a simulator or controller binding) are called. This is very similar to
the states of the :doc:`system <system-states>` itself.

.. graphviz:: flowcharts/model.dot
   :alt: Cloe states and their transitions

The following methods are called in the nominal course of a simulation:

Model()
   The constructor of a model is called once.

   After construction, it should be possible to call the following methods:
   - ``resolution()``
   - ``is_connected()``
   - ``is_operational()``
   - ``enroll(Registrar&)``
   None of these methods should necessarily require a connection to some
   backend.

connect()
   A connection should be established to whatever backend.

   This can potentially take a while; if I/O is involved it is highly
   recommended to implement appropriate abort behavior. A connection is only
   established in order to run a simulation.

   After a connection is established, ``is_connected()`` should return true.

enroll(Registrar&)
   All trigger events and actions, as well as any HTTP handlers should be
   registered with the registrar.

   This should ideally be callable both when the model is connected as well as
   when it is not connected, in order to support offline stack file checking.

start(const Sync&)
   Called once at the beginning of each simulation.

process(const Sync&)
   Called for each step of a simulation.

stop(const Sync&)
   Called once at the end of each simulation, regardless of whether the
   simulation is a success or not.

disconnect()
   The connection to any backend should be terminated, any open files should be
   closed.

~Model()
   The destructor of a model is called once.

Additional methods that can be called in a simulation are:

pause(const Sync&)
   Called once every time a simulation transitions from a running state into
   a paused state.

   The synchronization information is the same as will be used in the next
   process step.

resume(const Sync&)
   Called once every time a simulation transitions from a paused state back
   into a running state.

   The synchronization information is the same as was used in the pause call
   and will also be used in the next process step.

reset()
   Called after a simulation has been stopped to make the model valid for
   another start.

   Not every model needs to support this, however supporting it will enhance
   the simulations which need many iterations but do not want to renegotiate
   a connection every time. Machine learning is a typical use-case for this.

abort()
   This method is called asynchronously from another thread and should cause
   some method that is currently executing to abort (and throw ``AsyncAbort``).
   Not every method need take this into consideration; however any method that
   contains any kind of I/O should be abortable.
