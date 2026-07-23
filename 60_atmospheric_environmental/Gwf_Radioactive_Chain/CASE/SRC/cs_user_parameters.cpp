/*============================================================================
 * Gwf_Radioactive_Chain - CDO groundwater flow with a radioactive decay chain.
 *
 * Same saturated Darcy flow as Gwf_Darcy_Tracer. A three-member radioactive
 * decay chain is injected at the inlet:
 *
 *     grandfather --(lambda_A)--> father --(lambda_B)--> son (stable)
 *
 * Each species is a tracer transported by the Darcy flux; a first-order decay
 * reaction couples them (parent loss = daughter gain, in moles). With a small
 * dispersivity the flow is advection dominated, so a water parcel ages by its
 * travel time tau = x / v as it moves downstream. The steady-state spatial
 * profiles therefore reproduce the Bateman solution mapped through tau = x/v:
 * the grandfather decays, the father grows then decays (ingrowth), the son
 * accumulates. Validated against the Bateman equations.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

/* Decay constants [1/s]: grandfather, father, son (stable) */
static const double  lambda_A = 1.0e-7;
static const double  lambda_B = 4.0e-8;
static const double  lambda_C = 0.0;

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

  /* Saturated isotropic soil over the whole domain. */

  const double  K       = 1.0e-4;
  const double  theta_s = 0.4;
  const double  rho_b   = 2000.0;   /* unused here (no sorption) */

  cs_gwf_add_iso_soil("cells", rho_b, K, theta_s, CS_GWF_SOIL_SATURATED);

  /* Three-member radioactive decay chain (moles). Each decay is 1:1, the son
     is stable, so grandfather + father + son is conserved along the flow. */

  const int  n = 3;
  const char *names[3] = {"grandfather", "father", "son"};
  cs_gwf_tracer_model_t  models[3] = {0, 0, 0};
  double  lambda[3] = {lambda_A, lambda_B, lambda_C};

  cs_gwf_tracer_decay_chain_t  *tdc =
    cs_gwf_add_decay_chain(n, CS_GWF_TRACER_UNIT_MOLE, "chain",
                           names, models, lambda);

  /* Same transport parameters for every species: no molecular diffusion, a
     small dispersivity (advection dominated), no sorption. */

  for (int i = 0; i < tdc->n_tracers; i++)
    cs_gwf_tracer_set_soil_param(tdc->tracers[i], nullptr,
                                 0.0,          /* molecular diffusivity */
                                 0.5, 0.05,    /* alpha_L, alpha_T */
                                 0.0);         /* kd */
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

  /* Chain inlet: pure grandfather (=1), no daughters yet (=0), so the spatial
     profiles start from the Bateman initial condition at tau = 0. */

  cs_gwf_tracer_decay_chain_t  *tdc = cs_gwf_tracer_decay_chain_by_name("chain");
  cs_real_t  one = 1.0, zero = 0.0;

  cs_equation_add_bc_by_value(cs_gwf_tracer_decay_chain_get_equation_param(tdc, 0),
                              CS_BC_DIRICHLET, "left", &one);
  cs_equation_add_bc_by_value(cs_gwf_tracer_decay_chain_get_equation_param(tdc, 1),
                              CS_BC_DIRICHLET, "left", &zero);
  cs_equation_add_bc_by_value(cs_gwf_tracer_decay_chain_get_equation_param(tdc, 2),
                              CS_BC_DIRICHLET, "left", &zero);
}

END_C_DECLS
