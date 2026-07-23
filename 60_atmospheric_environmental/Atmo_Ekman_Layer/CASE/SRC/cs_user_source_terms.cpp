/*============================================================================
 * Atmo_Ekman_Layer - geostrophic forcing.
 *
 * The large-scale pressure gradient is imposed as a constant body force. In
 * geostrophic balance with the Coriolis force it fixes the geostrophic wind:
 * a force (0, rho*f*G, 0) gives an interior wind U_g = (G, 0, 0). Near the
 * rough ground friction unbalances the geostrophic balance and the wind veers
 * with height (the Ekman spiral).
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

void
cs_user_source_terms(cs_domain_t  *domain,
                     int           f_id,
                     cs_real_t    *st_exp,
                     cs_real_t    *st_imp)
{
  CS_NO_WARN_IF_UNUSED(st_imp);

  const cs_field_t  *f = cs_field_by_id(f_id);
  if (f != CS_F_(vel))
    return;

  const cs_real_t  *cell_vol = domain->mesh_quantities->cell_vol;

  /* Geostrophic pressure-gradient force: (0, rho*f*G, 0),
     rho = 1.225, f = 1.0312e-4 s^-1, G = 10 m/s -> U_g = (10, 0, 0) */
  const cs_real_t  fy = 1.225 * 1.0312e-4 * 10.0;

  cs_real_t  (*_st_exp)[3] = (cs_real_t (*)[3])st_exp;

  for (cs_lnum_t c_id = 0; c_id < domain->mesh->n_cells; c_id++)
    _st_exp[c_id][1] += fy * cell_vol[c_id];
}

END_C_DECLS
