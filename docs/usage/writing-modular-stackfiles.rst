Creating Modular Stackfiles
===========================

In the previous two sections, we saw how to create a stackfile and how to get
information from plugins so we can configure them in the stackfiles. For every
simulator and controller we use in a simulation, we might have hundreds or
thousands of scenarios we want to run. In order to reduce repetition, and the
errors that naturally come with repetition, we should write modular stackfiles.
Cloe supports merging multiple stackfiles into one contiguous simulation
description. This section will show you techniques and strategies you can use
to get this done.

A simple example is provided in ``tests/test_engine_smoketest.json``:

.. literalinclude:: ../../tests/test_engine_smoketest.json
    :language: json
