/*============================================================================
 * Gwf_Darcy_Tracer - CDO groundwater flow model (saturated, single phase).
 *
 * A saturated soil block is driven by two imposed hydraulic heads (left > right).
 * The Richards equation (steady, pure diffusion div(K grad h) = 0) then fixes a
 * uniform Darcy flux q = -K grad h. A passive tracer, injected at the inlet, is
 * convected by q and spread by mechanical dispersion, giving an advection-
 * dispersion front validated against the Ogata-Banks analytic solution.
 *
 * Everything specific to the groundwater module is set here in C: the legacy
 * GUI groundwater path is not wired to the solver in v9.1, the CDO module
 * (cs_gwf_*) is the supported route. The setup.xml only holds the mesh, the
 * time stepping and the output.
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

  /* Boundaries: everything is no-flow (symmetry) by default; the two x-faces
     carry the imposed heads (inlet = high head, outlet = low head). */

  cs_boundary_t  *bdy = domain->boundaries;
  cs_boundary_set_default(bdy, CS_BOUNDARY_SYMMETRY);
  cs_boundary_add(bdy, CS_BOUNDARY_INLET,  "left");
  cs_boundary_add(bdy, CS_BOUNDARY_OUTLET, "right");

  /* Activate CDO and the saturated single-phase groundwater flow model.
     Post-processing: write the Darcy flux balance and its boundary values. */

  cs_param_cdo_mode_set(CS_PARAM_CDO_MODE_ONLY);

  cs_gwf_activate(CS_GWF_MODEL_SATURATED_SINGLE_PHASE,
                  0,                                   /* no physical option  */
                  CS_GWF_POST_DARCY_FLUX_BALANCE |
                  CS_GWF_POST_DARCY_FLUX_AT_BOUNDARY);

  /* Saturated isotropic soil over the whole domain.
       K       : hydraulic conductivity [m/s] (diffusion of the Richards eq.)
       theta_s : porosity (saturated water content)
       rho_b   : bulk density (only used with sorption; kept for completeness) */

  const double  K       = 1.0e-4;
  const double  theta_s = 0.4;
  const double  rho_b   = 1500.0;

  cs_gwf_add_iso_soil("cells", rho_b, K, theta_s, CS_GWF_SOIL_SATURATED);

  /* Passive tracer convected by the Darcy flux, with mechanical dispersion.
       molecular diffusivity : 0 (dispersion dominated)
       alpha_L, alpha_T      : longitudinal / transverse dispersivity [m]
       kd                    : distribution coefficient (0 = no sorption)   */

  cs_gwf_tracer_t  *tr = cs_gwf_add_tracer(0, "tracer", "C");
  cs_gwf_tracer_set_soil_param(tr,
                               nullptr,     /* apply to all soils */
                               0.0,         /* molecular diffusivity */
                               1.0, 0.1,    /* alpha_L, alpha_T */
                               0.0);        /* kd */
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

  /* Tracer: unit concentration injected at the inlet.
     The initial condition is zero everywhere (default). */

  cs_equation_param_t  *t_eqp = cs_equation_param_by_name("tracer");
  cs_real_t  c_in = 1.0;
  cs_equation_add_bc_by_value(t_eqp, CS_BC_DIRICHLET, "left", &c_in);
}

END_C_DECLS
