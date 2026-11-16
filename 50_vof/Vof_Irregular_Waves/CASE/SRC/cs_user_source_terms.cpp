/*============================================================================
 * Absorbing beach: momentum sink in front of the end wall.
 *
 * For x > beach_start the momentum equation receives S = -rho lambda(x) u,
 * with a damping rate growing as the square of the distance into the
 * beach, so that the waves enter it without being reflected by its front.
 * The term is implicit (unconditionally stable) and acts on the velocity
 * only: the void fraction is transported freely, no water is created.
 *============================================================================*/

#include "cs_headers.h"

#include "wave_tank.h"

void
cs_user_source_terms(cs_domain_t  *domain,
                     int           f_id,
                     [[maybe_unused]] cs_real_t  *st_exp,
                     cs_real_t    *st_imp)
{
  if (cs_field_by_id(f_id) != CS_F_(vel))
    return;

  const cs_real_3_t *cell_cen = domain->mesh_quantities->cell_cen;
  const cs_real_t *cell_vol = domain->mesh_quantities->cell_vol;
  const cs_real_t *rho = CS_F_(rho)->val;

  cs_real_33_t *imp = (cs_real_33_t *)st_imp;

  const double l_beach = wave_tank::tank_length - wave_tank::beach_start;

  for (cs_lnum_t c_id = 0; c_id < domain->mesh->n_cells; c_id++) {
    const double xi = (cell_cen[c_id][0] - wave_tank::beach_start)/l_beach;
    if (xi <= 0.)
      continue;
    const double lambda = wave_tank::beach_coef*xi*xi;
    for (int i = 0; i < 3; i++)
      imp[c_id][i][i] -= cell_vol[c_id]*rho[c_id]*lambda;
  }
}
