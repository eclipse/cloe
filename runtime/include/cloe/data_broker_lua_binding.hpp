#pragma once

#include "data_broker_binding.hpp"
#include "data_broker_signal.hpp"
#include <cloe/cloe_fwd.hpp>

#include <sol/state_view.hpp>

#include <functional>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <string_view>
#include <utility>
#include <type_traits>

namespace cloe::databroker {

namespace detail {

/**
  * Detects the presence of the to_lua function (based on ADL)
  */
template <typename T, typename = void>
struct has_to_lua : std::false_type {};
/**
  * Detects the presence of the to_lua function (based on ADL)
  */
template <typename T>
struct has_to_lua<
    T, std::void_t<decltype(to_lua(std::declval<sol::state_view>(), std::declval<T*>()))>>
    : std::true_type {};
/**
  * Detects the presence of the to_lua function (based on ADL)
  */
template <typename T>
constexpr bool has_to_lua_v = has_to_lua<T>::value;

/**
  * Invokes to_lua procedure, if detecting its presence
  */
template <typename T>
void to_lua(sol::state_view lua) {
  if constexpr (has_to_lua_v<T>) {
    to_lua(lua, static_cast<T*>(nullptr));
  } else {
    // nop
  }
}

/**
    * Dynamic class which embeds all signals in shape of properties into the Lua-VM
    */
class SignalsObject {
 private:
  /**
      * Lua-Getter Function (C++ -> Lua)
      */
  using lua_getter_fn = std::function<sol::object(sol::this_state&)>;
  /**
      * Lua-Setter Function (Lua -> C++)
      */
  using lua_setter_fn = std::function<void(sol::stack_object&)>;
  /**
      * Lua accessors (getter/setter)
      */
  struct lua_accessor {
    lua_getter_fn getter;
    lua_setter_fn setter;
  };
  /**
      * Signals map (name -> accessors)
      */
  using accessors = std::unordered_map<std::string, lua_accessor>;
  /**
      * Mapped signals
      */
  accessors accessors_;
  /**
      * Lua usertype, declares this class towards Lua
      */
  sol::usertype<SignalsObject> signals_table_;

 public:
  SignalsObject(sol::state_view& lua)
      : accessors_()
      , signals_table_(lua.new_usertype<SignalsObject>(
            "SignalsObject", sol::meta_function::new_index, &SignalsObject::set_property_lua,
            sol::meta_function::index, &SignalsObject::get_property_lua)) {}

  /**
      * \brief Getter function for dynamic Lua properties
      * \param name Accessed name on Lua level
      * \param s Current Lua-state
      */
  sol::object get_property_lua(const char* name, sol::this_state s) {
    auto iter = accessors_.find(name);
    if (iter != accessors_.end()) {
      auto result = iter->second.getter(s);
      return result;
    } else {
      throw std::out_of_range(
          fmt::format("Failure to access signal '{}' from Lua since it is not bound.", name));
    }
  }
  /**
      * \brief Setter function for dynamic Lua properties
      * \param name Accessed name on Lua level
      * \param object Lua-Object assigned to the property
      */
  void set_property_lua(const char* name, sol::stack_object object) {
    auto iter = accessors_.find(name);
    if (iter != accessors_.end()) {
      iter->second.setter(object);
    } else {
      throw std::out_of_range(
          fmt::format("Failure to access signal '{}' from Lua since it is not bound.", name));
    }
  }
  /**
     * Factory which produces the gluecode to r/w Lua properties
     */
  template <typename T>
  struct LuaAccessorFactory {
    using type = T;
    using value_type = T;
    static lua_accessor make(const SignalPtr& signal) {
      lua_accessor result;
      result.getter = [signal](sol::this_state& state) -> sol::object {
        const value_type& value = signal->value<value_type>();
        return sol::make_object(state, value);
      };
      result.setter = [signal](sol::stack_object& obj) -> void {
        T value = obj.as<value_type>();
        signal->set_value<value_type>(value);
      };
      return result;
    }
  };
  /**
     * Factory which produces the gluecode to r/w Lua properties
     * \note Specialization for std::optional
     */
  template <typename T>
  struct LuaAccessorFactory<std::optional<T>> {
    using type = std::optional<T>;
    using value_type = T;
    static lua_accessor make(const SignalPtr& signal) {
      lua_accessor result;
      result.getter = [signal](sol::this_state& state) -> sol::object {
        const type& value = signal->value<type>();
        if (value) {
          return sol::make_object(state, value.value());
        } else {
          return sol::make_object(state, sol::lua_nil);
        }
      };
      result.setter = [signal](sol::stack_object& obj) -> void {
        type value;
        if (obj != sol::lua_nil) {
          value = obj.as<value_type>();
        }
        signal->set_value<type>(value);
      };
      return result;
    }
  };

  /**
      *  \brief Binds one signal to Lua
      *  \param signal signal to be bound to Lua
      *  \param lua_name name of the signal in Lua
      */
  template <typename T>
  void bind(const SignalPtr& signal, std::string_view lua_name) {
    lua_accessor accessor = LuaAccessorFactory<T>::make(signal);
    auto inserted = accessors_.try_emplace(std::string(lua_name), std::move(accessor));
    if (!inserted.second) {
      throw std::out_of_range(fmt::format(
          "Failure adding lua-accessor for signal {}. Name already exists.", lua_name));
    }
  }
};

}  // namespace detail

class LuaDataBrokerBinding : public DataBrokerBinding {
 public:
  /**
  * Function which declares a specific datatype to the Lua-VM
  */
  using lua_signal_declarator_t = std::function<void(sol::state_view)>;
  /**
  * Function which integrates a specific datum into the Lua-VM
  */
  using lua_signal_adapter_t = std::function<void(const SignalPtr&, sol::state_view, std::string_view)>;

  explicit LuaDataBrokerBinding(sol::state_view lua) : lua_(std::move(lua)), signals_object_(lua_) {
    declare<bool>();
    declare<int8_t>();
    declare<uint8_t>();
    declare<int16_t>();
    declare<uint16_t>();
    declare<int32_t>();
    declare<uint32_t>();
    declare<int64_t>();
    declare<uint64_t>();
    declare<float>();
    declare<double>();
  }

  void bind(std::string_view signals_name) override {
    lua_[signals_name] = &signals_object_;
  }

  void bind_signal(SignalPtr signal, std::string_view signal_name, std::string_view lua_name) override {
    auto type = std::type_index(*signal->type());
    auto iter = bindings_.find(type);
    if (iter == bindings_.end()) {
      throw std::runtime_error(
          fmt::format("DataBroker: <internal logic error>: Lua type binding "
                      "for type \"{}\" not implemented", signal->type()->name()));
    }
    const lua_signal_adapter_t& adapter = iter->second;
    adapter(signal, lua_, lua_name);
  }

  /**
    * \brief Declares a DataType to Lua (if not yet done)
    * \note: The function can be used independent of a bound Lua instance
    */
  template<typename T>
  void declare() {
    assert_static_type<T>();
    using compatible_type = databroker::compatible_base_t<T>;

    // Check whether this type was already processed, if not declare it and store an adapter function in bindings_
    std::type_index type{typeid(compatible_type)};
    auto iter = bindings_.find(type);
    if (iter == bindings_.end()) {
      // Check whether this type was already declared to the Lua-VM, if not declare it
      auto declared_types_iter = declared_types().find(type);
      if (declared_types_iter == declared_types().end()) {
        declared_types()[type] = true;
        ::cloe::databroker::detail::to_lua<T>(lua_);
      }

      // Create adapter for Lua-VM
      lua_signal_adapter_t adapter = [this](const SignalPtr& signal, sol::state_view state,
                                            std::string_view lua_name) {
        // Subscribe to the value-changed event to indicate the signal is used
        signal->subscribe<T>([](const T&) {});
        // Implement the signal as a property in Lua
        signals_object_.bind<T>(signal, lua_name);
      };
      // Store adapter function
      bindings_.emplace(type, std::move(adapter));
    }
  }

  /**
  * \brief Declares a DataType to Lua (if not yet done)
  * \note: The function can be used independent of a bound Lua instance
  */
  template <typename T>
  void declare_type(lua_signal_declarator_t type_declarator) {
    assert_static_type<T>();
    using compatible_type = compatible_base_t<T>;
    std::type_index type{typeid(compatible_type)};
    auto iter = declared_types().find(type);
    if (iter == declared_types().end()) {
      declared_types()[type] = true;
      // declare type
      type_declarator(lua_);
    }
  }

  /**
   * \brief Binds the signals-object to Lua
   * \param signals_name Name which shall be used for the table
   * \param parent_table Parent-table to use
   */
  void bind(std::string_view signals_name, sol::table parent) {
    parent[signals_name] = &signals_object_;
  }

 private:

  sol::state_view lua_;
  std::unordered_map<std::type_index, lua_signal_adapter_t> bindings_{};
  /**
  * Instance of signals body which incorporates all bound signals
  */
  detail::SignalsObject signals_object_;
};

/**
  * Signal-Metainformation for generation of Lua documentation
  */
struct LuaAutocompletionTag : MetaInformation::Tag<LuaAutocompletionTag> {
/**
 * X-Macro: enum definition & enum-to-string conversion
 */
#define LUADATATYPE_LIST \
  X(Class, 0)            \
  X(Number, 1)           \
  X(String, 2)

  enum class LuaDatatype {
#define X(name, value) name = value,
    LUADATATYPE_LIST
#undef X
  };

  friend std::string to_string(LuaDatatype type) {
    switch (type) {
#define X(name, value)    \
  case LuaDatatype::name: \
    return #name;
      LUADATATYPE_LIST
#undef X
      default:
        return {};
    }
  }
#undef LUADATATYPE_LIST

/**
 * X-Macro: enum definition & enum-to-string conversion
 */
#define PHYSICALQUANTITIES_LIST \
  X(Dimensionless, "[]")        \
  X(Radian, "[rad]")            \
  X(Length, "[m]")              \
  X(Time, "[s]")                \
  X(Mass, "[kg]")               \
  X(Temperature, "[K]")         \
  X(ElectricCurrent, "[A]")     \
  X(Velocity, "[m/s]")          \
  X(Acceleration, "[m/s^2]")    \
  X(Jerk, "[m/s^3]")            \
  X(Jounce, "[m/s^4]")          \
  X(Crackle, "[m/s^5]")

  enum class PhysicalQuantity {
#define X(name, value) name,
    PHYSICALQUANTITIES_LIST
#undef X
  };

  friend std::string to_string(PhysicalQuantity type) {
    switch (type) {
#define X(name, value)         \
  case PhysicalQuantity::name: \
    return #value;
      PHYSICALQUANTITIES_LIST
#undef X
      default:
        return {};
    }
  }
#undef PHYSICALQUANTITIES_LIST

  /**
    * Lua datatype of the signal
    */
  LuaDatatype datatype;
  /**
    * Lua datatype of the signal
    */
  PhysicalQuantity unit;
  /**
    * Documentation text
    * \note Use <br> to achieve a linebreak
    */
  std::string text;

  LuaAutocompletionTag(LuaDatatype datatype_, PhysicalQuantity unit_, std::string text_)
      : datatype{std::move(datatype_)}, unit{std::move(unit_)}, text{std::move(text_)} {}
};

}
