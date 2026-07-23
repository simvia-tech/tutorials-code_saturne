/*============================================================================
 * Elec_Imposed_Power - electrode potentials with automatic rescaling.
 *
 * PURPOSE OF THIS FILE
 * --------------------
 * As in Elec_Joule_Bar, the electrode potentials must be imposed here
 * (GUI limitation of the Joule variant in this version). But with the
 * potential SCALING enabled (variable_scaling, ielcor = 1), the module
 * multiplies the electrode potentials each time step by its scaling
 * coefficient coejou, so that the dissipated power converges to the
 * imposed one. The Dirichlet value below is therefore only a SEED
 * shape: it must be evaluated through a function that applies coejou,
 * exactly as the GUI does for the arc model.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

/*----------------------------------------------------------------------------
 * dof function: seed potential rescaled by the module's coejou.
 *----------------------------------------------------------------------------*/

static void
_rescaled_pot(cs_lnum_t         n_elts,
              const cs_lnum_t  *elt_ids,
              bool              dense_output,
              void             *input,
              cs_real_t        *retval)
{
  const cs_real_t seed = *((const cs_real_t *)input);
  const cs_real_t pot_val = seed * cs_glob_elec_option->coejou;

  if (dense_output || elt_ids == nullptr) {
    for (cs_lnum_t i = 0; i < n_elts; i++)
      retval[i] = pot_val;
  }
  else {
    for (cs_lnum_t i = 0; i < n_elts; i++)
      retval[elt_ids[i]] = pot_val;
  }
}

/*----------------------------------------------------------------------------
 * Set boundary conditions to be applied (definition step).
 *----------------------------------------------------------------------------*/

void
cs_user_boundary_conditions_setup(cs_domain_t  *domain)
{
  CS_UNUSED(domain);

  cs_equation_param_t *eqp
    = cs_field_get_equation_param(cs_field_by_name("elec_pot_r"));

  /* Grounded electrode: 0 V (invariant under rescaling) */

  static cs_real_t pot0 = 0.;
  cs_equation_add_bc_by_value(eqp, CS_BC_DIRICHLET, "Electrode0", &pot0);

  /* Powered electrode: seed value of 10 V, rescaled by coejou at every
     evaluation. With R = 10 ohm and P = 40 W imposed, the converged
     potential difference is U = sqrt(P R) = 20 V, i.e. coejou = 2. */

  static cs_real_t pot1_seed = 10.;
  cs_equation_add_bc_by_dof_func(eqp,
                                 CS_BC_DIRICHLET,
                                 "Electrode1",
                                 cs_flag_boundary_face,
                                 _rescaled_pot,
                                 &pot1_seed);
}

END_C_DECLS
