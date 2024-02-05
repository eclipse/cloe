#pragma once

namespace cloe {

template <typename T>
class BasicContainer;
class Signal;
template <typename T>
using Container = BasicContainer<databroker::compatible_base_t<T>>;

template <typename T>
class BasicContainer {
 public:
  using value_type = databroker::compatible_base_t<T>;

 private:
  /**
   * Access-token for regulating API access (public -> private)
   */
  struct access_token {
    explicit access_token(int /*unused*/){};
  };

  value_type value_{};
  databroker::on_value_changed_callback_t<value_type> on_value_changed_{};
  Signal* signal_{};

 public:
  BasicContainer() = default;
  BasicContainer(Signal* signal,
                 databroker::on_value_changed_callback_t<value_type>
                     on_value_changed,
                 access_token /*unused*/)
      : value_(), on_value_changed_(std::move(on_value_changed)), signal_(signal) {
    update_accessor_functions(this);
  }
  BasicContainer(const BasicContainer&) = delete;
  BasicContainer(BasicContainer<value_type>&& source) { *this = std::move(source); }

  ~BasicContainer() { update_accessor_functions(nullptr); }
  BasicContainer& operator=(const BasicContainer&) = delete;
  BasicContainer& operator=(BasicContainer&& rhs) {
    value_ = std::move(rhs.value_);
    on_value_changed_ = std::move(rhs.on_value_changed_);
    signal_ = std::move(rhs.signal_);
    update_accessor_functions(this);

    rhs.signal_ = nullptr;

    return *this;
  }
  BasicContainer& operator=(databroker::signal_type_cref_t<T> value) {
    value_ = value;
    if (on_value_changed_) {
      on_value_changed_(value_);
    }
    return *this;
  }

  const value_type& value() const { return value_; }
  value_type& value() { return value_; }
  void set_value(databroker::signal_type_cref_t<T> value) { *this = value; }

  [[nodiscard]] bool has_subscriber() const;
  [[nodiscard]] std::size_t subscriber_count() const;

  // mimic std::optional
  constexpr const value_type* operator->() const noexcept { return &value_; }
  constexpr value_type* operator->() noexcept { return &value_; }
  constexpr const value_type& operator*() const noexcept { return value_; }
  constexpr value_type& operator*() noexcept { return value_; }

 private:
  void update_accessor_functions(BasicContainer* container);

  friend class Signal;
};

// Compare two BasicContainer

template <typename T, typename U>
constexpr bool operator==(const BasicContainer<T>& lhs, const BasicContainer<U>& rhs) {
  return *lhs == *rhs;
}
template <typename T, typename U>
constexpr bool operator!=(const BasicContainer<T>& lhs, const BasicContainer<U>& rhs) {
  return *lhs != *rhs;
}
template <typename T, typename U>
constexpr bool operator<(const BasicContainer<T>& lhs, const BasicContainer<U>& rhs) {
  return *lhs < *rhs;
}
template <typename T, typename U>
constexpr bool operator<=(const BasicContainer<T>& lhs, const BasicContainer<U>& rhs) {
  return *lhs <= *rhs;
}
template <typename T, typename U>
constexpr bool operator>(const BasicContainer<T>& lhs, const BasicContainer<U>& rhs) {
  return *lhs > *rhs;
}
template <typename T, typename U>
constexpr bool operator>=(const BasicContainer<T>& lhs, const BasicContainer<U>& rhs) {
  return *lhs >= *rhs;
}

// Compare BasicContainer with a value

template <typename T, typename U>
constexpr bool operator==(const BasicContainer<T>& lhs, const U& rhs) {
  return *lhs == rhs;
}
template <typename T, typename U>
constexpr bool operator==(const T& lhs, const BasicContainer<U>& rhs) {
  return lhs == *rhs;
}
template <typename T, typename U>
constexpr bool operator!=(const BasicContainer<T>& lhs, const U& rhs) {
  return *lhs != rhs;
}
template <typename T, typename U>
constexpr bool operator!=(const T& lhs, const BasicContainer<U>& rhs) {
  return lhs != *rhs;
}
template <typename T, typename U>
constexpr bool operator<(const BasicContainer<T>& lhs, const U& rhs) {
  return *lhs < rhs;
}
template <typename T, typename U>
constexpr bool operator<(const T& lhs, const BasicContainer<U>& rhs) {
  return lhs < *rhs;
}
template <typename T, typename U>
constexpr bool operator<=(const BasicContainer<T>& lhs, const U& rhs) {
  return *lhs <= rhs;
}
template <typename T, typename U>
constexpr bool operator<=(const T& lhs, const BasicContainer<U>& rhs) {
  return lhs <= *rhs;
}
template <typename T, typename U>
constexpr bool operator>(const BasicContainer<T>& lhs, const U& rhs) {
  return *lhs > rhs;
}
template <typename T, typename U>
constexpr bool operator>(const T& lhs, const BasicContainer<U>& rhs) {
  return lhs > *rhs;
}
template <typename T, typename U>
constexpr bool operator>=(const BasicContainer<T>& lhs, const U& rhs) {
  return *lhs >= rhs;
}
template <typename T, typename U>
constexpr bool operator>=(const T& lhs, const BasicContainer<U>& rhs) {
  return lhs >= *rhs;
}

}

#include "data_broker_container_impl.hpp"
