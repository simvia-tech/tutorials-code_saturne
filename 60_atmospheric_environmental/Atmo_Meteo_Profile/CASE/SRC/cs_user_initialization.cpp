/*============================================================================
 * Atmo_Meteo_Profile - initial fields consistent with the meteo profile.
 *
 * The domain is initialized with the logarithmic wind profile of the meteo
 * file (u* = 0.4 m/s, z0 = 0.1 m), uniform potential temperature (neutral) and
 * the matching surface-layer turbulence. The meteo inlet then sustains the
 * profile through the domain. Vertical direction: z.
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
  cs_field_t   *th = cs_thermal_model_field();   /* potential temperature */

  const cs_real_t  ustar = 0.8, z0 = 0.1, theta_ref = 288.15;
  const cs_real_t  kappa = cs_turb_xkappa, cmu = cs_turb_cmu;

  for (cs_lnum_t c_id = 0; c_id < domain->mesh->n_cells; c_id++) {
    const cs_real_t  z = cell_cen[c_id][2];      /* z is the vertical */
    vel[c_id][0] = (ustar/kappa) * log((z + z0)/z0);
    vel[c_id][1] = 0.0;
    vel[c_id][2] = 0.0;
    cvar_k[c_id]   = ustar*ustar / sqrt(cmu);
    cvar_eps[c_id] = ustar*ustar*ustar / (kappa * (z + z0));
    if (th != nullptr)
      th->val[c_id] = theta_ref;
  }
}

END_C_DECLS
