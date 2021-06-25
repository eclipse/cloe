Running a Test
==============

.. highlight:: console

In the previous section, :doc:`first-steps`, we saw how to create a stack file
with basic parameters to get a simulation running. This doesn't quite solve the
more complex problem of getting multiple complex systems -- like VTD and
a complex ADAS using ROS -- working in tandem.

Suppose you are a tester and want to perform a test with the help of Cloe.
As a tester, you are not really interested in debugging code. The focus is
rather on efficiently performing simulations of various test cases.
For this usage scenario, a single source-of-information for a test and
reliably reproducing the simulation of the test-case are of greatest import.

Cloe simulations can be deployed as sets of containers which relieves testers
from the tedious task of retrieving the source code as well as setting up and
using one or more build environments for the different parts of the simulation.

An orchestrator like Docker Compose or Kubernetes can be used to run a
simulation in containers on the local machine or in a container cluster, on
premises or in the public cloud.

Since the configuration of such an orchestrator can be complex it is advisable
to create a layer of software to hide this complexity from the tester. This
layer would use a subset of the orchestrator's features suitable to configure
all relevant aspects of the simulation containers (command-line parameters,
file input/output, ...).

Such a tool might be published in this repository at a later point in time
along with its documentation.

.. todo:: Consider to publish an example docker-compose.yaml suitable to run a
   single example test-case.
