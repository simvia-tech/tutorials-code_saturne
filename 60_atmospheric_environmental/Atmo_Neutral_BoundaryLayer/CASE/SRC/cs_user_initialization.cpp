/*============================================================================
 * Atmo_Neutral_BoundaryLayer - initialize a log-law surface layer.
 *
 * The equilibrium solution is a logarithmic wind profile. Starting the k-epsilon
 * fields close to it (log velocity, constant-stress-layer k and epsilon) speeds
 * up convergence toward the steady state. Wall units: u* = 0.4 m/s, z0 = 0.1 m.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

void
cs_user_initialization(cs_domain_t  *domain)
{
  const cs_real_3_t  *cell_cen
    = (const cs_real_3_t *)domain->mesh_quantities->cell_cen;

  cs_real_3_t  *vel = (cs_real_3_t *)CS_F_(vel)->val;
  cs_real_t    *cvar_k = CS_F_(k)->val;
  cs_real_t    *cvar_eps = CS_F_(eps)->val;

  const cs_real_t  ustar = 0.4;
  const cs_real_t  z0 = 0.1;
  const cs_real_t  kappa = cs_turb_xkappa;   /* = 0.42 */
  const cs_real_t  cmu = cs_turb_cmu;        /* = 0.09 */

  for (cs_lnum_t c_id = 0; c_id < domain->mesh->n_cells; c_id++) {

    const cs_real_t  z = cell_cen[c_id][1];        /* y is the vertical */

    vel[c_id][0] = (ustar/kappa) * log((z + z0)/z0);
    vel[c_id][1] = 0.0;
    vel[c_id][2] = 0.0;

    /* Constant-flux surface layer: k = u*^2/sqrt(Cmu), eps = u*^3/(kappa z) */
    cvar_k[c_id]   = ustar*ustar / sqrt(cmu);
    cvar_eps[c_id] = ustar*ustar*ustar / (kappa * (z + z0));
  }
}

END_C_DECLS
