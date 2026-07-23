/*============================================================================
 * Th_Stefan_Melting - one-phase Stefan problem with the CDO
 * solidification/melting module.
 *
 * PURPOSE OF THIS FILE
 * --------------------
 * The solidification module lives in the CDO framework and has no GUI
 * page: the whole physical setup is defined here.
 *
 *  - cs_user_zones: define the boundary zones from the mesh groups
 *    (in the CDO framework, zones are declared before being used).
 *
 *  - cs_user_model: switch the run to CDO-only mode, declare the
 *    boundaries, activate the solidification module with the STEFAN
 *    model (pure thermal phase change, sharp front: solidus equal to
 *    liquidus), and set its two parameters: the phase-change
 *    temperature and the latent heat.
 *
 *  - cs_user_finalize_setup: define the material properties (density,
 *    heat capacity, conductivity) and the thermal initial and
 *    boundary conditions.
 *
 * Configuration (one-phase melting):
 *   bar 0 < x < 0.1 m, initially solid at T_ini = -0.1, a hair below
 *   the fusion temperature T_m = 0 (starting EXACTLY at T_m would
 *   classify every cell as liquid from the first step and no front
 *   would ever exist: the 1% subcooling is numerically necessary and
 *   physically negligible); at t = 0 the left wall jumps to T_w = 10.
 *   rho = 1000, cp = 1000, lambda = 1 (SI) -> alpha = 1e-6 m2/s
 *   latent heat = 25000 J/kg -> Stefan number St = cp dT / L = 0.4
 *
 * Exact solution: front at x_f(t) = 2 lam sqrt(alpha t) with lam the
 * root of lam exp(lam^2) erf(lam) = St/sqrt(pi), here lam = 0.42124,
 * and an erf temperature profile in the liquid.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

/*----------------------------------------------------------------------------
 * Define volume and surface zones.
 *----------------------------------------------------------------------------*/

void
cs_user_zones(void)
{
  /* Boundary zones, from the cartesian-mesher groups (name = criteria) */

  cs_boundary_zone_define("X0", "X0", 0);
  cs_boundary_zone_define("X1", "X1", 0);
  cs_boundary_zone_define("lateral", "Y0 or Y1 or Z0 or Z1", 0);
}

/*----------------------------------------------------------------------------
 * Select physical model options, including user fields.
 *----------------------------------------------------------------------------*/

void
cs_user_model(void)
{
  cs_domain_t  *domain = cs_glob_domain;

  /* Declare the whole domain boundary as walls */

  cs_boundary_t  *bdy = domain->boundaries;

  cs_boundary_add(bdy, CS_BOUNDARY_WALL, "X0");
  cs_boundary_add(bdy, CS_BOUNDARY_WALL, "X1");
  cs_boundary_add(bdy, CS_BOUNDARY_WALL, "lateral");

  /* The solidification module belongs to the CDO framework */

  cs_param_cdo_mode_set(CS_PARAM_CDO_MODE_ONLY);

  /* Activate the solidification module, STEFAN model: pure thermal
     phase change, no fluid motion (the Navier-Stokes arguments are
     part of the generic signature) */

  cs_solidification_activate(CS_SOLIDIFICATION_MODEL_STEFAN,
                             0,      /* solidification options */
                             CS_SOLIDIFICATION_POST_ENTHALPY,
                             domain->boundaries,
                             CS_NAVSTO_MODEL_INCOMPRESSIBLE_NAVIER_STOKES,
                             0,      /* NavSto model flag */
                             CS_NAVSTO_COUPLING_MONOLITHIC,
                             0);     /* NavSto post flag */

  /* Stefan model parameters: phase-change temperature and latent heat */

  cs_solidification_set_stefan_model(0.,       /* t_change */
                                     25000.);  /* latent heat [J/kg] */

  /* Tighten the internal temperature/enthalpy coupling of the model
     (defaults: 15 iterations) */

  cs_solidification_stefan_t
    *stefan = cs_solidification_get_stefan_struct();

  stefan->n_iter_max = 50;
  stefan->max_delta_h = 1e-3 * 25000.;
}

/*----------------------------------------------------------------------------
 * Define or modify output user parameters.
 *----------------------------------------------------------------------------*/

void
cs_user_finalize_setup(cs_domain_t   *domain)
{
  CS_UNUSED(domain);

  /* Material properties (isotropic, whole domain) */

  cs_property_t  *rho = cs_property_by_name(CS_PROPERTY_MASS_DENSITY);
  cs_property_def_iso_by_value(rho, nullptr, 1000.);

  cs_property_t  *cp = cs_property_by_name(CS_THERMAL_CP_NAME);
  cs_property_def_iso_by_value(cp, nullptr, 1000.);

  cs_property_t  *lambda = cs_property_by_name(CS_THERMAL_LAMBDA_NAME);
  cs_property_def_iso_by_value(lambda, nullptr, 1.);

  /* Thermal equation: initial and boundary conditions.
     Default boundary condition is a homogeneous Neumann: only the two
     bar ends need a Dirichlet. */

  cs_equation_param_t  *th_eqp = cs_equation_param_by_name(CS_THERMAL_EQNAME);

  cs_real_t  t_ini = -0.1;  /* solid, a hair below fusion (see header) */
  cs_equation_add_ic_by_value(th_eqp, nullptr, &t_ini);

  static cs_real_t  t_hot = 10., t_cold = -0.1;
  cs_equation_add_bc_by_value(th_eqp, CS_BC_DIRICHLET, "X0", &t_hot);
  cs_equation_add_bc_by_value(th_eqp, CS_BC_DIRICHLET, "X1", &t_cold);
}

END_C_DECLS
