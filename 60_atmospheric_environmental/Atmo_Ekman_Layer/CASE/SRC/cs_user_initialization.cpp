/*============================================================================
 * Atmo_Ekman_Layer - initial fields.
 *
 * The flow is started from the geostrophic wind (uniform, along x) with a small
 * surface-layer turbulence, and relaxes to the Ekman spiral under Coriolis and
 * ground friction. Vertical direction: z.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

void
cs_user_initialization(cs_domain_t  *domain)
{
  cs_real_3_t  *vel = (cs_real_3_t *)CS_F_(vel)->val;
  cs_real_t    *cvar_k = CS_F_(k)->val;
  cs_real_t    *cvar_eps = CS_F_(eps)->val;

  const cs_real_t  G = 10.0;                 /* geostrophic wind [m/s] */
  const cs_real_t  k0 = 0.1, eps0 = 1.0e-3;

  for (cs_lnum_t c_id = 0; c_id < domain->mesh->n_cells; c_id++) {
    vel[c_id][0] = G;
    vel[c_id][1] = 0.0;
    vel[c_id][2] = 0.0;
    cvar_k[c_id]   = k0;
    cvar_eps[c_id] = eps0;
  }
}

END_C_DECLS
