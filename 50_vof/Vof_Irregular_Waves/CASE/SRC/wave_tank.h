#ifndef WAVE_TANK_H
#define WAVE_TANK_H

/*============================================================================
 * Wave tank: sea state shared by the user routines.
 *
 * The free-surface elevation is a sum of linear (Airy) components,
 *
 *   eta(x, t) = sum_j a_j cos(k_j x - omega_j t + phi_j)
 *
 * whose amplitudes follow a JONSWAP spectrum (Hs, Tp, gamma) and whose
 * wavenumbers satisfy the dispersion relation omega^2 = g k tanh(k h).
 * The sea state is read from the notebook of setup.xml, so that it can be
 * changed from the GUI.
 *============================================================================*/

#include "cs_headers.h"

#include <cmath>
#include <random>
#include <vector>

namespace wave_tank {

/* Tank layout (m); must match the mesh of setup.xml */

constexpr double beach_start = 9.0;   /* the absorbing beach starts here */
constexpr double tank_length = 15.0;  /* and ends at the end wall */
constexpr double tank_height = 0.8;   /* bottom to lid */
constexpr double beach_coef  = 8.0;   /* peak damping rate (1/s) */

/* Submerged breakwater (m), carved out of the mesh if "structure" is 1 in
   the notebook; its sides must lie on cell faces */

constexpr double structure_x0 = 6.0, structure_x1 = 6.9;
constexpr double structure_height = 0.35;
constexpr double tank_width = 0.02;   /* thickness of the mesh */

constexpr double gauges[] = {2.0, 5.0, 8.0};  /* wave gauge abscissas */
constexpr int    n_gauges = sizeof(gauges)/sizeof(gauges[0]);

constexpr double rho_water = 1000., rho_air = 1.;  /* as in setup.xml */
constexpr double g = 9.81;

/* One linear wave component */

struct component_t {
  double omega;  /* angular frequency (rad/s) */
  double a;      /* amplitude (m) */
  double k;      /* wavenumber (rad/m) */
  double phi;    /* phase (rad) */
};

/* Sea state (built once, at the first call) */

struct sea_state_t {
  double h;                       /* still water depth (m) */
  double t_ramp;                  /* duration of the start-up ramp (s) */
  std::vector<component_t> c;     /* wave components */
};

/* Wavenumber from the dispersion relation, by Newton iterations */

inline double
wavenumber(double omega, double h)
{
  double k = omega*omega/g;  /* deep-water guess */
  for (int it = 0; it < 50; it++) {
    const double th = std::tanh(k*h);
    const double f  = g*k*th - omega*omega;
    const double df = g*th + g*k*h*(1. - th*th);
    k -= f/df;
  }
  return k;
}

inline const sea_state_t &
sea_state()
{
  static sea_state_t s;
  if (!s.c.empty())
    return s;

  const double hs    = cs_notebook_parameter_value_by_name("Hs");
  const double tp    = cs_notebook_parameter_value_by_name("Tp");
  const double gamma = cs_notebook_parameter_value_by_name("gamma");
  const bool regular = cs_notebook_parameter_value_by_name("regular") > 0.5;

  s.h      = cs_notebook_parameter_value_by_name("depth");
  s.t_ramp = 2.*tp;

  const double pi = cs_math_pi;

  if (regular) {
    /* A single wave of height Hs and period Tp */
    const double omega = 2.*pi/tp;
    s.c.push_back({omega, 0.5*hs, wavenumber(omega, s.h), 0.});
  }
  else {
    /* JONSWAP spectrum sampled on [0.5 fp, 2 fp] with n equal bins: the
       signal repeats after 1/df = n Tp/1.5, longer than the simulation. */
    const int n = 100;
    const double fp = 1./tp, df = 1.5*fp/n;

    std::vector<double> sj(n);
    double m0 = 0.;
    for (int j = 0; j < n; j++) {
      const double f = 0.5*fp + (j + 0.5)*df;
      const double sigma = (f <= fp) ? 0.07 : 0.09;
      const double r = std::exp(-0.5*cs_math_pow2((f - fp)/(sigma*fp)));
      sj[j] =   std::pow(f, -5.) * std::exp(-1.25*std::pow(fp/f, 4.))
              * std::pow(gamma, r);
      m0 += sj[j]*df;
    }
    const double scale = cs_math_pow2(hs/4.)/m0;  /* Hs = 4 sqrt(m0) */

    std::mt19937 rng(2026);  /* fixed seed: the sea state is reproducible */
    for (int j = 0; j < n; j++) {
      const double f = 0.5*fp + (j + 0.5)*df;
      const double omega = 2.*pi*f;
      const double a = std::sqrt(2.*scale*sj[j]*df);
      const double phi = 2.*pi*(rng()/4294967296.);
      s.c.push_back({omega, a, wavenumber(omega, s.h), phi});
    }
  }

  /* Keep a record of the components next to the results */

  if (cs_glob_rank_id <= 0) {
    FILE *f = fopen("wave_components.csv", "w");
    fprintf(f, "omega,amplitude,wavenumber,phase\n");
    for (const component_t &w : s.c)
      fprintf(f, "%.12g,%.12g,%.12g,%.12g\n", w.omega, w.a, w.k, w.phi);
    fclose(f);
  }

  return s;
}

/* Start-up ramp, from 0 to 1 over t_ramp */

inline double
ramp(double t)
{
  const sea_state_t &s = sea_state();
  if (t >= s.t_ramp)
    return 1.;
  return 0.5*(1. - std::cos(cs_math_pi*t/s.t_ramp));
}

} /* namespace wave_tank */

#endif /* WAVE_TANK_H */
