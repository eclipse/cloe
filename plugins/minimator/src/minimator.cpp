/*
 * Copyright 2020 Robert Bosch GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * \file minimator.cpp
 *
 * This file defines the minimator example simulator plugin.
 *
 * In principle, creating a simulator binding for Cloe is fairly simple. You
 * must create a dynamic library that contains a plugin manifest that defines
 * a factory that creates objects that fulfill the `cloe::Simulator` interface.
 * (After introducing a class we will omit the `cloe::` scope specification for
 * brevity.)
 *
 * The `Simulator` interface is little more than a `cloe::Model` that provides
 * access to one or more `cloe::Vehicle`. As such it has a lot in common with
 * all other models, such as `cloe::Controller` and `cloe::Component`.
 *
 * We have then, the following situation:
 *
 *    1. `Minimator` is a `Simulator`, which is a `Model`.
 *       \see cloe/simulator.hpp
 *       \see cloe/model.hpp
 *
 *    2. `Minimator` provides `MinimatorVehicle`, which is a `Vehicle`.
 *       \see cloe/vehicle.hpp
 *
 *    3. A `MinimatorFactory` is exported with `EXPORT_CLOE_PLUGIN`.
 *       \see cloe/plugin.hpp
 *
 * A factory or simulator isn't much good to us if we can't configure it. We
 * use the `Fable` library for this purpose. This is included in the Cloe
 * runtime and is available in the `cloe` namespace as well as the original
 * `fable` namespace.
 */

#include <functional>  // for function<>
#include <memory>      // for unique_ptr<>
#include <string>      // for string
#include <vector>      // for vector<>

#include <cloe/component/ego_sensor.hpp>        // for NopEgoSensor
#include <cloe/component/lane_sensor.hpp>       // for LaneBoundarySensor
#include <cloe/component/latlong_actuator.hpp>  // for LatLongActuator
#include <cloe/component/object_sensor.hpp>     // for NopObjectSensor
#include <cloe/handler.hpp>                     // for ToJson
#include <cloe/models.hpp>                      // for CloeComponent
#include <cloe/plugin.hpp>                      // for EXPORT_CLOE_PLUGIN
#include <cloe/registrar.hpp>                   // for Registrar
#include <cloe/simulator.hpp>                   // for Simulator
#include <cloe/sync.hpp>                        // for Sync
#include <cloe/vehicle.hpp>                     // for Vehicle

namespace minimator {

/**
 * MinimatorConfiguration is what we use to configure `Minimator` from some
 * JSON input.
 *
 * The Cloe runtime takes care of reading the configuration from the stack
 * file and passing it to the `MinimatorFactory`, which can then pass it
 * to `Minimator` during construction.
 *
 * So, the input will be deserialized from `/simulators/N/args`, where `N` is
 * some entry in the `simulators` object:
 *
 *     {
 *       "version": "4",
 *       "simulators": [
 *         {
 *           "binding": "minimator"
 *           "args": {
 *             "vehicles": [
 *               "ego1",
 *               "ego2"
 *             ]
 *           }
 *         }
 *       ]
 *     }
 *
 * Since our minimalistic simulator doesn't do much yet, our configuration
 * is quite simple: a number of names which will each become a vehicle.
 */
struct MinimatorConfiguration : public cloe::Confable {
  std::vector<std::string> vehicles{"default"};

  // The `CONFABLE_SCHEMA` is simple enough and is the recommended way to
  // augment a class that inherits from `Confable` to expose `from_conf`,
  // `from_json`, and `to_json` methods. The `Schema` type is a sort of
  // polymorphic type that automatically derives a JSON schema from a set of
  // pointers. This schema is used to provide serialization and deserialization.
  //
  // \see fable/confable.hpp
  // \see fable/schema.hpp
  //
  CONFABLE_SCHEMA(MinimatorConfiguration) {
    // For us, each `Schema` describing a `Confable` will start with an
    // initializer list of pairs: this describes a JSON object. Each property in
    // this object may be another object or another primitive JSON type.
    // In this configuration, we want to deserialize into a vector of strings.
    //
    // `Schema` contains some magic to make it "easy" for you to use.
    // The following eventually boils down to:
    //
    //     fable::schema::Struct{
    //        {
    //          "vehicles",
    //          fable::schema::Vector<std::string, fable::schema::String>(
    //            &vehicles,
    //            "list of vehicle names to make available"
    //          )
    //        }
    //     }
    //
    // You can hopefully see why `Schema` contains the magic it contains.
    return cloe::Schema{
        {"vehicles", cloe::Schema(&vehicles, "list of vehicle names to make available")},
    };
  }
};

/**
 * MinimatorLaneSensor is a very static lane boundary sensor.
 *
 * It returns the 4 lane boundaries of a 3-lane 4m lane-width road of a total
 * length of 100m. The road is laterally centered at the origin.
 */
class MinimatorLaneSensor : public cloe::LaneBoundarySensor {
 public:
  MinimatorLaneSensor() : cloe::LaneBoundarySensor("minimator_lane_sensor") {
    const int n = 4;
    const double w = 4.0;
    const double l = 100.0;
    for (int i = 0; i != n; ++i) {
      cloe::LaneBoundary lb;
      lb.id = i;
      lb.prev_id = -1;
      lb.next_id = -1;
      lb.dx_start = 0;
      lb.dy_start = (n - 1) * w / 2.0 - w * i;
      lb.heading_start = 0.0;
      lb.curv_hor_start = 0.0;
      lb.curv_hor_change = 0.0;
      lb.dx_end = l;
      lb.type = i % (n - 1) ? cloe::LaneBoundary::Type::Dashed : cloe::LaneBoundary::Type::Solid;
      lb.color = cloe::LaneBoundary::Color::White;
      lb.points.push_back(Eigen::Vector3d(lb.dx_start, lb.dy_start, 0));
      lb.points.push_back(Eigen::Vector3d(lb.dx_end, lb.dy_start, 0));
      lane_boundaries_[i] = lb;
    }
    mount_pose_.setIdentity();
  }
  virtual ~MinimatorLaneSensor() = default;

  /**
   * Return the static set of lane boundaries.
   */
  const cloe::LaneBoundaries& sensed_lane_boundaries() const override { return lane_boundaries_; }

  /**
   * Return the frustum of the lane sensor.
   */
  const cloe::Frustum& frustum() const override { return frustum_; }

  /**
   * Return the mounting position of the lane sensor.
   */
  const Eigen::Isometry3d& mount_pose() const override { return mount_pose_; }

 private:
  cloe::LaneBoundaries lane_boundaries_;
  cloe::Frustum frustum_;
  Eigen::Isometry3d mount_pose_ = Eigen::Isometry3d::Identity();
};

/**
 * `MinimatorVehicle` is the implementation of a vehicle that comes from
 * a `Minimator` simulator.
 *
 * In a Cloe simulation, a `Vehicle` provides the platform on which
 * communication between simulators and controllers occurs. This communication
 * happens through components that are contained in the vehicle. These
 * components are filled and read by the simulator that provides the vehicle.
 * The controller doesn't need to know what exact type each component is or how
 * it gets its data, as long as the components fulfill one of the common
 * component interfaces. This leaves the simulator plugin the job of dealing
 * with concrete types. In fact, the simulator must normally provide each
 * component implementation.
 *
 * During initialization, `Minimator` will create a vehicle for each name that
 * is provided in the configuration.
 *
 * \see cloe/vehicle.hpp
 */
class MinimatorVehicle : public cloe::Vehicle {
 public:
  /**
   * Construct a `MinimatorVehicle`.
   *
   * \arg id    unique ID within simulator's set of vehicles
   * \arg name  unique name within simulator's set of vehicles
   *
   * ## Components
   *
   * There are a great number of components that can be added to a vehicle.
   * (It is also possible for a vehicle to have no components -- we could be
   * dealing with a vehicle from the 1970s for all we know -- but such
   * a vehicle wouldn't be very interesting for us.) In our minimalistic
   * simulator, we provide three components.
   *
   *   - Ego sensor. This provides information on the vehicle itself, such as
   *     how fast the vehicle is traveling or where it is in the world.
   *   - Object sensor. This provides information on the world outside of the
   *     vehicle, such as would come from a video camera or radar.
   *   - Actuator. This lets of send actuation commands to the vehicle, such
   *     as an acceleration or a steering angle.
   *
   * We would normally create our own implementation of these sensors, based
   * on our feature set, but since this is a super-minimalistic simulator,
   * we'll use dummy sensors.
   *
   * \see cloe/component.hpp
   * \see cloe/component/ego_sensor.hpp
   * \see cloe/component/object_sensor.hpp
   * \see cloe/component/latlong_actuator.hpp
   */
  MinimatorVehicle(uint64_t id, const std::string& name) : Vehicle(id, name) {
    // Create a new `EgoSensor` and store it in the vehicle, making it available
    // by the standard names as defined by the enum values `DEFAULT_EGO_SENSOR`
    // and `GROUNDTRUTH_EGO_SENSOR`.
    //
    // The `new_component` method will put this new object in a shared pointer.
    // If you want more control, use `set_component` or `add_component`.
    //
    // \see Vehicle::new_component
    this->new_component(new cloe::NopEgoSensor(),
                        cloe::CloeComponent::GROUNDTRUTH_EGO_SENSOR,
                        cloe::CloeComponent::DEFAULT_EGO_SENSOR);

    // Similarly here.
    this->new_component(new cloe::NopObjectSensor(),
                        cloe::CloeComponent::GROUNDTRUTH_WORLD_SENSOR,
                        cloe::CloeComponent::DEFAULT_WORLD_SENSOR);

    this->new_component(new MinimatorLaneSensor(),
                        cloe::CloeComponent::GROUNDTRUTH_LANE_SENSOR,
                        cloe::CloeComponent::DEFAULT_LANE_SENSOR);

    // The `LatLongActuator` component isn't exactly a dummy component, but we
    // won't be reading from it, so writing to it won't do much good.
    this->new_component(new cloe::LatLongActuator(), cloe::CloeComponent::DEFAULT_LATLONG_ACTUATOR);
  }

  /**
   * Update vehicle component data for the given time step.
   *
   * If any components in the vehicle need to clear their cache or update
   * underlying data, this is the time to do it. If any trigger events are
   * associated with the vehicle, this is also the right point to trigger them.
   *
   * If the vehicle is not used in the simulation, this method will not be
   * called.
   *
   * \arg sync Simulation synchronization information
   * \return Minimum simulation time of all components
   * \see Vehicle::process
   */
  cloe::Duration process(const cloe::Sync& sync) final { return Vehicle::process(sync); }
};

/**
 * `MinimatorSimulator` binds all the above classes together in a coherent
 * structure.
 *
 * It receives the configuration, creates and provides vehicles, and maintains
 * the connection to the underlying simulator (if any).
 *
 * This class implements the `Simulator` and the `Model` interfaces, which are
 * very well documented. Have a look!
 *
 * \see cloe/simulator.hpp
 * \see cloe/model.hpp
 */
class MinimatorSimulator : public cloe::Simulator {
 public:
  /**
   * Construct a `MinimatorSimulator` instance with the given name and
   * configuration.
   *
   * This signature is required by the factory class at the end of this file.
   * Having any additional signatures is not really needed by Cloe; the only
   * reason you might have additional ones is for testing or if you want to
   * implement the factory `make` method yourself. See end of this file for
   * the macro calls that define the factory and the make method.
   *
   * \arg name Unique name by which this simulator will be referenced
   * \arg c    Configuration of this instance
   */
  MinimatorSimulator(const std::string& name, const MinimatorConfiguration& c)
      : Simulator(name), config_(c) {}

  /**
   * Destruct a `MinimatorSimulator` instance.
   *
   * We're not doing anything special, so the default will do just fine.
   */
  virtual ~MinimatorSimulator() noexcept = default;

  /**
   * Initiate a connection to the simulator and initialize all data.
   *
   * We're not actually connecting to some other simulator, so all we will do
   * here is create some vehicles and make these available to the simulation.
   *
   * \see Model::connect
   *
   * If you need to make use of a TCP connection, have a look at the
   * `TcpTransceiver` class. If this method makes any use of I/O, it is also
   * expected that a call to abort will let us cleanly exit out. The
   * `TcpTransceiverFactory` class aids us in this by using the `AbortFlag`
   * type as a way to prematurely exit.
   *
   * \see cloe/core/abort.hpp
   * \see cloe/utility/tcp_transceiver.hpp
   * \see Model::abort
   */
  void connect() final {
    // It's important when overriding methods to either call the superclass
    // method or ensure that we do everything it did. If we did not have this
    // call here, then the `is_connected()` method would return inconsistent
    // results.
    Simulator::connect();

    // For each of the vehicle names in the configuration, create a new
    // vehicle. We are responsible for ensuring that the vehicles are alive
    // for the duration of a simulation. We use `std::shared_ptr` for this.
    for (size_t i = 0; i < config_.vehicles.size(); i++) {
      vehicles_.emplace_back(std::make_shared<MinimatorVehicle>(i, config_.vehicles[i]));
    }
  }

  /**
   * Tear down the connection to the simulator and remove all data.
   *
   * This is effectively the reverse action to a connect.
   *
   * \see Model::disconnect
   */
  void disconnect() final {
    // Based on the state transition diagram in the documentation of `Model`,
    // this call should be preceded by an `abort` or `stop` call.
    assert(!is_operational());

    // Empty the list of vehicles.
    vehicles_.clear();

    // Also call superclass method.
    Simulator::disconnect();
  }

  /**
   * Reset the simulator.
   *
   * This is useful for use-cases such as machine-learning where we want to
   * restart the simulation, but not necessarily change the scenario or have
   * to completely stop and start the simulation.
   *
   * This method does not _need_ to be implemented. The default implementation
   * will throw a `ModelError` notifying the simulation that this operation
   * is not supported. But it's fairly simple for us to implement it, so we
   * will.
   *
   * \see Model::reset
   */
  void reset() final {
    disconnect();
    connect();
  }

  /**
   * Abort the simulator.
   *
   * This method may be called asynchronously. See the documentation of the
   * interface for more on this. It may also be called while a connect is still
   * in progress. The idea is that calling abort should somehow cause this
   * simulator binding to exit out of some waiting state it might be in.
   *
   * Have a look at the VTD simulator plugin for how we make use of the
   * `AbortFlag` if this is relevant to your plugin.
   *
   * \see Model::abort
   * \see Model::connect
   * \see Model::process
   */
  void abort() final {
    // We don't have any I/O here, so we override the default implementation.
    // We can safely call stop() even if we are aborting simulation.
    operational_ = false;
  }

  /**
   * Register any events, actions, or handlers with the registrar.
   *
   * Events and Actions are part of the trigger framework in Cloe. These let
   * the user specify event-action pairs to dynamically affect the simulation.
   * If there are any events or actions that a Simulator may want to expose,
   * this is the place to do it.
   *
   * Handlers are HTTP handlers that can simply expose data or provide hooks
   * for changing the simulation. It is not recommended for handlers to have
   * side-effects however, as these are not tracked by Cloe for ensuring a
   * reproducible simulation. Actions are a much better solution for this.
   *
   * \see Model::enroll
   * \see cloe/registrar.hpp
   * \see cloe/trigger.hpp
   * \see cloe/handler.hpp
   */
  void enroll(cloe::Registrar& r) final {
    // When we register an API handler, this is made available under the API
    // endpoint, so given the name `minimator` this will look like:
    //
    //    http://localhost:8080/api/simulators/minimator/state
    //
    // The handler type lets us know what kind of synchronization we should
    // use. There are three possibilities, but using the `BUFFERED` type is
    // usually the right way. The server in Cloe creates a double-buffer and
    // therefore ensures that data-races do not occur.
    //
    // Each of the following handlers makes use of the `ToJson` handler. This
    // simply uses the global `void to_json(Json&, const T&)` method for
    // deserializing into JSON. This is automatically provided by the
    // `Confable` type, but for `MinimatorSimulator` we have to define it
    // ourself.
    r.register_api_handler(
        "/state", cloe::HandlerType::BUFFERED, cloe::handler::ToJson<MinimatorSimulator>(this));
    r.register_api_handler("/configuration",
                           cloe::HandlerType::BUFFERED,
                           cloe::handler::ToJson<MinimatorConfiguration>(&config_));
  }

  /**
   * Return the number of vehicles that are available.
   *
   * This only needs to work after a connect.
   *
   * \see Simulator::num_vehicles
   */
  size_t num_vehicles() const final {
    assert(is_connected());
    return vehicles_.size();
  }

  /**
   * Return the vehicle specified.
   *
   * \see Simulator::get_vehicle
   */
  std::shared_ptr<cloe::Vehicle> get_vehicle(size_t i) const final {
    assert(i < num_vehicles());
    return vehicles_[i];
  }

  /**
   * Return the vehicle specified.
   *
   * \see Simulator::get_vehicle
   */
  std::shared_ptr<cloe::Vehicle> get_vehicle(const std::string& key) const final {
    for (const auto& v : vehicles_) {
      if (v->name() == key) {
        return v;
      }
    }
    return nullptr;
  }

  /**
   * Process everything up until the time given in `sync`.
   *
   * This is where all the step-for-step work in the simulator binding will
   * occur. When the work is done, the new simulation time should be returned.
   * This indicates that this model has reached that point in time with its
   * processing. Because `Vehicle::process` is also called, it may not be
   * necessary for this method to do much work.
   *
   * If a scenario or some other user interaction in the simulator causes the
   * simulator to stop, the `operational_` boolean can be toggled.
   *
   * \see Model::process
   * \see cloe/sync.hpp
   */
  cloe::Duration process(const cloe::Sync& sync) final {
    assert(is_connected());
    assert(is_operational());

    // Our simulator here doesn't really do anything at all, so we can keep
    // running forever.
    return sync.time();
  }

  /**
   * Serialize MinimatorSimulator into JSON.
   *
   * This is required for the `ToJson` handler that is used in the `enroll`
   * method.
   */
  friend void to_json(cloe::Json& j, const MinimatorSimulator& b) {
    j = cloe::Json{
        {"is_connected", b.connected_}, {"is_operational", b.operational_},
        {"running", nullptr},           {"num_vehicles", b.num_vehicles()},
        {"vehicles", b.vehicles_},
    };
  }

 private:
  MinimatorConfiguration config_;
  std::vector<std::shared_ptr<cloe::Vehicle>> vehicles_;
};

// The plugin manifest we will define at the end of this file requires
// a simulator factory that can be configured which will then create an
// instance of the MinimatorSimulator. The code needed for this is pretty much
// the same for each plugin, with differing minor details. The
// `DEFINE_SIMULATOR_FACTORY` macro takes care of the boilerplate code.
// The resulting class implements the `SimulatorFactory` interface.
//
// \see cloe/simulator.hpp
DEFINE_SIMULATOR_FACTORY(MinimatorFactory,
                         MinimatorConfiguration,
                         "minimator",
                         "minimalistic simulator")

// The `SimulatorFactory` requires the definition of a `make` method which
// returns a `Simulator` instance. This is also the same for most plugins,
// so we can let the `DEFINE_SIMULATOR_FACTORY_MAKE` do the lifting for us.
//
// \see cloe/simulator.hpp
DEFINE_SIMULATOR_FACTORY_MAKE(MinimatorFactory, MinimatorSimulator)

}  // namespace minimator

// Finally, we export the plugin manifest. This creates a global static struct
// which contains the type and version of plugin and a pointer to a function
// that creates the factory we want. This must be defined in a single file
// and in the global namespace.
//
// \see cloe/plugin.hpp
EXPORT_CLOE_PLUGIN(minimator::MinimatorFactory)
