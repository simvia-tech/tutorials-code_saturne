/*============================================================================
 * Relaxation time scale of the homogeneous two-phase model.
 *
 * The fractions relax towards their equilibrium values with the time scale
 * tau returned here for each cell:
 *   phi^{n+1} = exp(-dt/tau) phi^n + (1 - exp(-dt/tau)) phi_eq.
 * The code default is tau = 1e-30 s (instantaneous equilibrium). A uniform
 * value is imposed here from the notebook variable "tau" of setup.xml; a
 * closure law would use the equilibrium fractions, the internal energy and
 * the specific volume received in argument.
 *============================================================================*/

#include "cs_headers.h"

BEGIN_C_DECLS

/*----------------------------------------------------------------------------*/
/*!
 * \brief Define the relaxation time scale towards thermodynamic equilibrium.
 *
 * \param[in]   mesh       pointer to mesh
 * \param[in]   alpha_eq   equilibrium volume fraction
 * \param[in]   y_eq       equilibrium mass fraction
 * \param[in]   z_eq       equilibrium energy fraction
 * \param[in]   ei         specific internal energy
 * \param[in]   v          specific volume
 * \param[out]  relax_tau  relaxation time scale (s), initialized to 1e-30
 */
/*----------------------------------------------------------------------------*/

void
cs_user_hgn_thermo_relax_time(const cs_mesh_t                   *mesh,
                              [[maybe_unused]] const cs_real_t  *alpha_eq,
                              [[maybe_unused]] const cs_real_t  *y_eq,
                              [[maybe_unused]] const cs_real_t  *z_eq,
                              [[maybe_unused]] const cs_real_t  *ei,
                              [[maybe_unused]] const cs_real_t  *v,
                              cs_real_t                         *relax_tau)
{
  const cs_real_t tau = cs_notebook_parameter_value_by_name("tau");

  if (tau > 0.) {
    for (cs_lnum_t c_id = 0; c_id < mesh->n_cells; c_id++)
      relax_tau[c_id] = tau;
  }
}

END_C_DECLS
