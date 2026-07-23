/*============================================================================
 * Atmo_Neutral_BoundaryLayer - constant streamwise body force.
 *
 * The neutral surface layer is driven by a constant horizontal pressure
 * gradient, imposed here as a body force f_x = rho u*^2 / H on the x-momentum.
 * At equilibrium it balances the ground shear stress, fixing the friction
 * velocity u* = sqrt(f_x H / rho) by construction (as in the turbulent channel).
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

  /* f_x = rho u*^2 / H  with rho = 1.225, u* = 0.4 m/s, H = 1000 m */
  const cs_real_t  fx = 1.225 * 0.4 * 0.4 / 1000.0;

  cs_real_t  (*_st_exp)[3] = (cs_real_t (*)[3])st_exp;

  for (cs_lnum_t c_id = 0; c_id < domain->mesh->n_cells; c_id++)
    _st_exp[c_id][0] += fx * cell_vol[c_id];
}

END_C_DECLS
