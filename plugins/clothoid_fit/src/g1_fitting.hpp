/*
 * Copyright 2023 Robert Bosch GmbH
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
/*
 * The present code was adapted from
 * https://github.com/ebertolazzi/G1fitting/blob/master/src/Clothoid.cc
 *
 * Copyright (c) 2016, Enrico Bertolazzi
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in
 *      the documentation and/or other materials provided with the distribution
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \file g1_fitting.hpp
 *
 */

namespace g1_fit {

/**
 * Compute clothoid parameters from a start and an endpoint. The clothoid curve
 * is defined as
 *   x(s) = x0 + \int_0^s cos( 0.5*dk*t^2 + k*t + theta0 ) dt
 *   y(s) = y0 + \int_0^s sin( 0.5*dk*t^2 + k*t + theta0 ) dt
 *
 * Refer to Eq. 1 in https://arxiv.org/pdf/1209.0910.pdf.
 *
 * \param x0: x(0) in equation above [m].
 * \param y0: y(0) in equation above [m].
 * \param theta0: Curve angle at s=0 [rad], theta(s)= 0.5*dk*t^2 + k*t + theta0.
 * \param x1: x(L) in equation above [m].
 * \param y1: y(L) in equation above [m].
 * \param theta1: Curve angle at s=L [rad].
 * \param k: Curvature at s=0 [1/m], see equation above.
 * \param dk: Curvature change [1/m^2], see equation above.
 * \param L: Clothoid length [m] from (x0, y0) to (x1, y1).
 */
void calc_clothoid(double x0,
                   double y0,
                   double theta0,
                   double x1,
                   double y1,
                   double theta1,
                   double& k,
                   double& dk,
                   double& L);

/**
 * Compute standard Fresnel integrals:
 *   c(y) = \int_0^y cos( (pi/2)*x^2 ) dx
 *   s(y) = \int_0^y sin( (pi/2)*x^2 ) dx
 *
 * Refer to Eq. 14 in https://arxiv.org/pdf/1209.0910.pdf.
 *
 * The present algorithm is described in
 *   Thompson, W. J., 1997. Atlas for Computing Mathematical Functions: An Illustrated Guide for Practitioners with
 *   Programs in C and Mathematica with CDrom, 1st Edition. John Wiley & Sons, Inc., New York, NY, USA,
 *
 * using the modifications proposed by
 *   Venkata Sivakanth Telasula (2023). Fresnel Cosine and Sine Integral Function
 *   (https://www.mathworks.com/matlabcentral/fileexchange/9017-fresnel-cosine-and-sine-integral-function), MATLAB Central File Exchange.
 *
 * \param y: See equation above.
 * \param int_c: Fresnel cosine integral.
 * \param int_s: Fresnel sine integral.
 */
void calc_std_fresnel_integral(double y, double& int_c, double& int_s);

}  // namespace g1_fit
