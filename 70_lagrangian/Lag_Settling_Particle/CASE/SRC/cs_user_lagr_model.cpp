/*============================================================================
 * Lag_Settling_Particle - Lagrangian model options.
 *
 * PURPOSE OF THIS FILE
 * --------------------
 * The carrier flow of this case is a QUIESCENT box of water: the k-epsilon
 * model enabled in the GUI is physically inert (the fluid stays at rest)
 * and is only there because the Lagrangian module of this version requires
 * an active turbulence model. For the particle trajectories to remain
 * deterministic (pure drag + gravity, comparable to the analytic laws),
 * the stochastic turbulent dispersion must be disabled; this switch is not
 * exposed in the GUI: it can only be set here.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

/*----------------------------------------------------------------------------
 * Define Lagrangian model options.
 *----------------------------------------------------------------------------*/

void
cs_user_lagr_model(void)
{
  /* Laminar carrier flow: no turbulent dispersion of the particles
     (idistu) and no turbulent diffusion of the fluid seen (idiffl). */

  cs_glob_lagr_model->idistu = 0;
  cs_glob_lagr_model->idiffl = 0;
}

END_C_DECLS
