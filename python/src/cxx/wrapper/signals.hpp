#pragma once

#include <fmt/format.h>
#include <pybind11/pybind11.h>
#include <functional>
#include <map>
#include <set>
#include "cloe/data_broker.hpp"

namespace cloe::py {

class Signals {
 public:
  using getter_fn = std::function<pybind11::object()>;
  using setter_fn = std::function<void(const pybind11::object&)>;

  /**
  * accessors (getter/setter)
  */
  struct accessor {
    getter_fn getter;
    setter_fn setter;
  };
  /**
  * Signals map (name -> accessors)
  */
  using accessors = std::map<std::string, accessor, std::less<>>;

  Signals() = default;

  [[nodiscard]] const getter_fn &getter(std::string_view name) const;

  [[nodiscard]] const setter_fn &setter(std::string_view name) const;

  [[nodiscard]] std::vector<std::string> bound_signals() const;

  const accessors::value_type& operator[](std::string_view key) const {
    if(auto it = accessors_.find(key); it != end(accessors_)) {
     return *it;
    }
    throw std::runtime_error(fmt::format("Could not find signal for key {}", key));
  }

  /**
    *  \brief Binds one signal to Lua
    *  \param signal signal to be bound to Lua
    *  \param lua_name name of the signal in Lua
    */
  template <typename T>
  void bind(const SignalPtr& signal, std::string_view lua_name) {
    auto inserted = accessors_.try_emplace(std::string(lua_name), accessor {
      .getter = [signal]() { return pybind11::cast(signal->value<T>()); },
      .setter = [signal](const pybind11::object& val) { signal->set_value<T>(val.cast<T>()); }
    });
    if (!inserted.second) {
      throw std::out_of_range(fmt::format(
          "Failure adding lua-accessor for signal {}. Name already exists.", lua_name));
    }
  }

 private:
  /**
  * Mapped signals
  */
  accessors accessors_;
};

}
