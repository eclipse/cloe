#pragma once

#include "data_broker_event.hpp"
#include "data_broker_meta_information.hpp"

#include <functional>

namespace cloe {

template<typename T>
class BasicContainer;

/**
 * Signal represents the properties of a signal at runtime
 *
 * \note: Design-Goals:
 * - Design-#1: The class shall expose a uniform interface (via type-erasure)
 * - Design-#2: The class shall be constructible via external helpers
 * - Design-#3: CppCoreGuidelines CP-1 does not apply. See also: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rconc-multi
 * - Design-#4: Templates shall be instantiated explicitly
 * \note: Implementation-Notes:
 * - Implementation-#1: Type specific aspects are implemented in templates (Design-#1)
 * - Implementation-#2: Objects are created via a factory-method (to prevent instances of incomplete initialization)
 * - Implementation-#3: An access-token is used to have a public c'tor which in fact shall be inaccesible (Design-#2).
 */
class Signal {
 private:
  /**
   * Access-token for regulating API access (public -> private).
   */
  struct access_token {
    explicit access_token(int /*unused*/){};
  };

 public:
  /**
   * Getter-function types
   *
   * Note:
   * When implementing this function with lamdas (e.g. set_getter<>() )
   * take care to explicitly define the return-type.
   *
   * Example:
   * []() -> const int& { return value; }
   *
   * Undefined behaviour:
   * []()     { return value; }
   *      ^^^ No return type specified
   */
  template <typename T>
  using typed_get_value_function_t = std::function<databroker::signal_type_cref_t<T>()>;
  using type_erased_get_value_function_t = std::any;

  /**
   * Setter-function types
   */
  template <typename T>
  using typed_set_value_function_t = std::function<void(databroker::signal_type_cref_t<T>)>;
  using type_erased_set_value_function_t = std::any;
  /**
   * Event types
   */
  template <typename T>
  using typed_value_changed_event_t = databroker::Event<databroker::signal_type_cref_t<T>>;
  using type_erased_value_changed_event_t = std::any;
  /**
   * Event trigger-function types
   */
  template <typename T>
  using typed_on_value_change_event_function_t =
      std::function<void(databroker::signal_type_cref_t<T>)>;
  using type_erased_on_value_change_event_function_t = std::any;

 private:
  /// Name(s) of the signal
  std::vector<std::string> names_{};
  /// std::type_info of the signal
  const std::type_info* type_{nullptr};
  /// getter-function
  type_erased_get_value_function_t get_value_{};
  /// setter-function
  type_erased_set_value_function_t set_value_{};
  /// Event which gets raised when the signal value changes
  type_erased_value_changed_event_t value_changed_event_{};
  /// Triggers the value-changed event
  type_erased_on_value_change_event_function_t on_value_changed_{};
  /// std::function returning the count of event subscribers
  std::function<std::size_t()> subscriber_count_{};
  /// metadata accompanying the signal
  MetaInformation metainformations_;

  /**
   * Private default c'tor
   * \note: Implementation-#2: The class shall be created only via a factory-method
   */
  Signal() = default;

 public:
  /**
   * Public c'tor, accessible only via private access-token
   * \note: Design-#1: The class shall be constructible via external helpers
   */
  explicit Signal(access_token /*unused*/) : Signal() {}
  Signal(const Signal&) = delete;
  Signal(Signal&&) = default;
  virtual ~Signal() = default;
  Signal& operator=(const Signal&) = delete;
  Signal& operator=(Signal&&) = default;

  /**
   * Return the type info for the signal.
   */
  constexpr const std::type_info* type() const { return type_; }

 private:
  /**
   * Validate that the stored signal type matches the template type.
   *
   * \tparam T Expected type of the signal
   */
  template <typename T>
  constexpr void assert_dynamic_type() const {
    const std::type_info* static_type = &typeid(T);
    const std::type_info* dynamic_type = type();
    if ((dynamic_type == nullptr) || (!(*dynamic_type == *static_type))) {
      throw std::logic_error(
          fmt::format("mismatch between dynamic-/actual-type and static-/requested-type; "
                      "signal type: {}, requested type: {}",
                      dynamic_type != nullptr ? dynamic_type->name() : "", static_type->name()));
    }
  }

 public:
  /**
   * Return the getter function of the signal.
   *
   * \tparam T Type of the signal
   * \return Getter function of the signal
   *
   * Example:
   * ```
   * Signal s = ...;
   * const typed_get_value_function_t<int>* f = s.getter<int>();
   * int v = (*f)();
   * ```
   */
  template <typename T>
  const typed_get_value_function_t<T>* getter() const {
    assert_static_type<T>();
    assert_dynamic_type<T>();

    const typed_get_value_function_t<T>* get_value_fn =
        std::any_cast<typed_get_value_function_t<T>>(&get_value_);
    return get_value_fn;
  }

  /**
   * Set the getter function of the signal.
   *
   * \tparam T Type of the signal
   * \param get_value Getter function of the signal
   *
   * Example:
   * ```
   * Signal s = ...;
   * s.set_getter<type>([&]() -> const type & { return value; });
   * ```
   * Usage Note: When using lambdas, the explicit return-type definition is important!
   *
   * Undefined behaviour:
   * ```
   * Signal s = ...;
   * s.set_getter<type>([&]()     { return value; });
   *                          ^^^ No return type specified, type conversion rules apply
   * ```
   */
  template <typename T>
  void set_getter(typed_get_value_function_t<T> get_value_fn) {
    assert_static_type<T>();
    assert_dynamic_type<T>();

    get_value_ = std::move(get_value_fn);
  }

  /**
   * Return the current value of the signal.
   *
   * \tparam T Type of the signal
   * \return databroker::signal_type_cref_t<T>, Current value of the signal
   * \note databroker::compatible_base_t<T> == T, if the method compiles
   */
  template <typename T>
  databroker::signal_type_cref_t<T> value() const {
    assert_static_type<T>();
    assert_dynamic_type<T>();
    using compatible_type = databroker::compatible_base_t<T>;

    const typed_get_value_function_t<T>* getter_fn = getter<compatible_type>();
    if (getter_fn && (*getter_fn)) {
      databroker::signal_type_cref_t<T> value = getter_fn->operator()();
      return value;
    }
    throw std::logic_error(
        fmt::format("unable to get value for signal without getter-function: {}", names_.front()));
  }

  /**
   * Return the getter function of the signal.
   *
   * \tparam T Type of the signal
   * \return const typed_set_value_function_t<T>*, Getter function of the signal
   */
  template <typename T>
  const typed_set_value_function_t<T>* setter() const {
    assert_static_type<T>();
    assert_dynamic_type<T>();

    const typed_set_value_function_t<T>* set_value_fn =
        std::any_cast<typed_set_value_function_t<T>>(&set_value_);
    return set_value_fn;
  }

  /**
   * Set the setter function of the signal.
   *
   * \tparam T Type of the signal
   * \param set_value Getter function of the signal
   */
  template <typename T>
  void set_setter(typed_set_value_function_t<T> set_value_fn) {
    assert_static_type<T>();
    assert_dynamic_type<T>();

    set_value_ = std::move(set_value_fn);
  }

  /**
   * Set the value of the signal.
   *
   * \tparam T Type of the signal
   * \param value Value of the signal
   */
  template <typename T>
  void set_value(databroker::signal_type_cref_t<T> value) const {
    assert_static_type<T>();
    assert_dynamic_type<T>();
    using compatible_type = databroker::compatible_base_t<T>;

    const typed_set_value_function_t<T>* setter_fn = setter<compatible_type>();
    if (setter_fn && *setter_fn) {
      setter_fn->operator()(value);
      return;
    }
    throw std::logic_error(
        fmt::format("unable to set value for signal without setter-function: {}", names_.front()));
  }

  /**
   * Return the trigger function for the value_changed event.
   *
   * \tparam T Type of the signal
   * \return Trigger function for raising the value_changed event
   */
  template <typename T>
  const typed_on_value_change_event_function_t<T>& trigger() const {
    assert_static_type<T>();
    assert_dynamic_type<T>();

    const typed_on_value_change_event_function_t<T>* trigger_fn =
        std::any_cast<typed_on_value_change_event_function_t<T>>(&on_value_changed_);
    assert(trigger_fn);
    return *trigger_fn;
  }

  /**
   * Tags a signal with metadata
   *
   * \tparam T Type of the tag
   * \param metadata Metadata used to tag the signal
   * \note This is the overload for non-void tags (T2 != void)
   */
  template <typename T>
  std::enable_if_t<!std::is_void_v<typename T::tag_type>> add(typename T::tag_type metadata) {
    static_assert(std::is_reference_v<typename T::tag_type> == false);
    metainformations_.add<T>(metadata);
  }
  /**
   * Tags a signal with metadata constructed from parameters
   *
   * \tparam T Type of the tag
   * \tparam TArgs Type of the metadata c'tor parameters
   * \param args Metadata c'tor arguments
   * \note This is the overload for non-void tags (T2 != void)
   */
  template <typename T, typename... TArgs>
  std::enable_if_t<!std::is_void_v<typename T::tag_type>> add(TArgs&&... args) {
    static_assert(std::is_reference_v<typename T::tag_type> == false);
    metainformations_.add<T>(std::forward<TArgs>(args)...);
  }

  /**
   * Tags a signal with a void tag
   *
   * \tparam T Type of the tag
   */
  template <typename T>
  // clang-format off
  std::enable_if_t<
    std::is_void_v<typename T::tag_type>
  >
  // clang-format on
  add() {
    metainformations_.add<T>();
  }
  /**
   * Get a tag of the signal
   *
   * \tparam T Type of the tag
   * \returns const typename T::tag_type* pointing to the tag-value (or nullptr), if typename T::tag_type != void
   * \returns bool expressing the presence of the tag, if typename T::tag_type == void
   */
  template <typename T>
  auto metadata() -> decltype(metainformations_.get<T>()) {
    return metainformations_.get<T>();
  }
  /**
   * Get all tags of the signal
   */
  const MetaInformation& metadatas() const { return metainformations_; }

 private:
  /**
   * Unpack the event to Event<const T*> and provide it to the caller.
   *
   * \tparam T Type of the event
   * \param callback Caller function which accepts the unpacked event
   * \note: In case type T and actual type do not match an exception is thrown
   */
  template <typename T>
  void subscribe_impl(
      const std::function<void(databroker::Event<databroker::signal_type_cref_t<T>>&)>& callback) {
    typed_value_changed_event_t<T>* value_changed_event =
        std::any_cast<typed_value_changed_event_t<T>>(&value_changed_event_);
    if (callback) {
      assert(value_changed_event);
      callback(*value_changed_event);
    }
  }

 public:
  /**
   * Subscribe to value-changed events.
   *
   * \tparam T Type of the signal
   * \param callback event-function which will be called when the value changed
   */
  template <typename T>
  void subscribe(databroker::on_value_changed_callback_t<T> callback) {
    assert_static_type<T>();
    assert_dynamic_type<T>();

    subscribe_impl<T>(
        [callback = std::move(callback)](typed_value_changed_event_t<T>& value_changed_event) {
          value_changed_event.add(std::move(callback));
        });
  }

  /**
   * Return the count of subscribers to the value_changed event.
   *
   * \return size_t Count of subscribers to the value_changed event
   */
  std::size_t subscriber_count() const { return subscriber_count_(); }

  /**
   * Indicate whether the value_changed event has subscribers.
   *
   * \return bool True if the value_changed event has subscribers, false otherwise
   */
  bool has_subscriber() const { return subscriber_count() > 0; }

  /**
   * Return the list of names assigned to the signal.
   *
   * \return List of names assigned to the signal
   */
  const std::vector<std::string>& names() const { return names_; }

  /**
   * Return the first assigned name of the signal.
   *
   * \return First name of the signal
   */
  const std::string& name() const {
    if (names_.empty()) {
      throw std::logic_error("signal does not have a name");
    }
    return names_.front();
  }

  /**
   * Return the first assigned name of the signal.
   *
   * \return First name of the signal
   */
  std::string name_or(std::string def) const {
    if (names_.empty()) {
      return def;
    }
    return names_.front();
  }

  /**
   * Add a name of the signal.
   *
   * \param name Name of the signal
   */
  void add_name(std::string_view name) { names_.emplace_back(name); }

 private:
  /**
   * Factory for Signal.
   *
   * \tparam T Type of the signal
   * \param name Name of the signal
   * \return owning unique pointer to Signal
   * \note: Design-Note #1 reasons the existance of this factory
   */
  template <typename T>
  static std::unique_ptr<Signal> make() {
    assert_static_type<T>();
    using compatible_type = databroker::compatible_base_t<T>;

    auto signal = std::make_unique<Signal>(access_token(0));
    signal->initialize<compatible_type>();
    return signal;
  }

  /**
   * Create the container for a signal.
   *
   * \tparam T Type of the signal
   * \return BasicContainer for the signal
   * \note Design-#4: Templates shall be instantiated explicitly
   */
  template <typename T>
  BasicContainer<T> create_container();

 public:
 private:
  template <typename T>
  void initialize() {
    assert_static_type<T>();

    type_ = &typeid(T);
    // Create event
    value_changed_event_ = typed_value_changed_event_t<T>();
    typed_value_changed_event_t<T>* value_changed_event =
        &std::any_cast<typed_value_changed_event_t<T>&>(value_changed_event_);
    // Create event-trigger
    typed_on_value_change_event_function_t<T> on_value_changed =
        [value_changed_event](databroker::signal_type_cref_t<T> value) {
          value_changed_event->raise(std::move(value));
        };
    on_value_changed_ = on_value_changed;
    // Create subscriber_count function
    subscriber_count_ = [value_changed_event]() { return value_changed_event->count(); };
  }

  template <typename T>
  friend class BasicContainer;
  friend class DataBroker;
};
}

#include "data_broker_signal_impl.hpp"
