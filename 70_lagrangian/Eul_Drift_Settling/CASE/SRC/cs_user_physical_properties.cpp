/*============================================================================
 * Eul_Drift_Settling - relaxation times of the drift scalars.
 *
 * PURPOSE OF THIS FILE
 * --------------------
 * The drift velocity of each scalar is tau_p * g (see
 * cs_drift_convective_flux.cpp): the relaxation time field entirely
 * defines the settling law. Here tau_p is prescribed as an EFFECTIVE
 * constant, tau_p = v_t / g, with v_t the Schiller-Naumann terminal
 * velocity of each glass bead class in water, so that the scalars
 * settle at exactly the same velocities as the discrete particles of
 * the companion case Lag_Settling_Particle.
 *
 * (The plain Stokes choice of the official example,
 * tau_p = rho_p d^2 / (18 mu), would ignore both the buoyancy of the
 * beads and the finite-Reynolds drag correction: fine for dust in a
 * gas, 2.5 times off for a 300 um glass bead in water.)
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

void
cs_user_physical_properties(cs_domain_t  *domain)
{
  const cs_lnum_t n_cells = domain->mesh->n_cells;

  /* Schiller-Naumann terminal velocities of 50/150/300 um glass beads
     (rho_p = 2500) in water at 20 degC, divided by |g| */

  const char *names[] = {"c50", "c150", "c300"};
  const cs_real_t v_t[] = {1.9856e-3, 1.46494e-2, 4.02670e-2};
  const cs_real_t g = 9.81;

  for (int i = 0; i < 3; i++) {

    cs_field_t *f_tau
      = cs_field_by_composite_name(names[i], "drift_tau");

    const cs_real_t taup = v_t[i] / g;

    for (cs_lnum_t c_id = 0; c_id < n_cells; c_id++)
      f_tau->val[c_id] = taup;
  }
}

END_C_DECLS
