/*============================================================================
 * Air-helium gas mix in the compressible module.
 *
 * The gas mix model has no GUI panel: it is activated here, together with
 * the matching equation of state and the variable specific heat it needs.
 *============================================================================*/

#include "cs_headers.h"

BEGIN_C_DECLS

/*----------------------------------------------------------------------------*/
/*!
 * \brief Select physical model options, including user fields.
 */
/*----------------------------------------------------------------------------*/

void
cs_user_model(void)
{
  /* Air/helium gas mix: the mass fractions of O2 and N2 are solved,
     the helium mass fraction is deduced (y_he = 1 - y_o2 - y_n2). */

  cs_glob_physical_model_flag[CS_GAS_MIX] = CS_GAS_MIX_AIR_HELIUM;

  /* Ideal gas mix equation of state of the compressible module
     (overrides the "constant gamma" choice of the GUI). */

  cs_get_glob_cf_model()->ieos = CS_EOS_GAS_MIX;

  /* The specific heat of the mixture varies with its composition: it must
     be a variable property. This has to be set here, before the property
     fields are created. */

  cs_get_glob_fluid_properties()->icp = 0;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Define or modify general numerical and physical user parameters.
 *
 * \param[in, out] domain    pointer to a cs_domain_t structure
 */
/*----------------------------------------------------------------------------*/

void
cs_user_parameters([[maybe_unused]] cs_domain_t   *domain)
{
  /* Workaround for v9.1: the gas mix model creates a property field "tempk"
     and maps the Kelvin temperature pointer to it, overriding the mapping of
     the compressible module to its solved "temperature" variable. The gas mix
     property update then aborts when it queries the diffusivity of that
     property. The mapping to the solved temperature is restored here. */

  cs_field_pointer_map(CS_ENUMF_(t_kelvin), cs_field_by_name("temperature"));
}

END_C_DECLS
