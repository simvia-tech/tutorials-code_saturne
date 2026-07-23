/*============================================================================
 * Eul_Drift_Settling - drift-flux flags on the user scalars.
 *
 * PURPOSE OF THIS FILE
 * --------------------
 * The three user scalars defined in the GUI (c50, c150, c300) represent
 * concentrations of particle classes. The drift flux that makes them
 * settle relative to the carrier is NOT exposed in the GUI: the drift
 * flags can only be set here, and the associated relaxation time field
 * is filled in cs_user_physical_properties.cpp.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

/*----------------------------------------------------------------------------
 * Select physical model options, including user fields.
 *----------------------------------------------------------------------------*/

void
cs_user_model(void)
{
}

/*----------------------------------------------------------------------------
 * Define or modify general numerical and physical user parameters.
 *----------------------------------------------------------------------------*/

void
cs_user_parameters(cs_domain_t  *domain)
{
  CS_UNUSED(domain);

  /* Activate the drift flux on the three concentration scalars:
     each becomes a particle class settling at tau_p * g, where the
     relaxation time field <name>_drift_tau is filled in
     cs_user_physical_properties.cpp. */

  const int key_drift = cs_field_key_id("drift_scalar_model");

  /* ZERO_BNDY_FLUX cancels the drift flux through the boundaries:
     without it, the top wall acts as an infinite reservoir (the
     boundary drift flux keeps feeding the domain) and the settling
     columns grow instead of translating. Particles reaching the
     bottom accumulate there, as a sediment layer would. */

  const int drift = CS_DRIFT_SCALAR_ON + CS_DRIFT_SCALAR_ADD_DRIFT_FLUX
                  + CS_DRIFT_SCALAR_ZERO_BNDY_FLUX;

  const char *names[] = {"c50", "c150", "c300"};

  for (int i = 0; i < 3; i++) {
    cs_field_t *f = cs_field_by_name(names[i]);
    cs_field_set_key_int(f, key_drift, drift);
  }
}

END_C_DECLS
