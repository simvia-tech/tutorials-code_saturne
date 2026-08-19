/*============================================================================
 * Lag_Twoway_Sand - Lagrangian model options.
 *
 * The carrier is laminar, so the stochastic models that stand for the effect of
 * turbulence on the particles have no field to work from and the solver refuses
 * to run with them. Neither switch is exposed in the GUI.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

void
cs_user_lagr_model(void)
{
  cs_glob_lagr_model->idistu = 0;   /* no turbulent dispersion of particles */
  cs_glob_lagr_model->idiffl = 0;   /* no turbulent diffusion, fluid seen   */
}

END_C_DECLS
