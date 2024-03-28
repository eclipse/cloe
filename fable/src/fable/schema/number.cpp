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
 * \file fable/schema/number_impl.cpp
 * \see  fable/schema/number_impl.hpp
 * \see  fable/schema/number.hpp
 *
 * This file provides the expected instantiations for the Number<T> schema.
 * This should improve compilation time significantly.
 *
 * If the type you want to use is not here, you can simply include
 * number_impl.hpp header.
 */

#include <fable/schema/number_impl.hpp>

namespace fable::schema {

template class Number<char>;
template class Number<signed char>;
template class Number<unsigned char>;
template class Number<signed short>;
template class Number<unsigned short>;
template class Number<signed int>;
template class Number<unsigned int>;
template class Number<signed long int>;
template class Number<unsigned long int>;
template class Number<signed long long int>;
template class Number<unsigned long long int>;
template class Number<float>;
template class Number<double>;
template class Number<long double>;

}  // namespace fable::schema
