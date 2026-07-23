/*============================================================================
 * User functions (and method definitions).
 *
 * CavitatingThrottle test case — cavitation (Merkle) model parameters.
 *
 * PURPOSE OF THIS FILE
 * --------------------
 * The cavitation model parameters of code_saturne are NOT exposed in the
 * GUI: they can only be set here. Without this file, the reference velocity
 * `uinf` keeps an unset sentinel value and the Merkle source terms are
 * numerically ZERO: the run proceeds but no vapor is ever created.
 * This file is therefore MANDATORY for any cavitation computation.
 *============================================================================*/

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

  /* Cavitation (Merkle) model parameters
     ------------------------------------ */

  cs_cavitation_parameters_t *cavit_param
    = cs_get_glob_cavitation_parameters();

  /* Reference saturation pressure [Pa].
   *
   * IMPORTANT: it is compared to the TOTAL pressure, which includes the
   * reference pressure P0 set in the GUI (101325 Pa here), i.e. presat is
   * an ABSOLUTE pressure.
   * Water: 2340 Pa at 20 degC, 7380 Pa at 40 degC, 101325 Pa at 100 degC.
   * Adjust to the temperature of YOUR experiment. */

  cavit_param->presat = 2340.;

  /* Reference velocity and length scales of the flow [m/s], [m].
   *
   * They calibrate the vaporization/condensation time scale of the Merkle
   * model (t_inf = linf/uinf) and its source-term prefactors.
   * From the code_saturne user guide: "considering the cavitating flow
   * across a foil in a duct, the reference velocity should be the bulk
   * velocity and the reference length scale should be the chord of the
   * foil". Transposed to this throttle: bulk velocity in the throat
   * (~21 m/s) and the throat length (2 mm), i.e. the streamwise extent
   * of the cavitating restriction.
   *
   * If you do not set uinf, it keeps a huge negative sentinel value and
   * the model produces NOTHING (this is the single most common cavitation
   * setup mistake). */

  cavit_param->uinf = 21.;
  cavit_param->linf = 2.e-3;

  /* Empirical constants of the Merkle model (defaults: cdest = 50,
   * cprod = 10000). Keep the defaults unless you have a good reason. */

  /* cavit_param->cdest = 5.e1;  */
  /* cavit_param->cprod = 1.e4;  */
}

END_C_DECLS
