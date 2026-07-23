/*============================================================================
 * Gwf_Sorption_Retardation - CDO groundwater flow with a sorbing tracer.
 *
 * Same saturated Darcy flow as Gwf_Darcy_Tracer, but two tracers are injected
 * at the inlet at once:
 *   - "conservative" : no sorption (kd = 0), moves at the pore velocity v
 *   - "sorbing"      : linear sorption (kd > 0), retarded by a factor
 *                        R = 1 + rho_b * kd / theta_s
 * The sorbing front therefore lags the conservative one by exactly R. Both are
 * validated against the (retarded) Ogata-Banks advection-dispersion solution.
 *
 * With rho_b = 2000 kg/m3, kd = 2e-4 m3/kg and theta_s = 0.4, R = 2: the
 * sorbing front travels at half the speed of the conservative one.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

/*----------------------------------------------------------------------------
 * Select physical model options (called before the mesh is read).
 *----------------------------------------------------------------------------*/

void
cs_user_model(void)
{
  cs_domain_t  *domain = cs_glob_domain;

  /* Boundaries: no-flow (symmetry) by default; the two x-faces carry the
     imposed hydraulic heads (inlet = high head, outlet = low head). */

  cs_boundary_t  *bdy = domain->boundaries;
  cs_boundary_set_default(bdy, CS_BOUNDARY_SYMMETRY);
  cs_boundary_add(bdy, CS_BOUNDARY_INLET,  "left");
  cs_boundary_add(bdy, CS_BOUNDARY_OUTLET, "right");

  /* Activate CDO and the saturated single-phase groundwater flow model. */

  cs_param_cdo_mode_set(CS_PARAM_CDO_MODE_ONLY);

  cs_gwf_activate(CS_GWF_MODEL_SATURATED_SINGLE_PHASE,
                  0,
                  CS_GWF_POST_DARCY_FLUX_BALANCE);

  /* Saturated isotropic soil over the whole domain.
       K       : hydraulic conductivity [m/s]
       theta_s : porosity
       rho_b   : bulk density [kg/m3] (drives the sorption retardation) */

  const double  K       = 1.0e-4;
  const double  theta_s = 0.4;
  const double  rho_b   = 2000.0;

  cs_gwf_add_iso_soil("cells", rho_b, K, theta_s, CS_GWF_SOIL_SATURATED);

  /* Two tracers, both convected by the Darcy flux with the same dispersion.
     They differ only by the distribution coefficient kd (last argument):
       - conservative : kd = 0        -> R = 1
       - sorbing      : kd = 2e-4     -> R = 1 + rho_b*kd/theta_s = 2 */

  cs_gwf_tracer_t  *tc = cs_gwf_add_tracer(0, "conservative", "Cc");
  cs_gwf_tracer_set_soil_param(tc, nullptr, 0.0, 1.0, 0.1, 0.0);

  cs_gwf_tracer_t  *ts = cs_gwf_add_tracer(0, "sorbing", "Cs");
  cs_gwf_tracer_set_soil_param(ts, nullptr, 0.0, 1.0, 0.1, 2.0e-4);
}

/*----------------------------------------------------------------------------
 * Finalize the setup: boundary conditions and initial condition.
 *----------------------------------------------------------------------------*/

void
cs_user_finalize_setup([[maybe_unused]] cs_domain_t  *domain)
{
  /* Richards equation: imposed hydraulic heads (drive the Darcy flux). */

  cs_equation_param_t  *r_eqp = cs_equation_param_by_name("Richards");
  cs_real_t  h_left = 1.0, h_right = 0.0;
  cs_equation_add_bc_by_value(r_eqp, CS_BC_DIRICHLET, "left",  &h_left);
  cs_equation_add_bc_by_value(r_eqp, CS_BC_DIRICHLET, "right", &h_right);

  /* Both tracers: unit concentration injected at the inlet (IC = 0). */

  cs_real_t  c_in = 1.0;
  cs_equation_add_bc_by_value(cs_equation_param_by_name("conservative"),
                              CS_BC_DIRICHLET, "left", &c_in);
  cs_equation_add_bc_by_value(cs_equation_param_by_name("sorbing"),
                              CS_BC_DIRICHLET, "left", &c_in);
}

END_C_DECLS
