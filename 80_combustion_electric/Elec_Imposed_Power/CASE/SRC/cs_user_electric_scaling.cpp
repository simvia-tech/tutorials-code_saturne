/*============================================================================
 * Elec_Imposed_Power - user correction of the potential scaling.
 *
 * PURPOSE OF THIS FILE
 * --------------------
 * With the imposed-power option (ielcor = 1), the built-in Joule
 * rescaling multiplies the potential by the POWER ratio
 * P_imposed / P_computed. Since P scales as U^2, that fixed point is
 * unstable: the iteration cycles endlessly between two values (period
 * two) instead of converging. This hook, called right after the
 * built-in scaling, applies the square-root factor that brings the
 * dissipated power exactly onto the imposed one: the potential
 * equation being linear, the scaling then converges immediately.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

void
cs_user_scaling_elec(const cs_mesh_t             *mesh,
                     const cs_mesh_quantities_t  *mesh_quantities,
                     cs_real_t                   *dt)
{
  CS_UNUSED(dt);

  cs_elec_option_t *elec_opt = cs_get_glob_elec_option();

  if (elec_opt->ielcor != 1)
    return;

  const cs_lnum_t n_cells = mesh->n_cells;
  const cs_real_t *volume = mesh_quantities->cell_vol;

  /* Dissipated power after the built-in rescaling of this time step */

  double somje = 0.;
  cs_real_t *cpro_joulp = CS_F_(joulp)->val;
  for (cs_lnum_t c_id = 0; c_id < n_cells; c_id++)
    somje += cpro_joulp[c_id] * volume[c_id];
  cs_parall_sum(1, CS_DOUBLE, &somje);

  if (somje < 1e-20)
    return;

  /* Square-root correction: after applying k, the power equals the
     imposed one exactly. Mild clamping for startup robustness. */

  double k = sqrt(elec_opt->puisim / somje);
  k = cs::min(cs::max(k, 0.1), 10.);

  elec_opt->coejou   *= k;
  elec_opt->pot_diff *= k;

  cs_real_t *cvar_potr = CS_F_(potr)->val;
  for (cs_lnum_t c_id = 0; c_id < n_cells; c_id++) {
    cvar_potr[c_id] *= k;
    cpro_joulp[c_id] *= k*k;
  }
}

END_C_DECLS
