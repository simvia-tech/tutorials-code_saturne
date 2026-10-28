/*============================================================================
 * Liquid water as a stiffened gas in the compressible module.
 *
 * The GUI only offers the constant-gamma ideal gas: the stiffened gas law
 * and its constants are set here. Everything else (mesh, initial state,
 * walls, probes) is defined in setup.xml.
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
  /* Single-phase stiffened gas: p = (gamma - 1) rho e - gamma pinf */

  cs_get_glob_cf_model()->ieos = CS_EOS_STIFFENED_GAS;
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
  /* Constants fitted so that water at p0 = 1 bar and T0 = 300 K has
     rho0 = 1000 kg/m3 and c = 1500 m/s with cv = 4180 J/kg/K:
       c^2 = gamma (gamma - 1) cv T0      ->  gamma = 1.9311
       pinf = rho0 (gamma - 1) cv T0 - p0  ->  pinf = 1.16753e9 Pa
     The sound speed of the law is c = sqrt(gamma (p + pinf) / rho). */

  cs_cf_model_t *cf_model = cs_get_glob_cf_model();
  cf_model->gammasg = 1.9311;
  cf_model->psginf  = 1.16753e9;

  /* For the stiffened gas, cv0 is not deduced from cp0 and the molar mass
     as for an ideal gas: it must be given here, with cp0 = gamma cv0. */

  cs_fluid_properties_t *fp = cs_get_glob_fluid_properties();
  fp->cv0 = 4180.;
  fp->cp0 = 1.9311*4180.;
}

END_C_DECLS
