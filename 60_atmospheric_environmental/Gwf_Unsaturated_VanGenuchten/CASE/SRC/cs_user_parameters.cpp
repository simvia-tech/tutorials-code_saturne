/*============================================================================
 * Gwf_Unsaturated_VanGenuchten - variably saturated flow (Richards + VGM).
 *
 * A vertical soil column, initially saturated, drains toward hydrostatic
 * equilibrium above a fixed water table. The unsaturated single-phase model
 * solves the nonlinear Richards equation with the Van Genuchten-Mualem
 * constitutive laws:
 *
 *     S_e(h) = [1 + |alpha h|^n]^(-m),   m = 1 - 1/n     (h < 0)
 *     theta  = theta_r + S_e (theta_s - theta_r)
 *     K(h)   = K_s S_e^L [1 - (1 - S_e^(1/m))^m]^2       (Mualem)
 *
 * with h the pressure head (h = H - z under gravity). At equilibrium the flux
 * vanishes, so H = const = 0 and h = -z: the moisture profile theta(z) then
 * follows the Van Genuchten retention curve, the reference used for validation.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

/*----------------------------------------------------------------------------
 * Initial hydraulic head: H = y (elevation), i.e. pressure head h = H - y = 0,
 * so the column starts fully saturated.
 *----------------------------------------------------------------------------*/

static void
_init_head(cs_real_t          time,
           cs_lnum_t          n_elts,
           const cs_lnum_t   *pt_ids,
           const cs_real_t   *xyz,
           bool               dense_output,
           [[maybe_unused]] void *input,
           cs_real_t         *retval)
{
  CS_NO_WARN_IF_UNUSED(time);
  for (cs_lnum_t i = 0; i < n_elts; i++) {
    const cs_lnum_t  id = (pt_ids == nullptr) ? i : pt_ids[i];
    const cs_lnum_t  r_id = (dense_output) ? i : id;
    retval[r_id] = xyz[3*id + 1];      /* y coordinate = elevation */
  }
}

/*----------------------------------------------------------------------------
 * Select physical model options (called before the mesh is read).
 *----------------------------------------------------------------------------*/

void
cs_user_model(void)
{
  cs_domain_t  *domain = cs_glob_domain;

  /* Elevation direction for the groundwater module. The pressure head is built
     as h = H - (x . g), so g is the unit vector along which the elevation head
     is measured (pointing up). Here the column is vertical along +y, so g =
     (0, 1, 0): at equilibrium H = 0 gives h = -y (suction increasing upward). */

  cs_physical_constants_t  *pc = cs_get_glob_physical_constants();
  pc->gravity[0] = 0.0; pc->gravity[1] = 1.0; pc->gravity[2] = 0.0;

  /* Boundaries: no-flow (symmetry) everywhere except the bottom water table. */

  cs_boundary_t  *bdy = domain->boundaries;
  cs_boundary_set_default(bdy, CS_BOUNDARY_SYMMETRY);
  cs_boundary_add(bdy, CS_BOUNDARY_INLET, "bottom");

  /* Activate CDO and the unsaturated single-phase groundwater flow model,
     with gravity. */

  cs_param_cdo_mode_set(CS_PARAM_CDO_MODE_ONLY);

  cs_gwf_activate(CS_GWF_MODEL_UNSATURATED_SINGLE_PHASE,
                  CS_GWF_GRAVITATION,
                  CS_GWF_POST_PERMEABILITY | CS_GWF_POST_DARCY_FLUX_BALANCE);

  /* Van Genuchten-Mualem soil (a silt-loam type soil).
       K_s     : saturated conductivity [m/s]
       theta_s : saturated water content (porosity)
       theta_r : residual water content
       alpha   : Van Genuchten scaling parameter [1/m]
       n       : Van Genuchten shape parameter
       L       : Mualem tortuosity */

  const double  Ks = 2.9e-6, theta_s = 0.43, rho_b = 1500.0;
  cs_gwf_soil_t  *soil = cs_gwf_add_iso_soil("cells", rho_b, Ks, theta_s,
                                             CS_GWF_SOIL_VGM_SINGLE_PHASE);
  cs_gwf_soil_set_vgm_spf_param(soil,
                                0.078,   /* theta_r */
                                3.6,     /* alpha [1/m] */
                                1.56,    /* n */
                                0.5);    /* L */
}

/*----------------------------------------------------------------------------
 * Finalize the setup: boundary and initial conditions.
 *----------------------------------------------------------------------------*/

void
cs_user_finalize_setup([[maybe_unused]] cs_domain_t  *domain)
{
  cs_equation_param_t  *r_eqp = cs_equation_param_by_name("Richards");

  /* Water table at the bottom: hydraulic head H = 0. */

  cs_real_t  h_bot = 0.0;
  cs_equation_add_bc_by_value(r_eqp, CS_BC_DIRICHLET, "bottom", &h_bot);

  /* Initially saturated column: H = y (pressure head = 0 everywhere). */

  cs_equation_add_ic_by_analytic(r_eqp, "cells", _init_head, nullptr);
}

END_C_DECLS
