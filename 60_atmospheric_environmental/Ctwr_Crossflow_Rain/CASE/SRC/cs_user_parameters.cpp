/*============================================================================
 * Ctwr_Crossflow_Rain - activate the cooling tower model and set air props.
 *
 * The cooling tower (ctwr) module models a humid-air flow exchanging heat and
 * mass with injected water in a packing (fill) zone: evaporative cooling. The
 * model is activated here; the humid-air properties are those of the reference
 * ctwr example. The exchange zone is defined in cs_user_zones.cpp.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

void
cs_user_model(void)
{
  /* Activate the cooling tower model with Poppe's evaporation model. */
  cs_glob_physical_model_flag[CS_COOLING_TOWERS] = 1;
  cs_ctwr_option_t *ct_opt = cs_get_glob_ctwr_option();
  ct_opt->evap_model = CS_CTWR_POPPE;
}

void
cs_user_parameters([[maybe_unused]] cs_domain_t  *domain)
{
  /* Dilatable (variable-density) algorithm and variable density. */
  cs_get_glob_velocity_pressure_model()->idilat = 2;
  cs_get_glob_fluid_properties()->irovar = 1;

  /* Humid-air properties (reference ctwr values). */
  cs_fluid_properties_t *fp = cs_get_glob_fluid_properties();
  fp->ro0    = 1.2;
  fp->viscl0 = 1.765e-05;

  cs_air_fluid_props_t *air = cs_glob_air_props;
  air->cp_a    = 1006.0;         /* dry air heat capacity */
  air->cp_v    = 1831.0;         /* water vapour heat capacity */
  air->cp_l    = 4179.0;         /* liquid water heat capacity */
  air->hv0     = 2501600.0;      /* latent heat of vaporization */
  air->rho_l   = 997.85615;      /* liquid water density */
  air->lambda_h = 2.493;
  air->lambda_l = 0.02493;
  air->humidity0 = 5.626e-03;    /* reference humidity */
  air->droplet_diam = 0.005;
}

END_C_DECLS
