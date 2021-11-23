Controller
==========

The controller component represents the HMI of the actual controller used
in the simulation.

It generates an interface consisting of buttons, graphs and other
sub-components, based on a JSON configuration, which is delivered via Cloe's
JSON-Api.

Props
-----
The props of the controller component are:

- Cloe host (``host: string``)
- Connection state (``connected: boolean``)
- Simulation time (``simTime: number``)
- Controller Api (``controller: { id: number, endpointBase: string }``

Fetch the UI Configuration
--------------------------

The ``render`` method of the controller component checks whether the
JSON configuration is loaded. This is indicated by the boolean value
of ``uiSpecificationLoaded``. In case it's not loaded, ``render``
continues with loading the configuration. This is done with
``fetchUIConfiguration``.

This method fetches the UI configuration from the Cloe JSON-Api.
The endpoint used for this consists out of ``props.host`` +
``props.controller.endpointBase`` + ``"/ui"``.
The fetched UI configuration consists out of four parts:

- The controllers name
- The definitions of the HMI elements
- The layout in which the elements will be arranged
- The Api endpoint paths from which Cloe UI will fetch all relevant data

These Information will be stored in ``this.name``, ``this.elements``,
``this.layout`` and ``this.paths``.

Update Data
-----------

In case the specification is loaded already, ``render`` will continues
with ``updateData``. This method fetches all JSON endpoints which are
defined in ``this.sources``. This object includes all data endpoints of all
specified HMI elements and was filled during the loading process of the
elements.
The fetched data will also be stored in ``this.sources``.

Generate And Render JSX For HMI
-------------------------------

After the data was updated, ``render`` will continue with
``generateJSXComponent``. This method creates valid JSX Code for each
element stored in ``this.elements``, out of the elements specification,
the data belonging to the element and if need be ``props.simTime``.

Finally, ``renderController`` creates the complete JSX for the controller,
out of ``this.layout`` and the element-JSXs.
This JSX is returned by ``render``.
