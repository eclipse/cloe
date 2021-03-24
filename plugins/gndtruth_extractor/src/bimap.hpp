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
 * \file bimap.hpp
 */

#include <vector>  // for vector<>

#include <boost/algorithm/string/join.hpp>  // for boost::algorithm::join
#include <boost/assign/list_of.hpp>         // for list_of
#include <boost/bimap.hpp>                  // for bimap
#include <boost/range/adaptor/map.hpp>      // for boost::adaptors::map
#include <boost/range/algorithm/copy.hpp>   // for boost::algorithm::copy

namespace cloe {

/// BimapBase1 implements basic functions
template <typename Type1, typename Type2, typename CRTP>
class BimapBase1 {
 public:
  typedef Type1 type1;
  typedef Type2 type2;
  typedef boost::bimap<Type1, Type2> bm_type;
  static const typename bm_type::left_map& left() { return CRTP::map_.left; }
  static const typename bm_type::right_map& right() { return CRTP::map_.right; }
  static typename bm_type::left_const_iterator beginLeft() { return CRTP::map_.left.begin(); }
  static typename bm_type::left_const_iterator endLeft() { return CRTP::map_.left.end(); }
  static typename bm_type::left_const_iterator findLeft(const type1& rhs) {
    return CRTP::map_.left.find(rhs);
  }
  static typename bm_type::right_const_iterator beginRight() { return CRTP::map_.right.begin(); }
  static typename bm_type::right_const_iterator endRight() { return CRTP::map_.right.end(); }
  static typename bm_type::right_const_iterator findRight(const type2& rhs) {
    return CRTP::map_.right.find(rhs);
  }
};

/// BimapBase class for two-distinct types, which will get derived further
/// Remarks:
/// The methods defined by this class are not possible for a pair of identical template-arguments
/// Therefore
/// - a partial template specialization of this class exists
/// - are BimapBase1 & BimapBase2 split into two classes
template <typename Type1, typename Type2, typename CRTP>
class BimapBase2 : public BimapBase1<Type1, Type2, CRTP> {
 public:
  typedef BimapBase1<Type1, Type2, CRTP> base;
  typedef typename base::bm_type bm_type;
  typedef typename base::type1 type1;
  typedef typename base::type2 type2;

  /// Implementation of begin() for the left-map
  static auto beginImpl(const Type1* = nullptr) -> decltype(base::beginLeft()) {
    return base::beginLeft();
  }
  /// Implementation of begin() for the right-map
  static auto beginImpl(const Type2* = nullptr) -> decltype(base::beginRight()) {
    return base::beginRight();
  }
  /// Derivate of map::begin()
  template <typename T>
  static auto begin() -> decltype(beginImpl((T*)nullptr)) {
    return beginImpl((T*)nullptr);
  }
  /// Implementation of end() for the left-map
  static auto endImpl(const Type1* = nullptr) -> decltype(base::endLeft()) {
    return base::endLeft();
  }
  /// Implementation of end() for the right-map
  static auto endImpl(const Type2* = nullptr) -> decltype(base::endRight()) {
    return base::endRight();
  }
  /// map::end()
  template <typename T>
  static auto end() -> decltype(endImpl((T*)nullptr)) {
    return endImpl((T*)nullptr);
  }

  /// Implementation of find() for left-map
  static auto findImpl(const type1& item) -> decltype(base::findLeft(item)) {
    return base::findLeft(item);
  }
  /// Implementation of find() for right-map
  static auto findImpl(const type2& item) -> decltype(base::findRight(item)) {
    return base::findRight(item);
  }
  /// map::find()
  template <typename T>
  static auto find(const T& item) -> decltype(findImpl(item)) {
    return findImpl(item);
  }
};

/// BimapBase class for a pair of identical types, which will get derived further
// template <typename Type, typename CRTP>
// class BimapBase2<Type, Type, CRTP> : public std::string {};
//BimapBase1<Type, Type, CRTP> {};

/// Bimap class for a pair of identical types
template <typename Type1, typename Type2>
class Bimap : public BimapBase2<Type1, Type2, Bimap<Type1, Type2>> {
 public:
  typedef Type1 type1;
  typedef Type2 type2;
  typedef boost::bimap<type1, type2> bm_type;

 private:
  static const bm_type map_;
  template <typename Type3, typename Type4, typename CRTP2>
  friend class BimapBase2;
  template <typename Type3, typename Type4, typename CRTP2>
  friend class BimapBase1;
};

/// Bimap class for a pair of identical types
template <typename Type>
class Bimap<Type, Type> : public BimapBase2<Type, Type, Bimap<Type, Type>> {
 public:
  typedef Type type1;
  typedef Type type2;
  typedef boost::bimap<type1, type2> bm_type;

 private:
  static const bm_type map_;
  template <typename Type1, typename Type2, typename CRTP>
  friend class BimapBase2;
  template <typename Type1, typename Type2, typename CRTP>
  friend class BimapBase1;
};

template <typename TEnum>
class EnumStringMap : public BimapBase2<TEnum, std::string, EnumStringMap<TEnum>> {
 public:
  typedef BimapBase2<TEnum, std::string, EnumStringMap<TEnum>> base;
  typedef typename base::bm_type bm_type;
  typedef typename base::type1 type1;
  typedef typename base::type2 type2;

 private:
  static const bm_type map_;
  template <typename Type1, typename Type2, typename CRTP>
  friend class BimapBase2;
  template <typename Type1, typename Type2, typename CRTP>
  friend class BimapBase1;
};
template <>
class EnumStringMap<std::string> {};

#define IMPLEMENT_ENUMSTRINGMAP(ENUMSTRINGMAPTYPE)                                         \
  template <>                                                                              \
  const EnumStringMap<ENUMSTRINGMAPTYPE>::bm_type EnumStringMap<ENUMSTRINGMAPTYPE>::map_ = \
      boost::assign::list_of<EnumStringMap<ENUMSTRINGMAPTYPE>::bm_type::relation>

}  // namespace cloe
