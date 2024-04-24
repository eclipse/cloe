#include "json.hpp"

#include <fmt/format.h>

namespace {
nlohmann::json handle2json(const pybind11::handle &h) {
  // todo get rid of recursion
  if (h.is_none()) {
    return {};
  }
  if (pybind11::isinstance<pybind11::bool_>(h)) {
    return h.cast<bool>();
  }
  if (pybind11::isinstance<pybind11::int_>(h)) {
    return h.cast<std::int64_t>();
  }
  if (pybind11::isinstance<pybind11::str>(h)) {
    return h.cast<std::string>();
  }
  if (pybind11::isinstance<pybind11::tuple>(h) || pybind11::isinstance<pybind11::list>(h)) {
    auto arr = nlohmann::json::array();
    for (const auto &element : h) {
      arr.push_back(handle2json(element));
    }
    return arr;
  }
  if (pybind11::isinstance<pybind11::dict>(h)) {
    auto obj = nlohmann::json::object();
    for (const auto &key : h) {
      obj[pybind11::str(key).cast<std::string>()] = handle2json(h[key]);
    }
    return obj;
  }
  throw std::runtime_error(
      fmt::format("Could not convert {} to json.", pybind11::repr(h).cast<std::string>()));
}
}

namespace cloe::py {

nlohmann::json dict2json(const pybind11::dict &dict) {
  return handle2json(dict);
}

}
