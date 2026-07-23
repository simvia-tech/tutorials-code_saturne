/*============================================================================
 * Rayleigh bubble collapse - cavitation (Merkle) model parameters.
 *
 * Same mandatory pattern as the cavitating throttle: the cavitation
 * model parameters are not exposed in the GUI.
 *============================================================================*/

#include "cs_headers.h"

BEGIN_C_DECLS

void
cs_user_model(void)
{
}

void
cs_user_parameters(cs_domain_t  *domain)
{
  CS_UNUSED(domain);

  cs_cavitation_parameters_t *cavit_param
    = cs_get_glob_cavitation_parameters();

  /* Water at 20 degC, absolute saturation pressure */
  cavit_param->presat = 2340.;

  /* Rayleigh velocity scale sqrt(dp/rho) ~ 10 m/s, bubble radius */
  cavit_param->uinf = 10.;
  cavit_param->linf = 1.e-3;
}

END_C_DECLS
