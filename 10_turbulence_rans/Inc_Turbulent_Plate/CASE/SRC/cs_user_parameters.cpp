/*============================================================================
 * User parameters.
 *
 * Verification setting: align the Spalart-Allmaras von Karman constant with
 * the NASA TMR "standard SA" value used by CFL3D / FUN3D.
 *============================================================================*/

#include "cs_headers.h"

BEGIN_C_DECLS

/*----------------------------------------------------------------------------*/
/*
 * General-purpose user parameters, called after the setup is read.
 */
/*----------------------------------------------------------------------------*/

void
cs_user_parameters(cs_domain_t *domain)
{
  CS_UNUSED(domain);

  /* code_saturne uses kappa = 0.42 by default, whereas the standard SA model
     (NASA TMR, CFL3D / FUN3D) uses kappa = 0.41. The skin-friction coefficient
     is sensitive to this constant (about +3 to 4 % on Cf for 0.42 vs 0.41), so
     we set it here to compare on equal footing.

     cw1 is derived from kappa (cw1 = cb1/kappa^2 + (1+cb2)/sigma); it is
     normally recomputed in cs_turb_compute_constants(), but we recompute it
     explicitly so the result does not depend on the call order. */

  cs_turb_xkappa = 0.41;
  cs_turb_csaw1  =   cs_turb_csab1 / (cs_turb_xkappa * cs_turb_xkappa)
                   + (1.0 + cs_turb_csab2) / cs_turb_csasig;
}

END_C_DECLS
