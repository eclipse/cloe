#pragma once

#include <fmt/format.h>
#include <pybind11/pybind11.h>
#include <functional>
#include <map>
#include <memory>
#include <string_view>
#include <cloe/data_broker.hpp>

namespace cloe::py {

namespace detail {
using getter_fn = std::function<pybind11::object()>;
using setter_fn = std::function<void(const pybind11::object&)>;
/**
  * accessors (getter/setter)
  */
struct PYBIND11_EXPORT accessor {
  getter_fn getter;
  setter_fn setter;
};
}

struct PYBIND11_EXPORT Signals {
  using getter_fn = detail::getter_fn;
  using setter_fn = detail::setter_fn;
  /**
  * Signals map (name -> accessors)
  */
  using accessors = std::map<std::string, detail::accessor, std::less<>>;

  Signals();
  ~Signals();
  Signals(const Signals &) = delete;
  Signals& operator=(const Signals&) = delete;
  Signals(Signals&&) = default;
  Signals& operator=(Signals&&) = default;

  [[nodiscard]] const getter_fn &getter(std::string_view name) const;

  [[nodiscard]] const setter_fn &setter(std::string_view name) const;

  [[nodiscard]] std::vector<std::string> bound_signals() const;

  const accessors::value_type& operator[](std::string_view key) const {
    if(auto it = accessors_->find(key); it != accessors_->end()) {
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
    auto inserted = accessors_->try_emplace(std::string(lua_name), accessors::mapped_type {
      .getter = [signal]() { return pybind11::cast(signal->value<T>()); },
      .setter = [signal](const pybind11::object& val) { signal->set_value<T>(val.cast<T>()); }
    });
    if (!inserted.second) {
      throw std::out_of_range(fmt::format(
          "Failure adding lua-accessor for signal {}. Name already exists.", lua_name));
    }
  }

  /**
  * Mapped signals
  */
  std::unique_ptr<accessors> accessors_;
};

}
