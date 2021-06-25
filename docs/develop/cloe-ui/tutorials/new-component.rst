Add A New Component To Cloe UI
==============================

Cloe UI's top level component is called ``App``. It's subcomponents are:

- The Navigation Bar
- The Sidebar
- Main Components (Controller, Rendering...)

This tutorial will give an example on how to add a new Main Component
to Cloe UI. Let's say you want to add a component which displays the
current simulation time.

Preparation
-----------

You have to define the new component in a new file
``src/components/simTime.jsx``.

.. code-block:: jsx
   :linenos:

   import React from "react";

   function SimTime(props) {
       const { currentSimTime } = props;
       return <h1>{currentSimTime}</h1>;
   }
   export default SimTime;

Implementation Into Cloe UI
---------------------------

Now you have to add ``SimTime`` as subcomponent to the ``App`` component
of Cloe UI. ``App`` is defined in ``src/App.js``.

The first step is to import the new component. All needed components are
imported at the top of the file, you just add our new component like this:

.. code-block:: javascript

   import SimTime from "./components/simTime"

``App`` distributes all components on two columns. In order to do that,
its ``render()`` method creates an array which includes all components plus
an id. Add our component to it and provide the simulation time with
the prop ``currentSimTime``.

.. code-block:: javascript
   :linenos:

   const components = [
     { id: 0, component: <Rendering sensors={sensors} /> },
     { id: 1, component: <Simulation simulation={simulation} host={host} /> },
     { id: 2, component: <Binding binding={binding} /> },
     { id: 3, component: <EventListWrapper events={events} /> },
     { id: 4, component: <SimTime currentSimTime={this.state.simTime} /> }
   ];

The last step is to assign our component to an ``AppColumn``. This
will indicate on which of the two columns it will appear by default.

This assignment is done in the ``constructor`` method of ``app``. To
assign our component, you add it's id (4 in this case) to either
``this.componentsOnLeftSide`` or ``this.componentsOnRightSide``:

.. code-block:: javascript
   :linenos:

   this.componentsOnLeftSide = cookies.get("firstColumn") || [0, 4];
   this.componentsOnRightSide = cookies.get("secondColumn") || [1, 2, 3];

So in this example you will render it on the left side of the app, right under
the controller hmi component with the id ``0``.

Now you are finished and can reload Cloe UI to see the result.

.. note::
   Since the old ``componentsOnLeftSide`` and ``componentsOnRightSide``
   are stored in cookies which will overwrite the new assignment, make sure to
   delete the cookies before reloading Cloe UI.
