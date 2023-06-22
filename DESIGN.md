Redesign of Cloe
----------------

## Scheduler, Events and Callbacks

The main primitive of the scheduler is an *event*.
Each event can have:

  - `name : string`
  - `callbacks : callback[]`

Possible events:

  - (`Init`)
  - (`ReadFiles`)
  - `Configure` - merge configuration files, pass this to plugins
  - `Setup` - initialize plugins
  - `Connect`
  - `Start`
  - `PreCycle`
  - `Cycle`
  - `PostCycle`
  - `Stop`
  - `Disconnect`
  - `Reset`
  - `Abort`
  - `Pause`
  - `Wait`
  - `Resume`
  - `Sleep?` - for achieving a certain realtime factor
  - (`WriteFiles`)
  - (`Exit`)

Every event is triggered somewhere from with zero or more *callbacks*.
Each callback should have the following metadata:

  - `type : "lua"|"cpp"`
  - `source : file ":" line`
  - `name? : string`

Open Questions:

  - How do we deal with interrupts, especially interrupts that occur before the top
    interrupt has been processed.

    a) Queue the interrupt (zero or more)
    b) Discard the interrupt (zero or one)

## Plugins

Simplify plugin structure to not differentiate between simulator, controller, and component.
This artifical separation precludes plugins that do not cleanly fall into these pre-defined
categories.

Really, it's all about data flow, so a simulink kind of model would probably be very valuable.
There are inputs and outputs. Each part of a simulation exposes both, each port has a type=

Remove built-in nop simulators and controllers.
These are not required to run a simulation, and provide almost no value.
Better to use minimator than nop simulator, and better basic than nop controller.

Package some cloe plugins together.
This simplifies package management and speeds up builds.

Plugins don't have fixed implementation points, like `process` or `connect`,
but more general ones that are simply callbacks to events.

For example, the `vtd` plugin has the following methods:

  - `connect`
  - `configure`
  - `disconnect`
  - `step`
  - `abort`
  - `start`
  - `stop`
  - `reset`

And these could all be registered in a `setup` or `init` call.

And how would vehicle make these available? By registering them with the event handler.

Advantages:

  - Plugin can be implemented in any language, e.g. Lua.

## Connections

Connections between plugins are managed by Cloe.

Each plugin defines:

  - inputs : InputSpec[]
  - outputs : OutputSpec
  - components : ComponentSpec
  - configuration : object
  - state : object, current plugin state
  - setup : function

By making these explicit, we enable Cloe to run plugins in parallel safely.
We can also easily create these connections from simple JSON or XML configuration
files, and we can export them in a graphical format, e.g. via GraphViz.

Each input / output connection is described by a structure:

  - name : unique identifier within plugin namespace
  - description : string
  - type : input or output
  - function : (pointer to) getter / setter
  - signature : description of function signature
  - version? : API version for this function

Open Questions:

  - How do we deal with the fact that inputs and outputs may only be known after
    initialization (`Connect`) of plugin?

  - How do we safely prevent plugins running in parallel from using Lua callbacks
    concurrently?

## Simulation Output

- *input configuration*, which comes from Lua, JSON, CLI, and environment

  This should allow an exact replication of the simulation.

- *event injection*, for example from the web UI

  If we can avoid baking *event injection* into the engine directly, that would be great.
  This will require some more analysis, whether this can be realized through alternate
  mechanisms.

- *simulation execution*, which state followed after which when, did the callback fire, etc.

  There is a simple stack state that tracks state transitions and the callbacks that are fired
  from within each state. This can be streamed to a compressed output file, which prevents
  it from becoming too large a file to process. Back of the napkin estimate is ~5MB of
  uncompressed data per simulation minute. The format could be improved for an order of magnitude
  improvement in memory requirement.

  Each callback should be measured and the metadata of the callback should additionally include:

    - time : execution duration

- *simulation results*, success or failure

- *simulation metadata*, profiling data, how many kilometers driven, etc.

  Example data:

    - system characteristics
    - engine version
    - input configuration characteristics
      - input size
      - input format
    - feature usage
      - web server
      - debugger
      - lua configuration
      - json input
      - use of tests
    - simulation metadata
      - simulation duration
      - driven kilometers
      - number of plugins in use
      - number of connections between plugins
      - number of callbacks
      - registered events
    - performance data
      - realtime factor
      - cpu usage
      - memory usage
      - size of output

- *test results*, for each test run, id paired with results, measured data
