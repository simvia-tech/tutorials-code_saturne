/*============================================================================
 * Ctwr_Crossflow_Rain - initial humid-air state.
 *
 * The domain starts with cool, fairly dry air at rest-ish (a small upward
 * velocity), matching the inlet. The packing then cools the injected water and
 * warms/humidifies the air toward the steady state.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

void
cs_user_initialization(cs_domain_t  *domain)
{
  if (domain->time_step->nt_prev > 0)
    return;

  const cs_lnum_t  n_cells = domain->mesh->n_cells;

  cs_real_3_t *vel = (cs_real_3_t *)CS_F_(vel)->val;
  cs_real_t   *cvar_t = cs_thermal_model_field()->val;
  cs_field_t  *f_yw = cs_field_by_name_try("ym_water");

  for (cs_lnum_t c_id = 0; c_id < n_cells; c_id++) {
    vel[c_id][0] = 0.0;
    vel[c_id][1] = 0.0;
    vel[c_id][2] = 0.8;
    cvar_t[c_id] = 15.0;                      /* degC */
    if (f_yw != nullptr) f_yw->val[c_id] = 0.006;
  }
}

END_C_DECLS
