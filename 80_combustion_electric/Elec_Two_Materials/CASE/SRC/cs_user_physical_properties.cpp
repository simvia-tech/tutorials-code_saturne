/*============================================================================
 * Elec_Two_Materials - spatially variable electrical conductivity.
 *
 * PURPOSE OF THIS FILE
 * --------------------
 * The bar is made of two materials in series: sigma is a FIELD, and
 * this is the whole point of the case. The same loop accommodates any
 * spatial or temperature-dependent law (glass furnaces combine both:
 * electrode regions and strongly sigma(T) melts).
 *
 * As in Elec_Joule_Bar, the temperature-enthalpy conversion of the
 * Joule variant is also up to the user (T = H/cp with constant cp).
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

void
cs_user_physical_properties(cs_domain_t  *domain)
{
  const cs_lnum_t n_cells = domain->mesh->n_cells;
  const cs_real_3_t *cell_cen = domain->mesh_quantities->cell_cen;

  /* Two materials in series: sigma_1 for x < x_i, sigma_2 beyond. */

  const cs_real_t x_i     = 0.05;   /* material interface [m] */
  const cs_real_t sigma_1 = 100.;   /* [S/m] */
  const cs_real_t sigma_2 = 25.;    /* [S/m] */

  const int kivisl = cs_field_key_id("diffusivity_id");
  const cs_field_t *f_pot = cs_field_by_name("elec_pot_r");
  const int diff_id = cs_field_get_key_int(f_pot, kivisl);

  if (diff_id > -1) {
    cs_real_t *cpro_sigma = cs_field_by_id(diff_id)->val;
    for (cs_lnum_t c_id = 0; c_id < n_cells; c_id++)
      cpro_sigma[c_id] = (cell_cen[c_id][0] < x_i) ? sigma_1 : sigma_2;
  }

  /* Temperature from enthalpy (constant cp law, see Elec_Joule_Bar). */

  const cs_real_t cp0 = cs_glob_fluid_properties->cp0;
  const cs_real_t *cvar_h = cs_field_by_name("enthalpy")->val;
  cs_real_t *cpro_t = cs_field_by_name("temperature")->val;

  for (cs_lnum_t c_id = 0; c_id < n_cells; c_id++)
    cpro_t[c_id] = cvar_h[c_id] / cp0;
}

END_C_DECLS
