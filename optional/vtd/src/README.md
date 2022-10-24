VTD Binding
===========

## Communication Protocol

The basis for all communication with VTD falls on either the SCP message protocol, which is XML,
or the RDB protocol, which is binary.

### SCP
A single SCP channel is opened, which manages meta-simulation topics, as well as vehicle setup.
We also send vehicle label updates as SCP messages.

### RDB

There is a single global RDB channel for sending state and messages to VTD,
such as vehicle actuation and simulation time.

Then, each sensor has it's own RDB channel.
Currently, a "perfect sensor" is being used, which gives us all objects in the
world, including static and dynamic objects, as well as the ego vehicle itself.
