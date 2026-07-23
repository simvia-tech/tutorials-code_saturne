/*============================================================================
 * Inc_LES_Channel - initialize a perturbed field to trigger turbulence.
 *
 * A laminar start would stay laminar: the LES needs an initial disturbance to
 * transition. We superimpose on a plug-like mean profile U(y) both coherent
 * perturbations (sinusoidal modes, zero at the walls) and random white noise.
 * The constant momentum source then sustains the turbulence to a statistically
 * steady state. Wall units: h = 1, u_tau = 1.
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

void
cs_user_initialization(cs_domain_t  *domain)
{
  const cs_mesh_t  *m = domain->mesh;
  const cs_real_3_t  *cell_cen
    = (const cs_real_3_t *)domain->mesh_quantities->cell_cen;

  cs_real_3_t  *vel = (cs_real_3_t *)CS_F_(vel)->val;

  const cs_real_t  pi = cs_math_pi;
  const cs_real_t  lx = 2.0*pi, lz = pi;   /* box dimensions */
  const cs_real_t  uc = 18.0;              /* core mean velocity (wall units) */
  const cs_real_t  ac = 4.0;               /* coherent perturbation amplitude */
  const cs_real_t  an = 2.5;               /* random noise amplitude */

  for (cs_lnum_t c_id = 0; c_id < m->n_cells; c_id++) {

    const cs_real_t  x = cell_cen[c_id][0];
    const cs_real_t  y = cell_cen[c_id][1];
    const cs_real_t  z = cell_cen[c_id][2];

    const cs_real_t  env = 1.0 - y*y;              /* zero at both walls */
    const cs_real_t  umean = uc*(1.0 - pow(fabs(y), 6.0));

    /* random white noise in [-1,1] on the three components */
    cs_real_t  r[3];
    cs_random_uniform(3, r);

    vel[c_id][0] = umean
                 + ac*sin(2.0*pi*z/lz)*env
                 + an*(2.0*r[0] - 1.0);
    vel[c_id][1] = ac*sin(2.0*pi*x/lx)*env
                 + an*(2.0*r[1] - 1.0);
    vel[c_id][2] = ac*sin(2.0*pi*x/lx)*sin(2.0*pi*z/lz)*env
                 + an*(2.0*r[2] - 1.0);
  }
}

END_C_DECLS
