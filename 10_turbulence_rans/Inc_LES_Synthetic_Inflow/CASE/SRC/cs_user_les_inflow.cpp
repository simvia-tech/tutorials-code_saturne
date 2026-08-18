/*============================================================================
 * Inc_LES_Synthetic_Inflow - synthetic turbulence at an LES inlet.
 *
 * PURPOSE OF THIS FILE
 * --------------------
 * This routine IS the tutorial's showcased feature. An LES needs unsteady,
 * three-dimensional fluctuations at its inlet: a steady mean profile would
 * enter the domain as a laminar sheet and the calculation would have to
 * create turbulence on its own, over a long distance.
 *
 * cs_les_inflow_add_inlet() registers the inlet zone with a synthetic
 * turbulence generator. Here the Synthetic Eddy Method (SEM) is used: the
 * generator populates the inlet plane with a population of eddies that are
 * convected through it, producing a fluctuating velocity field whose
 * statistics match the prescribed reference values.
 *
 * The reference values below are those of the channel this case reproduces
 * (Re_tau = 180, wall units: rho = u_tau = h = 1):
 *   - vel_r  mean bulk velocity,
 *   - k_r    turbulent kinetic energy averaged over the channel,
 *   - eps_r  dissipation rate representative of the channel core.
 * Together they set the velocity and length scales of the eddies,
 * l = k^(3/2) / eps. Taking instead the volume-averaged dissipation, which the
 * near-wall peak dominates, yields eddies smaller than a cell: the solver then
 * clips them to the grid size and reports it in the log.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

/*----------------------------------------------------------------------------
 * Define parameters of synthetic turbulence at LES inflow.
 *----------------------------------------------------------------------------*/

void
cs_user_les_inflow_define(void)
{
  /* Do not read or write the inflow restart file: the eddies are
     regenerated from scratch at the beginning of the calculation. */

  cs_les_inflow_set_restart(false,   /* allow_read  */
                            false);  /* allow_write */

  {
    /* Number of eddies convected through the inlet plane */

    const int n_entities = 200;

    /* Reference statistics of the target flow */

    cs_real_t vel_r[3] = {15.6, 0., 0.};   /* mean velocity      */
    cs_real_t k_r      = 2.0;              /* turbulent energy   */
    cs_real_t eps_r    = 5.0;              /* dissipation rate   */

    cs_les_inflow_add_inlet(CS_INFLOW_SEM,
                            false,        /* inlet plane, not volume mode */
                            cs_boundary_zone_by_name("inlet"),
                            n_entities,
                            1,            /* verbosity */
                            vel_r,
                            k_r,
                            eps_r);
  }
}

/*----------------------------------------------------------------------------*/

END_C_DECLS