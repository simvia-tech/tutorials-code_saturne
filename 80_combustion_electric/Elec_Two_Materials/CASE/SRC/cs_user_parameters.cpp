/*============================================================================
 * Elec_Two_Materials - numerical options.
 *
 * PURPOSE OF THIS FILE
 * --------------------
 * The electrical conductivity is DISCONTINUOUS at the material
 * interface. With the default arithmetic average of the diffusivity at
 * faces, the interface conductance is overestimated and the computed
 * current is about 0.7% too high on this mesh. The finite-volume
 * remedy is classic: the HARMONIC average, which makes the series
 * resistance of the two materials exact. The same option matters for
 * any transported quantity with discontinuous diffusivity (conjugate
 * heat transfer, multi-material conduction).
 *============================================================================*/

/* code_saturne version 9.1 */

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

  /* Harmonic face interpolation of the potential diffusivity (sigma) */

  cs_equation_param_t *eqp
    = cs_field_get_equation_param(cs_field_by_name("elec_pot_r"));

  eqp->imvisf = 1;  /* 0: arithmetic (default), 1: harmonic */
}

END_C_DECLS
