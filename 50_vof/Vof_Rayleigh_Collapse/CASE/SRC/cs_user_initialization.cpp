/*============================================================================
 * User initialization for the Rayleigh bubble collapse (cavitation model).
 *
 * A spherical vapor bubble (radius R0, centered at the origin) is
 * initialized at rest in liquid water at ambient pressure. One octant
 * is computed, with symmetry planes at x=0, y=0, z=0. The bubble
 * collapses under the ambient overpressure; the collapse time is
 * compared to the Rayleigh solution.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_defs.h"

#include <assert.h>
#include <math.h>

#if defined(HAVE_MPI)
#include <mpi.h>
#endif

#include <ple_coupling.h>

#include "cs_headers.h"

/*----------------------------------------------------------------------------*/

BEGIN_C_DECLS

void
cs_user_initialization(cs_domain_t  *domain)
{
  const cs_lnum_t n_cells = domain->mesh->n_cells;

  /* Do not reinitialize on restart */
  if (domain->time_step->nt_prev > 0)
    return;

  /* ---- Parameters ---- */

  const cs_real_t R0 = 1.e-3;        /* initial bubble radius [m] */
  const cs_real_t thickness = 1.e-4; /* interface smoothing thickness [m] */

  const cs_real_t (*cell_cen)[3] =
    domain->mesh_quantities->cell_cen;

  cs_field_t *vf = cs_field_by_name_try("void_fraction");
  if (vf == nullptr)
    bft_error(__FILE__, __LINE__, 0,
              "VOF fraction field not found.");

  cs_real_t *vel = CS_F_(vel)->val;

  for (cs_lnum_t c_id = 0; c_id < n_cells; c_id++) {

    const cs_real_t x = cell_cen[c_id][0];
    const cs_real_t y = cell_cen[c_id][1];
    const cs_real_t z = cell_cen[c_id][2];

    const cs_real_t r = sqrt(x*x + y*y + z*z);

    /* vapor (alpha = 1) inside the bubble, liquid outside */
    vf->val[c_id] = 0.5 * (1.0 - tanh((r - R0) / thickness));

    vel[3*c_id]     = 0.;
    vel[3*c_id + 1] = 0.;
    vel[3*c_id + 2] = 0.;
  }
}

END_C_DECLS
