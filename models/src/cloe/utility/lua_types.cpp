/*
 * Copyright 2023 Robert Bosch GmbH
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
 * \file cloe/utility/lua_types.cpp
 * \see  cloe/utility/lua_types.hpp
 */

#include <cloe/utility/lua_types.hpp>

#include <Eigen/Dense>

#include <cloe/component/object.hpp>

namespace sol {
template <>
struct is_automagical<Eigen::Vector2d> : std::false_type {};
template <>
struct is_automagical<Eigen::Vector3d> : std::false_type {};
template <>
struct is_automagical<Eigen::Vector4d> : std::false_type {};
}  // namespace sol

namespace cloe {
namespace utility {

/**
  * Derives matrix type traits from the given typename T
  */
template <typename T>
struct MatrixTypeTraitsDetail {};
/**
  * Derives matrix type traits from the given typename T
  */
template <typename Scalar_, int Rows_, int Cols_, int Options_, int MaxRows_, int MaxCols_>
struct MatrixTypeTraitsDetail<Eigen::Matrix<Scalar_, Rows_, Cols_, Options_, MaxRows_, MaxCols_>> {
  using Scalar = Scalar_;
  static constexpr int Rows = Rows_;
  static constexpr int Cols = Cols_;
};

/**
  * Derives the SOL constructors-type from the given typename T
  */
template <typename T, int Rows, int Cols>
struct MatrixCtors {};
/**
  * \brief Derives the SOL constructors-type from the given typename T
  * \note Specialization for 2x1 matrices
  */
template <typename T>
struct MatrixCtors<T, 2, 1> {
  using Scalar = typename MatrixTypeTraitsDetail<T>::Scalar;
  using Ctors = sol::constructors<T(), T(const T&), T(Scalar, Scalar)>;
};
/**
  * \brief Derives the SOL constructors-type from the given typename T
  * \note Specialization for 3x1 matrices
  */
template <typename T>
struct MatrixCtors<T, 3, 1> {
  using Scalar = typename MatrixTypeTraitsDetail<T>::Scalar;
  using Ctors = sol::constructors<T(), T(const T&), T(Scalar, Scalar, Scalar)>;
};
/**
  * \brief Derives the SOL constructors-type from the given typename T
  * \note Specialization for 4x1 matrices
  */
template <typename T>
struct MatrixCtors<T, 4, 1> {
  using Scalar = typename MatrixTypeTraitsDetail<T>::Scalar;
  using Ctors = sol::constructors<T(), T(const T&), T(Scalar, Scalar, Scalar, Scalar)>;
};

/**
  * Type-Traits for Eigen matrices
  */
template <typename T>
struct MatrixTypeTraits {
  using Scalar = typename MatrixTypeTraitsDetail<T>::Scalar;
  using Ctors = typename MatrixCtors<T, MatrixTypeTraitsDetail<T>::Rows,
                                     MatrixTypeTraitsDetail<T>::Cols>::Ctors;
  static constexpr int Rows = MatrixTypeTraitsDetail<T>::Rows;
  static constexpr int Cols = MatrixTypeTraitsDetail<T>::Cols;
};

/**
  * Accessor functions for Eigen matrices
  */
template <int Row, int Col>
struct MatrixAccessor {
  template <typename T, typename Scalar = typename MatrixTypeTraits<T>::Scalar>
  static Scalar get(T& matrix) {
    return matrix[Row][Col];
  }
  template <typename T, typename Scalar = typename MatrixTypeTraits<T>::Scalar>
  static void set(T& matrix, Scalar value) {
    matrix[Row][Col] = value;
  }
};

/**
  * Accessor functions for Eigen matrices
  */
template <int Row>
struct MatrixAccessor<Row, 0> {
  template <typename T, typename Scalar = typename MatrixTypeTraits<T>::Scalar>
  static Scalar get(T& matrix) {
    return matrix[Row];
  }
  template <typename T, typename Scalar = typename MatrixTypeTraits<T>::Scalar>
  static void set(T& matrix, Scalar value) {
    matrix[Row] = value;
  }
};

const char* vector_names_xyzw[] = {"x", "y", "z", "w"};
const char* vector_names_r_phi[] = {"r", "phi", "", ""};
const char* vector_names_r_theta_phi[] = {"r", "theta", "phi", ""};
const char* vector_names_rho_eta_phi[] = {"rho", "eta", "phi", ""};

const std::vector namespace_eigen = {"eigen"};

const std::array namespace_prefix = {"cloe", "types"};

/**
  * \brief Traverses the global namespace-prefix as well as the given namespace
  * \param view Lua state_view
  * \param ns_iter Iterator pointing to the beginning of the namspace-array
  * \param ns_end Iterator pointing to the end of the namspace-array
  * \param table_fn Callback accepting a SOL-table which reflects the given namespace
  */
void traverse_namespace_impl(sol::state_view view, const std::vector<const char*>& ns,
                             std::function<void(sol::table&)> table_fn) {
  const char* name;
  sol::table table;

  // traverse the global namespace-prefix
  static_assert(namespace_prefix.size() > 0);
  {
    auto iter = namespace_prefix.cbegin();
    auto end = namespace_prefix.cend();
    name = *iter++;
    table = view[name].get_or_create<sol::table>();
    while (iter != end) {
      name = *iter++;
      table = table[name].get_or_create<sol::table>();
    }
  }
  // traverse the user-supplied namespace
  {
    auto iter = ns.cbegin();
    auto end = ns.cend();
    while (iter != end) {
      name = *iter++;
      table = table[name].get_or_create<sol::table>();
    }
  }

  table_fn(table);
}

/**
  * \brief Traverses the given namespace as a preparation for the registration of a type
  * \tparam T Type of the class/enum to be registered
  * \tparam ns_size Size of the namespace array
  * \param db Instance of the DataBroker
  * \param ns Array of ASCIIZ strings describing the namespace of the enum-type
  * \param table_fn Callback accepting a SOL-table which reflects the given namespace
  */
template <typename T>
void traverse_namespace(DataBroker& db, const std::vector<const char*>& ns,
                        std::function<void(sol::table&)> table_fn) {
  db.declare_type<T>([&](sol::state_view view) { traverse_namespace_impl(view, ns, table_fn); });
}

/**
  * \brief Registers a class under a given namespace
  * \tparam T Type of the class to be registered
  * \tparam Type of the SOL constructor-class
  * \tparam ns_size Size of the namespace array
  * \param db Instance of the DataBroker
  * \param ns Array of ASCIIZ strings describing the namespace of the enum-type
  * \param type_name ASCIIZ string describing the name of the class to be registered
  */
template <typename T, typename CtorType>
sol::usertype<T> register_usertype(DataBroker& db, const std::vector<const char*>& ns,
                                   const char* type_name) {
  sol::usertype<T> result;
  traverse_namespace<T>(
      db, ns, [&](sol::table& table) { result = table.new_usertype<T>(type_name, CtorType()); });
  return result;
}

/**
  * \brief Registers an enum under a given namespace
  * \tparam T Type of the enum to be registered
  * \tparam Args Types of parameter 'args'
  * \param db Instance of the DataBroker
  * \param ns std::vector of ASCIIZ strings describing the namespace of the enum-type
  * \param type_name ASCIIZ string describing the name of the enum to be registered
  * \param args Pairs of ASCIIZ-String of one enum-value & the enum-value itself
  */
template <typename T, typename... Args>
void register_enum(DataBroker& db, const std::vector<const char*>& ns, const char* type_name,
                   Args&&... args) {
  traverse_namespace<T>(
      db, ns, [&](sol::table& table) { table.new_enum(type_name, std::forward<Args>(args)...); });
}

/**
  * \brief Registers a vector-type under a given namespace
  * \tparam T Vector-type to be registered
  * \tparam ns_size Size of the namespace array
  * \tparam ints Index-sequence into the parameter member_names
  * \param ns Array of ASCIIZ strings describing the namespace of the type
  * \param type_name ASCIIZ string describing the name of the class to be registered
  * \param member_names ASCIIZ string array describing the names of the vector properties
  */
template <typename T, std::size_t... ints>
void register_vector(DataBroker& db, const std::vector<const char*>& ns, const char* type_name,
                     const char* member_names[], std::index_sequence<ints...>) {
  sol::usertype<T> usertype =
      register_usertype<T, typename MatrixTypeTraits<T>::Ctors>(db, ns, type_name);

  // Register properties x,y,z, w
  ((usertype[member_names[ints]] = sol::property(&MatrixAccessor<ints, 0>::template get<T>,
                                                 &MatrixAccessor<ints, 0>::template set<T>)),
   ...);
  // Register operators
  usertype[sol::meta_function::unary_minus] = [](const T& rhs) -> T { return -rhs; };
  usertype[sol::meta_function::addition] = [](const T& lhs, const T& rhs) -> T {
    return lhs + rhs;
  };
  usertype[sol::meta_function::subtraction] = [](const T& lhs, const T& rhs) -> T {
    return lhs - rhs;
  };

  usertype[sol::meta_function::equal_to] = [](const T& lhs, const T& rhs) -> bool {
    return lhs == rhs;
  };

  // Register methods
  usertype["norm"] = [](const T& that) ->
      typename MatrixTypeTraits<T>::Scalar { return that.norm(); };
  usertype["dot"] = [](const T& that, const T& arg) ->
      typename MatrixTypeTraits<T>::Scalar { return that.dot(arg); };

  // Vector3x can do a cross-product
  if constexpr (3 == sizeof...(ints)) {
    usertype["cross"] = [](const T& that, const T& arg) -> T { return that.cross(arg); };
  }
}

template <typename T>
void register_vector(DataBroker& db, const std::vector<const char*>& ns, const char* class_name,
                     const char* member_names[]) {
  register_vector<T>(db, ns, class_name, member_names,
                     std::make_index_sequence<MatrixTypeTraits<T>::Rows>{});
}

std::vector namespace_cloe_object = {"cloe", "Object"};

void register_gaspedal_sensor(DataBroker& db, const std::string& vehicle,
                              std::function<const double&()> gaspedal_getter) {
  {
    using type = double;
    auto signal = db.declare<type>(fmt::format("vehicles.{}.sensor.gaspedal.position", vehicle));
    signal->set_getter<type>(std::move(gaspedal_getter));
    auto documentation = fmt::format(
        "Normalized gas pedal position for the '{}' vehicle<br><br>"
        "Range [min-max]: [0-1]",
        vehicle);
    signal->add<cloe::LuaAutocompletionTag>(
        cloe::LuaAutocompletionTag::LuaDatatype::Number,
        cloe::LuaAutocompletionTag::PhysicalQuantity::Dimensionless,
        documentation);
    signal->add<cloe::SignalDocumentation>(documentation);
  }
}
void register_wheel_sensor(DataBroker& db,
                           const std::string& vehicle,
                           const std::string& wheel_name,
                           std::function<const ::cloe::Wheel&()>
                               wheel_getter) {
  {
    using type = cloe::Wheel;
    auto signal =
        db.declare<type>(fmt::format("vehicles.{}.sensor.wheels.{}", vehicle, wheel_name));
    signal->set_getter<type>([wheel_getter]() -> const type& { return wheel_getter(); });
    auto documentation = fmt::format(
        "Wheel sensor for the front-left wheel of the '{}' vehicle<br><br>"
        "rotation: Rotational angle of wheel around y-axis in [rad]<br>"
        "velocity: Compression of the spring in [m]<br>"
        "spring_compression: Compression of the spring in [m]",
        vehicle);
    signal->add<cloe::LuaAutocompletionTag>(
        cloe::LuaAutocompletionTag::LuaDatatype::Class,
        cloe::LuaAutocompletionTag::PhysicalQuantity::Dimensionless,
        documentation);
    signal->add<cloe::SignalDocumentation>(documentation);
  }
  {
    using type = decltype(cloe::Wheel::rotation);
    auto signal =
        db.declare<type>(fmt::format("vehicles.{}.sensor.wheels.{}.rotation", vehicle, wheel_name));
    signal->set_getter<type>([wheel_getter]() -> const type& { return wheel_getter().rotation; });
    auto documentation =
        fmt::format("Sensor for the rotation around y-axis of the {} wheel of the '{}' vehicle",
                    wheel_name, vehicle);
    signal->add<cloe::LuaAutocompletionTag>(cloe::LuaAutocompletionTag::LuaDatatype::Number,
                                            cloe::LuaAutocompletionTag::PhysicalQuantity::Radian,
                                            documentation);
    signal->add<cloe::SignalDocumentation>(documentation);
  }
  {
    using type = decltype(cloe::Wheel::velocity);
    auto signal =
        db.declare<type>(fmt::format("vehicles.{}.sensor.wheels.{}.velocity", vehicle, wheel_name));
    signal->set_getter<type>([wheel_getter]() -> const type& { return wheel_getter().velocity; });
    auto documentation =
        fmt::format("Sensor for the translative velocity of the {} wheel of the '{}' vehicle",
                    wheel_name, vehicle);
    signal->add<cloe::LuaAutocompletionTag>(cloe::LuaAutocompletionTag::LuaDatatype::Number,
                                            cloe::LuaAutocompletionTag::PhysicalQuantity::Velocity,
                                            documentation);
    signal->add<cloe::SignalDocumentation>(documentation);
  }
  {
    using type = decltype(cloe::Wheel::spring_compression);
    auto signal = db.declare<type>(
        fmt::format("vehicles.{}.sensor.wheels.{}.spring_compression", vehicle, wheel_name));
    signal->set_getter<type>(
        [wheel_getter]() -> const type& { return wheel_getter().spring_compression; });
    auto documentation =
        fmt::format("Wheel sensor for spring compression of the {} wheel of the '{}' vehicle",
                    wheel_name, vehicle);
    signal->add<cloe::LuaAutocompletionTag>(cloe::LuaAutocompletionTag::LuaDatatype::Number,
                                            cloe::LuaAutocompletionTag::PhysicalQuantity::Radian,
                                            documentation);
    signal->add<cloe::SignalDocumentation>(documentation);
  }
}

void register_lua_types(DataBroker& db) {
  register_vector<Eigen::Vector2i>(db, namespace_eigen, "Vector2i", vector_names_xyzw);
  register_vector<Eigen::Vector3i>(db, namespace_eigen, "Vector3i", vector_names_xyzw);
  register_vector<Eigen::Vector4i>(db, namespace_eigen, "Vector4i", vector_names_xyzw);

  register_vector<Eigen::Vector2f>(db, namespace_eigen, "Vector2f", vector_names_xyzw);
  register_vector<Eigen::Vector3f>(db, namespace_eigen, "Vector3f", vector_names_xyzw);
  register_vector<Eigen::Vector4f>(db, namespace_eigen, "Vector4f", vector_names_xyzw);

  register_vector<Eigen::Vector2d>(db, namespace_eigen, "Vector2d", vector_names_xyzw);
  register_vector<Eigen::Vector3d>(db, namespace_eigen, "Vector3d", vector_names_xyzw);
  register_vector<Eigen::Vector4d>(db, namespace_eigen, "Vector4d", vector_names_xyzw);

  // clang-format off
  register_enum<::cloe::Object::Type>(
      db, namespace_cloe_object, "Type",
      "Unknown", ::cloe::Object::Type::Unknown,
      "Static", ::cloe::Object::Type::Static,
      "Dynamic", ::cloe::Object::Type::Dynamic
  );
  register_enum<::cloe::Object::Class>(
      db, namespace_cloe_object, "Class",
      "Unknown", ::cloe::Object::Class::Unknown,
      "Pedestrian", ::cloe::Object::Class::Pedestrian,
      "Bike", ::cloe::Object::Class::Bike,
      "Motorbike", ::cloe::Object::Class::Motorbike,
      "Car", ::cloe::Object::Class::Car,
      "Truck", ::cloe::Object::Class::Truck,
      "Trailer", ::cloe::Object::Class::Trailer
  );
  // clang-format on
}

}  // namespace utility
}  // namespace cloe
