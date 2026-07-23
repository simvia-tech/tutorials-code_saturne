/*============================================================================
 * Inc_LES_Channel - constant streamwise momentum source term.
 *
 * Same driving as the RANS channel: a constant body force per unit volume
 * f_x = rho * u_tau^2 / h is added to the x-momentum equation. It balances the
 * wall friction at statistical steady state and fixes the friction Reynolds
 * number by construction. Non-dimensional here (rho = u_tau = h = 1) so f_x = 1.
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
  CS_UNUSED(st_imp);

  const cs_field_t  *f = cs_field_by_id(f_id);
  if (f != CS_F_(vel))
    return;

  const cs_lnum_t   n_cells = domain->mesh->n_cells;
  const cs_real_t  *cell_vol = domain->mesh_quantities->cell_vol;
  const cs_real_t   f_x = 1.0;   /* rho u_tau^2 / h, wall units */

  cs_real_3_t  *_st_exp = (cs_real_3_t *)st_exp;

  for (cs_lnum_t c_id = 0; c_id < n_cells; c_id++)
    _st_exp[c_id][0] += f_x * cell_vol[c_id];
}

END_C_DECLS
