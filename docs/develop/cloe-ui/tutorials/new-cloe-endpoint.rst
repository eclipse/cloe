Fetch An Additional Cloe API Endpoint
=====================================

All data which is fetched from the Cloe API is stored within the
``this.state.cloeData`` object of the ``App`` component.

Each fetched endpoint will get an object within ``this.state.cloeData``
which is named after the endpoint, eg:

.. code-block:: json

    {
        "cloeData": {
            "/binding":
            {
                "can_step": true,
                "is_connected": true
            },
            "/simulation":
            {
                "sim_time": 100,
                "step": 100
            }
        }
    }

To fetch an additional endpoint, just add it to the
``this.thirdPhaseEndpoints`` array of ``App``, which is defined in the
``constructor`` of the component:

.. code-block:: js

    this.thirdPhaseEndpoints = [
      "/simulation",
      "/triggers/queue",
      "/triggers/history",
      "/uuid",
      "/version"
    ];

After that, you can use the fetched data in your components:

.. code-block:: js

    <Simulation simulation={cloeData["/simulation"]} />
