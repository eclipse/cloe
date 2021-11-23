VTD Simulator Binding
=====================

The VTD simulator binding is used to enable Cloe to communicate with VTD. This
involves retrieving sensor-, vehicle- and groundtruth data from VTD and
converting it to the respective Cloe data structures. Additionally Cloe's
actuation requests are converted into VTD's format and VTD is requested to
proceed simulation for an instance of time.

Initially VTD is configured and a scenario is selected and started.
All this is done using information from the configuration.

.. _configuration:

Configuration
-------------

This simulator binding can be configured in a Cloe Stackfile. There are both
general and VTD specific configuration options. The general options are
described in the main config reference because they are common for any
simulator binding.

In the following sections you'll find an example and reference for VTD
simulator binding specific configuration options.

setup
    The VTD setup to use. It contains VTD configuration that influences how
    VTD is started and which parts of VTD (GUI, image generator(s), ...) are
    used. We mainly use it to distinguish between interactive and headless
    operation. The following setups are maintained for Cloe operation:

    - Cloe.Default for interactive operation with GUI and render window
    - Cloe.noGUInoIG for headless operation (e.g. automated tests)
    - Cloe.noGUI to open only the render window

scenario
    The path to the scenario relative to the VTD project's scenario directory.
    So any scenario directly located in that directory can just be defined by
    their name as in the example below.

dat_file
    VTD can optionally write a file recording all the available ground truth
    data in a VTD specific format. Specify a file path here to enable the
    output. Usually VTD will be run inside a container. In order to prevent the
    file to be deleted together with the container after simulation, you will
    have to define a storage entry in order to map the output directory to a
    sensible storage. Again, compare the example below.

vehicles
    VTD specific vehicle configuration for each vehicle.

vehicles/<name>
    Configuration for a particular VTD vehicle. The vehicle's name is as
    defined in the selected scenario. It is usually called Ego but it can be
    identified by the vehicle's "Animation" field set to "external" in the
    Scenario Editor. Note that in a multi agent simulation scenario there are
    multiple "external" vehicles with different names.

vehicles/<name>/sensors
    Definition of a vehicle's set of VTD sensors. There's currently two
    subsections *sensors* and *components* first of which describes the sensor
    configuration. Each sensor might offer data of different types like
    objects, lanes or road signs. Which are referred to as components in Cloe.

vehicles/<name>/sensors/<sensorname>
    Definition of a single VTD sensor. In future we may support different
    formats to specify the sensor properties in order to perform simple things
    in a simple way. Currently the only way is to define it in terms of the
    VTD module manager XML configuration as described in the following section.

vehicles/<name>/sensors/<sensorname>/xml
    VTD module manager XML configuration for a single VTD sensor. You can for
    example specify the mounting position and orientation, the sensor's field
    and range of view (a.k.a. frustum) and filters defining which types of
    objects are to be perceived by the sensor. Cloe needs you to fill in
    placeholders for values that are known at runtime but not at the time of
    your configuration.

    - `[[ sensor_id ]]` Sensor id to create a unique sensor name in the *Sensor* tag
    - `[[ sensor_name ]]` Sensor name e.g. for a speaking sensor name in *Sensor* tag
    - `[[ sensor_port ]]` TCP port for the sensor's RDB channel in the *Port* tag
    - `[[ player_id ]]` Player id for the *Player* tag

    Guess your configuration options from the example or refer to the
    `Vires VTD Wiki <https://redmine.vires.com>`__.

vehicles/<name>/components
    The component configuration maps the sensors' data streams to Cloe
    components and makes them available under a certain name.

vehicles/<name>/components/<component_name>
    The component name is used to get access to the component through the
    get functions of a Cloe Vehicle object. Each Cloe controller binding knows
    the vehicle it is associated with (or mounted in). The implementor of the
    binding can access sensor data by this component name and is likely also
    the author of this part of the Cloe Stackfile.

vehicles/<name>/components/<component_name>/from
    Refers to the <sensorname> as defined in sensors section. This defines the
    VTD sensor to take a particular type of data from. Note that all types of
    data in this sensor are expressed relatively to this sensor's mounting
    position and filtered according to the defined field and range of view.


vehicles/<name>/components/<component_name>/type
    Defines the Cloe component type. Currently available are

    - lane_sensor
    - object_sensor

    Note that this type of data needs to be enabled in the VTD sensor's
    configuration. E.g. the roadMarks filter for the lane_sensor component.
    There is no warning whatsoever in case this is missed, but the respective
    data will be unavailable in the controller binding implementation.

vehicles/<name>/components/<component_name>/override
    Declares whether the intention is to override an existing component with
    that name. Override defaults to false. If its value is false, then
    declaring a component name already in use will lead to an error message. If
    true, cloe expects the component name to be already in use and override it
    with the newly declared one or terminate with an error if no such component
    can be found.


Ground Truth
------------

Ground truth, that is complete and unaltered data from the VTD environment
model, is made available by VTD over the so called TaskControl RDB channel.
Data passed on this channel is agnostic of any sensor. Objects are
given in absolute coordinates (relative to VTD's map origin).

We make parts of the ground truth data on the following Cloe component:

- cloe::gndtruth_world_sensor => Groundtruth objects

For the closest thing to ground truth for lanes and traffic signs, refer to
:ref:`perfect_sensor`. Complete ground truth data for these types of data can
be found in the opendrive map used by the current scenario.


.. _perfect_sensor:

Perfect Sensor
--------------

In order to provide the user with a reasaonable default, the VTD plugin always
configures a *Perfect Sensor*. It is an idealized sensor as its parameters
suggest:

- Positioned in the vehicle's origin (ground under center of rear axle)
- Zero rotation (so exact same coordinate frame as vehicle)
- Surround view
- Range 0m - 180m
- Up to 50 objects

The different types of sensor data which in Cloe terms are called 'components'
are made available with the following component names:

- cloe::default_world_sensor => dynamic objects
- cloe::default_lane_sensor => lane boundary segments
- cloe::default_ego_sensor => ego pose/odometry

The above is sensible only for a single agent simulation. In case of a multi
agent scenario you should define your sensors and map your sensor components
explicitly as described in :ref:`configuration`: section.


Example
-------

.. code-block:: yaml

  simulators:
    - binding: vtd
      args:
        setup: Cloe.noGUInoIG
        project: example_vtd_project
        scenario: acc.xml
        dat_file: /output/vtd-basic-example.dat
        vehicles:
          Ego:
            components:
              "cloe::default_lane_sensor":
                from: camera
                type: lane_sensor
                override: true
              "cloe::default_world_sensor":
                from: camera
                type: object_sensor
                override: true
              "sysut::radar1":
                from: radar_1
                type: object_sensor
              "sysut::radar2":
                from: radar_2
                type: object_sensor

            sensors:
              radar_1:
                ...
              radar_5:
                ...
              camera:
                xml: >
                  <Sensor name='Sensor_[[ sensor_id ]]_[[ sensor_name ]]' type='video'>
                    <Load
                      lib='libModulePerfectSensor.so'
                      path=''
                      persistent='true'
                      />
                    <Frustum
                      near='0.0'
                      far='180.0'
                      left='180.0'
                      right='180.0'
                      bottom='180.0'
                      top='180.0'
                      />
                    <Origin type='sensor' />
                    <Cull maxObjects='50' enable='true' />
                    <Port name='RDBout' number='[[ sensor_port ]]' type='TCP' sendEgo='true' />
                    <Player id="[[ player_id ]]" />
                    <Position dx='0.0' dy='0.0' dz='0.0' dhDeg='0.0' dpDeg='0.0' drDeg='0.0' />
                    <Database resolveRepeatedObjects='true' continuousObjectTesselation='2.0' />
                    <Filter objectType='pedestrian'/>
                    <Filter objectType='vehicle'/>
                    <Filter objectType='obstacle'/>
                    <Filter
                      objectType="roadMarks"
                      roadmarkPreviewDistance="100.0"
                      tesselate="true"
                      tesselateNoPoints="10"
                      tesselateFixedStep="true"
                      />
                  </Sensor>
        stack:
          image: dtr.cucumber.tld/cloe/vtd-cloe:2.2.0
          storage:
            # define a storage volume from a path on your host
            - name: example_vtd_project
              host_path: ~/.vtd_projects/example_vtd_project
            - name: output
              host_path: .
  ...

Usage
-----

The following help can be viewed with ``cloe-engine usage vtd``:

.. runcmd:: cloe-launch exec -P ../conanfile.py -o:o with_vtd=True -- usage vtd
   :replace: "Path:.*\\//Path: <SOME_PATH>\\/"
   :syntax: yaml

JSON Schema
-----------

.. runcmd:: cloe-launch exec -P ../conanfile.py -o:o with_vtd=True -- usage --json vtd
   :replace: "\"\\$id\":.*\\//\"\\$id\": \"<SOME_PATH>\\/"
   :syntax: json
