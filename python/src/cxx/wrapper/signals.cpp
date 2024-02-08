#include "signals.hpp"

namespace cloe::py {

std::vector<std::string_view> Signals::bound_signals() const {
  std::vector<std::string_view> result{};
  result.reserve(accessors_.size());
  for (const auto &[key, _] : accessors_) {
    result.emplace_back(key);
  }
  return result;
}
const Signals::setter_fn &Signals::setter(std::string_view name) const {
  if (auto it = accessors_.find(name); it != end(accessors_)) {
    return it->second.setter;
  } else {
    throw std::out_of_range(
        fmt::format("Failure to access signal '{}' from Python since it is not bound.", name));
  };
}
const Signals::getter_fn &Signals::getter(std::string_view name) const {
  if(auto it = accessors_.find(name); it != end(accessors_)) {
    return it->second.getter;
  } else {
    throw std::out_of_range(
        fmt::format("Failure to access signal '{}' from Python since it is not bound.", name));
  }
}
}  // namespace cloe::py
