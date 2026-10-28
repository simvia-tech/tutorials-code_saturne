/*============================================================================
 * Return-to-equilibrium step of the homogeneous two-phase model.
 *
 * In code_saturne 9.1 the solver only calls this step when the compressible
 * flag equals 1, whereas the fractions exist for the value 2: the step is
 * therefore never applied by the solver itself. It is applied here, at the
 * end of each time step, which respects the fractional-step algorithm of the
 * model (convection of the fractions, then relaxation at constant density,
 * velocity and total energy).
 *
 * The notebook variable "relax" (0 by default) switches it on:
 *   code_saturne run --notebook-args relax=1
 *============================================================================*/

#include "cs_headers.h"

BEGIN_C_DECLS

/*----------------------------------------------------------------------------*/
/*!
 * \brief This function is called at the end of each time step.
 *
 * \param[in, out]  domain   pointer to a cs_domain_t structure
 */
/*----------------------------------------------------------------------------*/

void
cs_user_extra_operations([[maybe_unused]] cs_domain_t   *domain)
{
  const int relax = (int)cs_notebook_parameter_value_by_name("relax");

  if (relax > 0 && cs_glob_cf_model->hgn_relax_eq_st >= 0)
    cs_hgn_source_terms_step(domain->mesh);
}

END_C_DECLS
