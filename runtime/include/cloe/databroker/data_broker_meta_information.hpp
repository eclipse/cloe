#pragma once

#include <typeindex>
#include <any>
#include <unordered_map>
#include <string>

namespace cloe {
/**
 * MetaInformation collects abstract metainformation
 *
 * \note: Design-Goals:
 * - Design-#1: Key-Value (cardinality: 0-1:1). The key defines the value-type.
 * - Design-#2: Type-erasing techniques shall not eradicate type-safety nor put additional validation steps onto the users.
 * \note: Implementation-Notes:
 * - Implementation-#1:
 *   Implementations like "all-is-byte-arrays" or "JSON" were considered and disregarded.
 *   E.g. JSON is a serialization format. Using JSON-Schema Design-#2 would be covered.
 *   This would imply a) a dependency on multiple levels & b) significant runtime efforts.
 *   Shooting sparrows with canons. Pure C++ can do the job in <50 LOC + some for porcellain.
 */
class MetaInformation {
 private:
  using metainformation_map_t = std::unordered_map<std::type_index, std::any>;
  metainformation_map_t metainformations_;

 public:
  /**
    * Tag which identifies the metainformation and carries the type information of the actual metainformation
    */
  template <typename T>
  struct Tag {
   public:
    using tag_type = T;
  };

 public:
  MetaInformation() = default;
  virtual ~MetaInformation() = default;

 private:
  template <typename T>
  constexpr void assert_static_type() {
    // prevent usage of references
    static_assert(std::is_reference_v<typename T::tag_type> == false,
                  "References are unsupported.");
  }

 public:
  template <typename T>
  /**
    * Removes an metainformation
    * \tparam Tag of the metainformation to be removed
    */
  void remove() {
    auto tindex = std::type_index(typeid(T));
    auto iter = metainformations_.find(tindex);
    if (iter != metainformations_.end()) {
      metainformations_.erase(iter);
    }
  }
  /**
    * Adds a metainformation
    * \tparam T Type of the metainformation-tag
    * \param metainformation_any Actual metainformation to be added
    */
  template <typename T>
  void add_any(std::any metainformation_any) {
    auto tindex = std::type_index(typeid(T));
    auto iter = metainformations_.find(tindex);
    if (iter != metainformations_.end()) {
    }
    metainformations_[tindex] = std::move(metainformation_any);
  }
  /**
     * Returns a metainformation
     * \tparam T Type of the metainformation-tag
     * \returns std:any* if the metainformation is present, nullptr otherwise
     */
  template <typename T>
  const std::any* get_any() const {
    auto tindex = std::type_index(typeid(T));
    auto iter = metainformations_.find(tindex);
    if (iter != metainformations_.end()) {
      const std::any& metainformation_any = iter->second;
      return &metainformation_any;
    } else {
      return nullptr;
    }
  }
  /**
    * Returns a metainformation
    * \tparam T Type of the metainformation-tag
    * \returns Annotation of type T::tag_type* if the metainformation is present, nullptr otherwise
    */
  template <typename T>
  std::enable_if_t<!std::is_void_v<typename T::tag_type>, const typename T::tag_type>* get() const {
    const std::any* metainformation_any = get_any<T>();
    return (metainformation_any != nullptr)
               ? std::any_cast<typename T::tag_type>(metainformation_any)
               : nullptr;
  }
  /**
    * Returns a metainformation
    * \tparam T Type of the metainformation-tag
    * \returns true if the metainformation is present, false otherwise
    */
  template <typename T>
  std::enable_if_t<std::is_void_v<typename T::tag_type>, bool> get() const {
    const std::any* metainformation_any = get_any<T>();
    return (metainformation_any != nullptr);
  }

  /**
     * Adds a metainformation
     * \tparam T Type of the metainformation-tag
     * \param metainformation Actual metainformation to be added
     * \note This overload is enabled only when the effective tag_type is move constructible
     */
  template <typename T>
  // clang-format off
  std::enable_if_t<
       !std::is_void_v<typename T::tag_type>
    && !std::is_base_of_v< Tag< T >, T >
    &&  std::is_move_constructible_v<typename T::tag_type>
  >
  // clang-format on
  add(typename T::tag_type metainformation) {
    assert_static_type<T>();

    std::any metainformation_any = std::move(metainformation);
    add_any<T>(std::move(metainformation_any));
  }
  /**
     * Adds a metainformation
     * \tparam T Type of the metainformation-tag
     * \param metainformation Actual metainformation to be added
     * \note This overload is enabled only when the effective tag_type is copy constructible
     */
  template <typename T>
  // clang-format off
  std::enable_if_t<
       !std::is_void_v<typename T::tag_type>
    && !std::is_base_of_v< Tag< T >, T >
    &&  std::is_copy_constructible_v<typename T::tag_type>
    && !std::is_move_constructible_v<typename T::tag_type>
  >
  // clang-format on
  add(const typename T::tag_type& metainformation) {
    assert_static_type<T>();

    std::any metainformation_any = metainformation;
    add_any<T>(std::move(metainformation_any));
  }
  /**
     * Adds a metainformation
     * \tparam T Type of the metainformation-tag
     * \note This overload is enabled only when the effective tag_type is void
     */
  template <typename T>
  // clang-format off
  std::enable_if_t<
    std::is_void_v<typename T::tag_type>
  >
  // clang-format on
  add() {
    std::any metainformation_any;
    add_any<T>(std::move(metainformation_any));
  }
  /**
    * Adds a metainformation
    * \tparam T Type of the metainformation-tag
    * \param metainformation Actual metainformation to be added
    * \note This overload is enabled only when the tag inherits Tag<> and is the effective tag_type
    *
    * Usage Note:
    *
    * ```
    * struct my_tag : cloe::MetaInformation::Tag<my_tag> {
    *  ...
    * };
    * ...
    * my_tag tag;
    * ...
    * add(std::move(tag));
    * ```
    */
  template <typename T>
  // clang-format off
  std::enable_if_t<
       std::is_base_of_v< Tag< T >, T >
    && std::is_same_v<T, typename T::tag_type>
  >
  // clang-format on
  add(T metainformation) {
    assert_static_type<T>();

    std::any metainformation_any = std::move(metainformation);
    add_any<T>(std::move(metainformation_any));
  }
  /**
    * Adds a metainformation constructed from the supplied parameters
    * \tparam T Type of the metainformation-tag
    * \tparam TArgs... Type of the metainformation c'tor arguments
    * \param args Arguments for the c'tor of the metainformation
    * \note This overload is enabled only when the tag inherits Tag<> and is the effective tag_type
    *
    * Usage Note:
    *
    * ```
    * add<TagType>(arg1, arg2, ...);
    *  ...
    * };
    * ```
    */
  template <typename T, typename... TArgs>
  // clang-format off
  std::enable_if_t<
       std::is_base_of_v< Tag< T >, T >
    && std::is_same_v<T, typename T::tag_type>
  >
  // clang-format on
  add(TArgs... args) {
    assert_static_type<T>();
    T metainformation(std::forward<TArgs>(args)...);
    std::any metainformation_any = std::move(metainformation);
    add_any<T>(std::move(metainformation_any));
  }
};

struct SignalDocumentation : MetaInformation::Tag<SignalDocumentation> {
  /**
    * Documentation text
    * \note Use <br> to achieve a linebreak
    */
  std::string text;

  SignalDocumentation(std::string text_) : text{std::move(text_)} {}

  friend const std::string& to_string(const SignalDocumentation& doc) { return doc.text; }
};
}
