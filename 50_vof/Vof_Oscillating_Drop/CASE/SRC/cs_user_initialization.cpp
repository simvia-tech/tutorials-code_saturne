/*============================================================================
 * Vof_Oscillating_Drop - initialize an elliptic drop.
 *
 * PURPOSE OF THIS FILE
 * --------------------
 * The void fraction is initialized as an ellipse: a drop of radius
 * R = 0.02 m deformed by epsilon = 8% (semi-axes R(1+eps), R(1-eps)),
 * centered in the box. Surface tension then drives the mode-2
 * Rayleigh oscillation whose exact angular frequency (2D, inviscid)
 * is
 *
 *   omega^2 = 6 sigma / ((rho_in + rho_out) R^3)
 *
 * i.e. a period T = 0.761 s with the parameters of this case.
 * The interface is smoothed over about three cells (tanh profile) to
 * avoid a stair-cased start.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

/*----------------------------------------------------------------------------
 * Initialize variables.
 *----------------------------------------------------------------------------*/

void
cs_user_initialization(cs_domain_t  *domain)
{
  const cs_mesh_t  *m = domain->mesh;
  const cs_real_3_t  *cell_cen
    = (const cs_real_3_t *)domain->mesh_quantities->cell_cen;

  const cs_real_t  xc = 0.05, yc = 0.05;   /* drop center */
  const cs_real_t  radius = 0.02;          /* mean radius */
  const cs_real_t  eps = 0.08;             /* mode-2 deformation */
  const cs_real_t  a = radius*(1. + eps);  /* semi-axis along x */
  const cs_real_t  b = radius*(1. - eps);  /* semi-axis along y */
  const cs_real_t  delta = 1.2e-3;         /* interface smoothing (~3 cells) */

  cs_real_t  *cvar_voidf = CS_F_(void_f)->val;

  for (cs_lnum_t c_id = 0; c_id < m->n_cells; c_id++) {

    const cs_real_t  dx = cell_cen[c_id][0] - xc;
    const cs_real_t  dy = cell_cen[c_id][1] - yc;

    /* Signed distance-like level set of the ellipse (approximate) */

    const cs_real_t  r_ell = sqrt(dx*dx/(a*a) + dy*dy/(b*b));
    const cs_real_t  d = (r_ell - 1.) * radius;

    /* Drop: void fraction = 1 inside, 0 outside, tanh transition */

    cvar_voidf[c_id] = 0.5 * (1. - tanh(d/delta));

  }
}

END_C_DECLS
