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
 * \file g1_fitting.cpp
 * \see  g1_fitting.hpp
 *
 */

#include <math.h>     // for M_PI, ..
#include <algorithm>  // for min, max
#include <cmath>      // for abs, sqrt
#include <sstream>    // for ostringstream
#include <stdexcept>  // for runtime_error
#include <vector>     // for vector

#ifndef CLOTHOID_ASSERT
#define CLOTHOID_ASSERT(COND, MSG)                                                  \
  if (!(COND)) {                                                                    \
    std::ostringstream ost;                                                         \
    ost << "On line: " << __LINE__ << " file: " << __FILE__ << '\n' << MSG << '\n'; \
    throw std::runtime_error(ost.str());                                            \
  }
#endif

namespace g1_fit {

static const double fn[] = {
    0.49999988085884732562,   1.3511177791210715095,   1.3175407836168659241,
    1.1861149300293854992,    0.7709627298888346769,   0.4173874338787963957,
    0.19044202705272903923,   0.06655998896627697537,  0.022789258616785717418,
    0.0040116689358507943804, 0.0012192036851249883877};

static const double fd[] = {1.0,
                            2.7022305772400260215,
                            4.2059268151438492767,
                            4.5221882840107715516,
                            3.7240352281630359588,
                            2.4589286254678152943,
                            1.3125491629443702962,
                            0.5997685720120932908,
                            0.20907680750378849485,
                            0.07159621634657901433,
                            0.012602969513793714191,
                            0.0038302423512931250065};

static const double gn[] = {
    0.50000014392706344801,    0.032346434925349128728,   0.17619325157863254363,
    0.038606273170706486252,   0.023693692309257725361,   0.007092018516845033662,
    0.0012492123212412087428,  0.00044023040894778468486, -8.80266827476172521e-6,
    -1.4033554916580018648e-8, 2.3509221782155474353e-10};

static const double gd[] = {1.0,
                            2.0646987497019598937,
                            2.9109311766948031235,
                            2.6561936751333032911,
                            2.0195563983177268073,
                            1.1167891129189363902,
                            0.57267874755973172715,
                            0.19408481169593070798,
                            0.07634808341431248904,
                            0.011573247407207865977,
                            0.0044099273693067311209,
                            -0.00009070958410429993314};

static const double TWO_PI = 2.0 * M_PI;
static const double ONE_SQRTPI = 0.5 * M_2_SQRTPI;

void calc_std_fresnel_integral(double y, double& int_c, double& int_s) {
  double const eps = 1E-15;
  double const x = std::abs(y);

  auto calc_int_series = [x, &int_c, &int_s](double f, double g) {
    // Refer to Sect. 4.7 in  http://doi.acm.org/10.1145/1916461.1916470.
    double u = M_PI_2 * (x * x);
    double sin_u = sin(u);
    double cos_u = cos(u);
    int_c = 0.5 + f * sin_u - g * cos_u;
    int_s = 0.5 - f * cos_u - g * sin_u;
  };

  if (x < 1.0) {
    double twofn, denterm, sum;

    auto calc_series_expansion_small_x = [x, eps, &twofn, &denterm](double sum_init) -> double {
      double const s = M_PI_2 * (x * x);
      double const t = -s * s;
      double term;
      double fact = 1.0;
      double numterm = 1.0;
      double series = sum_init;
      do {
        twofn += 2.0;
        fact *= twofn * (twofn - 1.0);
        denterm += 4.0;
        numterm *= t;
        term = numterm / (fact * denterm);
        series += term;
      } while (std::abs(term) > eps * std::abs(series));

      return series;
    };

    // Cosine integral series.
    twofn = 0.0;
    denterm = 1.0;
    sum = calc_series_expansion_small_x(1.0);

    int_c = x * sum;

    // Sine integral series.
    twofn = 1.0;
    denterm = 3.0;
    sum = calc_series_expansion_small_x(1.0 / 3.0);

    int_s = M_PI_2 * sum * (x * x * x);

  } else if (x < 6.0) {
    auto calc_rational_approx_f_g = [x](const double func_n[11],
                                        const double func_d[12]) -> double {
      double sumn = 0.0;
      double sumd = func_d[11];
      for (int k = 10; k >= 0; --k) {
        sumn = func_n[k] + x * sumn;
        sumd = func_d[k] + x * sumd;
      }
      return sumn / sumd;
    };

    // Rational approximation for f.
    double f = calc_rational_approx_f_g(fn, fd);

    // Rational approximation for g.
    double g = calc_rational_approx_f_g(gn, gd);

    // Compute int_c and int_s.
    calc_int_series(f, g);
  } else {
    // x >= 6: Asymptotic expansions for  f  and  g.

    auto calc_series_expansion_large_x = [x, eps](double sign) -> double {
      double const s = M_PI * x * x;
      double const t = -1 / (s * s);
      double series = 1.0;
      double numterm = -1.0;
      double term = 1.0;
      double oldterm = 1.0;
      double eps10 = 0.1 * eps;
      double absterm;
      do {
        numterm += 4.0;
        term *= numterm * (numterm + (sign * 2.0)) * t;
        series += term;
        absterm = std::abs(term);
        CLOTHOID_ASSERT(oldterm >= absterm,
                        "In calc_std_fresnel_integral f/g not converged to eps, x = "
                            << x << " oldterm = " << oldterm << " absterm = " << absterm);
        oldterm = absterm;
      } while (absterm > eps10 * std::abs(series));
      return series;
    };

    // Expansion for f.
    double sum = calc_series_expansion_large_x(-1.0);
    double f = sum / (M_PI * x);

    //  Expansion for  g.
    sum = calc_series_expansion_large_x(1.0);
    double g = M_PI * x;
    g = sum / (g * g * x);

    // Compute int_c and int_s.
    calc_int_series(f, g);
  }
  if (y < 0) {
    int_c = -int_c;
    int_s = -int_s;
  }
}

/**
 * Compute moments of Fresnel integrals:
 *   c_k(t) = \int_0^t s^k * cos( (pi/2)*s^2 ) ds
 *   s_k(t) = \int_0^t s^k * sin( (pi/2)*s^2 ) ds
 *
 * Refer to Eq. 15 in https://arxiv.org/pdf/1209.0910.pdf.
 *
 * \param n_k: Number of moments to be computed.
 * \param t: See equation above.
 * \param c_k: Array of Fresnel cosine moments [c_0,c_1,...,c_{n_k-1}]
 * \param s_k: Array of Fresnel sine moments [s_0,s_1,...,s_{n_k-1}]
 */
void calc_std_fresnel_integral_moments(int n_k, double t, double c_k[], double s_k[]) {
  calc_std_fresnel_integral(t, c_k[0], s_k[0]);
  if (n_k > 1) {
    // Eq. 16 f.
    double tt = M_PI_2 * (t * t);
    double ss = sin(tt);
    double cc = cos(tt);
    c_k[1] = ss * M_1_PI;
    s_k[1] = (1 - cc) * M_1_PI;
    if (n_k > 2) {
      c_k[2] = (t * ss - s_k[0]) * M_1_PI;
      s_k[2] = (c_k[0] - t * cc) * M_1_PI;
    }
  }
}

/**
 * Compute moments of the following integrals for large a:
 *   x_k(a,b,0) = \int_0^1 t^k * cos( (a/2)*t^2 + b*t) dt
 *   y_k(a,b,0) = \int_0^1 t^k * sin( (a/2)*t^2 + b*t) dt
 *
 * Refer to Eqs. 20, 21, 22 in https://arxiv.org/pdf/1209.0910.pdf.
 *
 * \param n_k: Number of moments to be computed.
 * \param a: See equation above.
 * \param b: See equation above.
 * \param x_k: Array of cosine moments [x_0,x_1,...,x_{n_k-1}]
 * \param y_k: Array of sine moments [y_0,y_1,...,y_{n_k-1}]
 */
static void calc_integral_moments_a_large(int n_k, double a, double b, double x_k[], double y_k[]) {
  CLOTHOID_ASSERT(
      n_k < 4 && n_k > 0,
      "In calc_integral_moments_a_large first argument n_k must be in [1,3], n_k =" << n_k);

  // See Eq. 18 ff in https://arxiv.org/pdf/1209.0910.pdf.
  double s = a > 0 ? +1 : -1;
  double abs_a = std::abs(a);
  double z = ONE_SQRTPI * sqrt(abs_a);
  double l = s * b * ONE_SQRTPI / sqrt(abs_a);
  double gam = -0.5 * s * (b * b) / abs_a;
  double cg = cos(gam) / z;
  double sg = sin(gam) / z;

  // Evaluate Eq. 19.
  double Cl[3], Sl[3], Cz[3], Sz[3];
  calc_std_fresnel_integral_moments(n_k, l, Cl, Sl);
  calc_std_fresnel_integral_moments(n_k, l + z, Cz, Sz);

  double dC0 = Cz[0] - Cl[0];
  double dS0 = Sz[0] - Sl[0];
  // Evaluate Eq. 22.
  x_k[0] = cg * dC0 - s * sg * dS0;
  y_k[0] = sg * dC0 + s * cg * dS0;
  // The following implementation for k=1 and k=2 was verified against the recursive Eqs. 20 and 21.
  if (n_k > 1) {
    cg /= z;
    sg /= z;
    double dC1 = Cz[1] - Cl[1];
    double dS1 = Sz[1] - Sl[1];
    double DC = dC1 - l * dC0;
    double DS = dS1 - l * dS0;
    x_k[1] = cg * DC - s * sg * DS;
    y_k[1] = sg * DC + s * cg * DS;
    if (n_k > 2) {
      double dC2 = Cz[2] - Cl[2];
      double dS2 = Sz[2] - Sl[2];
      DC = dC2 + l * (l * dC0 - 2 * dC1);
      DS = dS2 + l * (l * dS0 - 2 * dS1);
      cg = cg / z;
      sg = sg / z;
      x_k[2] = cg * DC - s * sg * DS;
      y_k[2] = sg * DC + s * cg * DS;
    }
  }
}

/**
 * Compute Lommel function expansion:
 *   w(mu,nu,b) = \sum_0^\inf (-b^2)^n / alpha(n+1,mu,nu), where
 *     alpha(n,mu,nu) = \prod_1^n (mu + 2*m - 1)^2 - nu^2
 *
 * Refer to Eq. 27 ff in https://arxiv.org/pdf/1209.0910.pdf.
 *
 * \param mu: See equation above.
 * \param nu: See equation above.
 * \param b: See equation above.
 */
double calc_lommel_term(double mu, double nu, double b) {
  double tmp = 1 / ((mu + nu + 1) * (mu - nu + 1));  // alpha(1,mu,nu)
  double res = tmp;
  for (int n = 1; n <= 100; ++n) {
    // Modification TF: It seems that one should be subtracted, not added as in the original code:
    // https://github.com/ebertolazzi/G1fitting/blob/master/G1fitting/GeneralizedFresnelCS.m#L120
    // The modification follows from the 2nd formula in Eq. 27 after applying a binomial expansion formula.
    // After the modification, alpha(1,mu,nu) above makes sense.
    tmp *= (-b / (2 * n + mu - nu - 1)) * (b / (2 * n + mu + nu - 1));
    res += tmp;
    if (std::abs(tmp) < std::abs(res) * 1e-50) {
      break;
    }
  }
  return res;
}

/**
 * Compute moments of the following integrals for zero a:
 *   x_k(0,b,0) = \int_0^1 t^k * cos(b*t) dt = ( sin(b) - k*y_(k-1)(0,b,0) ) / b
 *   y_k(0,b,0) = \int_0^1 t^k * sin(b*t) dt = ( k*x_(k-1)(0,b,0) - cos(b) ) / b
 *
 * Refer to Eq. 25 in https://arxiv.org/pdf/1209.0910.pdf.
 *
 * \param n_k: Number of moments to be computed.
 * \param b: See equation above.
 * \param x_k: Array of cosine moments [x_0,x_1,...,x_{n_k-1}]
 * \param y_k: Array of sine moments [y_0,y_1,...,y_{n_k-1}]
 */
static void calc_integral_moments_a_zero(int n_k,
                                         double b,
                                         std::vector<double>& x_k,
                                         std::vector<double>& y_k) {
  double sin_b = sin(b);
  double cos_b = cos(b);
  double b_sq = b * b;
  // Compute starting point, see Eq. 25 f in https://arxiv.org/pdf/1209.0910.pdf.
  if (std::abs(b) < 1e-3) {
    x_k[0] = 1 - (b_sq / 6) * (1 - (b_sq / 20) * (1 - (b_sq / 42)));
    y_k[0] = (b / 2) * (1 - (b_sq / 12) * (1 - (b_sq / 30)));
  } else {
    x_k[0] = sin_b / b;
    y_k[0] = (1 - cos_b) / b;
  }
  // Use recurrence Eq. 25 for the stable part.
  int m = std::min(n_k, std::max(1, int(floor(2 * b))));
  for (int k = 1; k < m; ++k) {
    x_k[k] = (sin_b - k * y_k[k - 1]) / b;
    y_k[k] = (k * x_k[k - 1] - cos_b) / b;
  }
  // Use Lommel function for the unstable part, Eq. 27 ff.
  if (m < n_k) {
    double A = b * sin_b;
    double D = sin_b - b * cos_b;
    double B = b * D;
    double C = -b_sq * sin_b;
    // Why is this evaluated with m instead of k?? Shouldn't both terms be evaluated inside the following loop?
    // https://github.com/ebertolazzi/G1fitting/blob/master/G1fitting/GeneralizedFresnelCS.m#L155
    double rLa = calc_lommel_term(m + 0.5, 1.5, b);
    double rLd = calc_lommel_term(m + 0.5, 0.5, b);
    for (int k = m; k < n_k; ++k) {
      double rLb = calc_lommel_term(k + 1.5, 0.5, b);
      double rLc = calc_lommel_term(k + 1.5, 1.5, b);
      x_k[k] = (k * A * rLa + B * rLb + cos_b) / (1 + k);
      y_k[k] = (C * rLc + sin_b) / (2 + k) + D * rLd;
      rLa = rLc;
      rLd = rLb;
    }
  }
}

/**
 * Compute moments of the following integrals for small a:
 *   x_k(a,b,0) = \int_0^1 t^k * cos( (a/2)*t^2 + b*t) dt
 *   y_k(a,b,0) = \int_0^1 t^k * sin( (a/2)*t^2 + b*t) dt
 *
 * Refer to Eqs. 23, 24 in https://arxiv.org/pdf/1209.0910.pdf.
 *
 * \param n_k: Number of moments to be computed.
 * \param a: See equation above.
 * \param b: See equation above.
 * \param p: Number of terms in series expansion [1,10].
 * \param x_k: Array of cosine moments [x_0,x_1,...,x_{n_k-1}]
 * \param y_k: Array of sine moments [y_0,y_1,...,y_{n_k-1}]
 */
static void calc_integral_moments_a_small(
    int n_k, double a, double b, int p, double x_k[], double y_k[]) {
  CLOTHOID_ASSERT(p < 11 && p > 0, "In evalXYaSmall p = " << p << " must be in [1,10]");
  // x_k(0,b) and y_k(0,b) must be evaluated up to k=(4*p+2+n_k), see Eqs. 23 and 24.
  int nk0 = 4 * p + 2 + n_k;
  std::vector<double> x0(nk0), y0(nk0);
  calc_integral_moments_a_zero(nk0, b, x0, y0);

  // Compute n=0 terms.
  for (int k = 0; k < n_k; ++k) {
    x_k[k] = x0[k] - (a / 2) * y0[k + 2];
    y_k[k] = y0[k] + (a / 2) * x0[k + 2];
  }

  double t = 1;
  double aa = -a * a / 4;
  // Compute other series terms..
  for (int n = 1; n <= p; ++n) {
    t *= aa / (2 * n * (2 * n - 1));  // (-1)^n / ((2*n)!) * (a/2)^(2*n)
    double bf = a / (4 * n + 2);
    // ..for each moment k.
    for (int k = 0; k < n_k; ++k) {
      int jj = 4 * n + k;
      x_k[k] += t * (x0[jj] - bf * y0[jj + 2]);
      y_k[k] += t * (y0[jj] + bf * x0[jj + 2]);
    }
  }
}

/**
 * Compute moments of Fresnel integrals:
 *   x_k(a,b,c) = \int_0^1 t^k * cos( (a/2)*t^2 + b*t + c ) dt
 *   y_k(a,b,c) = \int_0^1 t^k * sin( (a/2)*t^2 + b*t + c ) dt
 *
 * Refer to Eq. 17 in https://arxiv.org/pdf/1209.0910.pdf.
 *
 * \param n_k: Number of moments to be computed.
 * \param a: See equation above.
 * \param b: See equation above.
 * \param c: See equation above.
 * \param x_k: Array of Fresnel cosine moments [x_0,x_1,...,x_{n_k-1}]
 * \param y_k: Array of Fresnel sine moments [y_0,y_1,...,y_{n_k-1}]
 */
void calc_gen_fresnel_integral_moments(
    int n_k, double a, double b, double c, double x_k[], double y_k[]) {
  CLOTHOID_ASSERT(n_k > 0 && n_k < 4, "n_k = " << n_k << " must be in [1,3]");

  // Threshold from https://github.com/ebertolazzi/G1fitting/blob/master/G1fitting/GeneralizedFresnelCS.m#L32.
  const double a_thresh = 0.01;

  // Evaluate x_k(a,b) and y_k(a,b), see Eq. 18 ff in https://arxiv.org/pdf/1209.0910.pdf.
  if (std::abs(a) < a_thresh) {
    const int n_terms = 3;
    calc_integral_moments_a_small(n_k, a, b, n_terms, x_k, y_k);
  } else {
    calc_integral_moments_a_large(n_k, a, b, x_k, y_k);
  }

  // Evaluate x_k(a,b,c) and y_k(a,b,c).
  double cos_c = cos(c);
  double sin_c = sin(c);
  for (int k = 0; k < n_k; ++k) {
    double xx = x_k[k];
    double yy = y_k[k];
    x_k[k] = xx * cos_c - yy * sin_c;
    y_k[k] = xx * sin_c + yy * cos_c;
  }
}

/**
 * Normalize angle to range [-M_PI, M_PI].
 *
 * \param x: Arbitrary angle [rad].
 */
inline double normalize_abs_pi(double x) {
  double xn = std::fmod(x + M_PI, TWO_PI);
  if (xn < 0.0) {
    xn += TWO_PI;
  }
  return xn - M_PI;
}

/**
 * Find guess for roots of function g(A).
 *
 * Inputs: Normalized angle used in the clothoid fitting problem.
 *   \param phi0: At clothoid start.
 *   \param phi1: At clothoid end.
 */
double calc_initial_guess(double phi0, double phi1) {
  static double const CF[] = {2.989696028701907,  0.716228953608281, -0.458969738821509,
                              -0.502821153340377, 0.261062141752652, -0.045854475238709};
  double x = phi0 * M_1_PI;
  double y = phi1 * M_1_PI;
  double xy = x * y;
  y *= y;
  x *= x;
  return (phi0 + phi1) * (CF[0] + xy * (CF[1] + xy * CF[2]) + (CF[3] + xy * CF[4]) * (x + y) +
                          CF[5] * (x * x + y * y));
}

/**
 * Find root of function g(A) defined as
 *   g(A) = \int_0^1 \sin( A*t^2+(delta-A)*t+phi0 ) dt
 *
 * \param a_guess: Initial guess.
 * \param delta: Angle used in the clothoid fitting problem.
 * \param phi0: Angle used in the clothoid fitting problem.
 * \param niter_max: Allowed number of Newton iterations.
 * \param tol: Max. allowed residual.
 */
double find_root(double a_guess, double delta, double phi0, int niter_max, double tol) {
  double a = a_guess;
  double g = 0;
  double dg, x_k[3], y_k[3];
  int niter = 0;
  do {
    // Compute Fresnel integral moments.
    calc_gen_fresnel_integral_moments(3, 2 * a, delta - a, phi0, x_k, y_k);
    // g(a) is the first moment, see Eq. 9 in https://arxiv.org/pdf/1209.0910.pdf.
    g = y_k[0];
    dg = x_k[2] - x_k[1];
    a -= g / dg;
  } while (++niter <= niter_max && std::abs(g) > tol);
  CLOTHOID_ASSERT(niter <= niter_max, "Newton do not converge, g = " << g << " niter = " << niter);
  return a;
}

/**
 * Calculate clothoid length for a given root.
 *
 * \param a: Root of g(A) = \int_0^1 \sin( A*t^2+(delta-A)*t+phi0 ) dt.
 * \param delta: Angle used in the clothoid fitting problem.
 * \param phi0: Angle used in the clothoid fitting problem.
 * \param r: Distance between clothoid start and end point.
 */
double calc_clothoid_length(double a, double delta, double phi0, double r) {
  double h_a[1], g_a[1];
  // Compute h(a), see Eq. 10 in https://arxiv.org/pdf/1209.0910.pdf.
  calc_gen_fresnel_integral_moments(1, 2 * a, delta - a, phi0, h_a, g_a);
  // See Eq. 12 in https://arxiv.org/pdf/1209.0910.pdf.
  double l = r / h_a[0];
  CLOTHOID_ASSERT(l > 0, "Negative length l = " << l);
  return l;
}

void calc_clothoid(double x0,
                   double y0,
                   double theta0,
                   double x1,
                   double y1,
                   double theta1,
                   double& k,
                   double& dk,
                   double& l) {
  const double dx = x1 - x0;
  const double dy = y1 - y0;
  const double r = hypot(dx, dy);
  const double phi = atan2(dy, dx);

  const double phi0 = normalize_abs_pi(theta0 - phi);
  const double phi1 = normalize_abs_pi(theta1 - phi);

  const double delta = phi1 - phi0;

  // Initial guess.
  double a = calc_initial_guess(phi0, phi1);

  // Newton solver.
  int niter_max = 10;
  double tol = 1e-12;
  a = find_root(a, delta, phi0, niter_max, tol);

  // Compute clothoid parameters for the final root result.
  l = calc_clothoid_length(a, delta, phi0, r);
  // See Eq. 12 in https://arxiv.org/pdf/1209.0910.pdf.
  k = (delta - a) / l;
  dk = 2 * a / l / l;
}

}  // namespace g1_fit
