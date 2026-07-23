/*============================================================================
 * Elec_Joule_Bar - electric potential boundary conditions.
 *
 * PURPOSE OF THIS FILE
 * --------------------
 * The Dirichlet values of the electric potential on the electrodes are
 * written by the GUI, but in this version the GUI reader only applies
 * the joule_effect variable boundary conditions when the electric ARC
 * model is active: for the pure Joule model they are silently ignored
 * (the potential then stays at zero everywhere and no heat is ever
 * produced). This file imposes them directly on the boundary zones.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

/*----------------------------------------------------------------------------
 * Set boundary conditions to be applied (definition step).
 *----------------------------------------------------------------------------*/

void
cs_user_boundary_conditions_setup(cs_domain_t  *domain)
{
  CS_UNUSED(domain);

  cs_equation_param_t *eqp
    = cs_field_get_equation_param(cs_field_by_name("elec_pot_r"));

  static cs_real_t pot0 = 0.;   /* grounded electrode [V] */
  static cs_real_t pot1 = 10.;  /* powered electrode  [V] */

  cs_equation_add_bc_by_value(eqp, CS_BC_DIRICHLET, "Electrode0", &pot0);
  cs_equation_add_bc_by_value(eqp, CS_BC_DIRICHLET, "Electrode1", &pot1);
}

END_C_DECLS
