/*============================================================================
 * Inc_Rough_Wall_Channel - constant streamwise momentum source term.
 *
 * PURPOSE OF THIS FILE
 * --------------------
 * The channel is periodic in the streamwise direction x, so there is no inlet
 * or outlet and no mean pressure gradient in the boundary conditions. The
 * driving is supplied here as a constant body force per unit volume added to
 * the x-momentum equation:
 *
 *     f_x = rho * u_tau^2 / h
 *
 * At statistical steady state this force is exactly balanced by the wall
 * friction on the two walls, so it fixes the friction velocity by
 * construction, whatever the wall roughness. This is what makes the roughness
 * study of this tutorial straightforward: u_tau is the same in every run, the
 * bulk velocity is the output, and the drop in bulk velocity between two runs
 * is directly the roughness function Delta U+.
 *
 * With the non-dimensional choice of this case (rho = 1, u_tau = 1, half-height
 * h = 1, nu = 1/Re_tau), the force is simply f_x = 1.
 *
 * The explicit source term expected by the solver is the force integrated over
 * each cell volume, st_exp[c][0] = f_x * cell_volume[c].
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

/*----------------------------------------------------------------------------*/

void
cs_user_source_terms(cs_domain_t  *domain,
                     int           f_id,
                     cs_real_t    *st_exp,
                     cs_real_t    *st_imp)
{
  CS_UNUSED(st_imp);

  const cs_field_t  *f = cs_field_by_id(f_id);

  /* Only act on the momentum (velocity) equation */
  if (f != CS_F_(vel))
    return;

  const cs_lnum_t   n_cells = domain->mesh->n_cells;
  const cs_real_t  *cell_vol = domain->mesh_quantities->cell_vol;

  /* Constant streamwise body force per unit volume: f_x = rho u_tau^2 / h.
     Non-dimensional here (rho = 1, u_tau = 1, h = 1) -> f_x = 1. */
  const cs_real_t  f_x = 1.0;

  cs_real_3_t  *_st_exp = (cs_real_3_t *)st_exp;

  for (cs_lnum_t c_id = 0; c_id < n_cells; c_id++)
    _st_exp[c_id][0] += f_x * cell_vol[c_id];
}

END_C_DECLS
