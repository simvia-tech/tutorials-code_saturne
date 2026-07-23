/*============================================================================
 * Elec_Joule_Bar - electrical conductivity of the bar.
 *
 * PURPOSE OF THIS FILE
 * --------------------
 * For the Joule variant of the electric module, the electrical
 * conductivity sigma is the diffusivity of the electric potential and
 * is NOT set through the GUI: it must be filled here (it is a field,
 * so any sigma(T) law fits; glass furnaces use strongly temperature-
 * dependent laws). This case uses a constant conductivity so the
 * solution has a closed form.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

void
cs_user_physical_properties(cs_domain_t  *domain)
{
  const cs_lnum_t n_cells = domain->mesh->n_cells;

  /* Electrical conductivity of the bar [S/m]: diffusivity field of
     the real electric potential. */

  const cs_real_t sigma = 100.;

  const int kivisl = cs_field_key_id("diffusivity_id");
  const cs_field_t *f_pot = cs_field_by_name("elec_pot_r");
  const int diff_id = cs_field_get_key_int(f_pot, kivisl);

  if (diff_id > -1) {
    cs_real_t *cpro_sigma = cs_field_by_id(diff_id)->val;
    for (cs_lnum_t c_id = 0; c_id < n_cells; c_id++)
      cpro_sigma[c_id] = sigma;
  }

  /* Temperature from enthalpy.

     For the Joule variant, the enthalpy-temperature conversion is also
     up to the user (the module only performs it for the electric arc
     model, from its tabulated plasma properties). With a constant
     specific heat the law is simply T = H / cp. */

  const cs_real_t cp0 = cs_glob_fluid_properties->cp0;
  const cs_real_t *cvar_h = cs_field_by_name("enthalpy")->val;
  cs_real_t *cpro_t = cs_field_by_name("temperature")->val;

  for (cs_lnum_t c_id = 0; c_id < n_cells; c_id++)
    cpro_t[c_id] = cvar_h[c_id] / cp0;
}

END_C_DECLS
