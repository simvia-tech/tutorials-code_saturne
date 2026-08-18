/*============================================================================
 * Amr_Vortex_Wake - the square obstacle, carved out of the Cartesian mesh.
 *
 * The bluff body is not meshed as such: the cells it occupies are removed at
 * preprocessing, and the faces freed by the removal become boundary faces
 * carrying the group name given here, which the GUI then treats as a wall.
 * This keeps the case free of any mesh file.
 *============================================================================*/

/* code_saturne version 9.2 */

#include "cs_headers.h"

BEGIN_C_DECLS

/*----------------------------------------------------------------------------
 * Cartesian background mesh.
 *
 * The region of interest is meshed uniformly, and the last part of the domain
 * is stretched towards a far outlet. That buffer is not decoration: vortices
 * reaching a free outlet induce inflow through it, which the condition handles
 * poorly and which shows up as a band of spurious vorticity on the boundary.
 * Coarsening the cells before the outlet damps the vortices numerically, so
 * they arrive weak and the boundary stays quiet.
 *----------------------------------------------------------------------------*/

void
cs_user_mesh_cartesian_define(void)
{
  if (cs_mesh_cartesian_get_number_of_meshes() == 0)
    cs_mesh_cartesian_create(nullptr);

  cs_mesh_cartesian_params_t *mp = cs_mesh_cartesian_by_id(0);

  /* x: uniform from -4 to 10, then stretched from 10 to 30 */
  {
    const int n_u = 140, n_b = 30;
    const cs_real_t h = 0.1, r = 1.09;
    cs_real_t *vx;
    CS_MALLOC(vx, n_u + n_b + 1, cs_real_t);

    for (int i = 0; i <= n_u; i++)
      vx[i] = -4.0 + h*i;

    cs_real_t sum = 0., dx = h;
    for (int i = 0; i < n_b; i++) { sum += dx; dx *= r; }
    const cs_real_t scale = (30.0 - 10.0)/sum;
    dx = h*scale;
    for (int i = 1; i < n_b; i++) { vx[n_u+i] = vx[n_u+i-1] + dx; dx *= r; }
    vx[n_u+n_b] = 30.0;

    cs_mesh_cartesian_define_dir_user(mp, 0, n_u + n_b, vx);
    CS_FREE(vx);
  }

  /* y and z: uniform */
  {
    const int ny = 80;
    cs_real_t *vy;
    CS_MALLOC(vy, ny + 1, cs_real_t);
    for (int i = 0; i <= ny; i++) vy[i] = -4.0 + 8.0*i/(cs_real_t)ny;
    cs_mesh_cartesian_define_dir_user(mp, 1, ny, vy);
    CS_FREE(vy);
  }
  {
    cs_real_t vz[2] = {-0.05, 0.05};
    cs_mesh_cartesian_define_dir_user(mp, 2, 1, vz);
  }
}

/*----------------------------------------------------------------------------*/

void
cs_user_mesh_modify(cs_mesh_t *mesh)
{
  /* Square bar of side D = 1 centred on the origin */

  const char criteria[]  = "box[-0.5, -0.5, -1.0, 0.5, 0.5, 1.0]";
  const char new_group[] = "Obstacle";

  cs_mesh_remove_cells_from_selection_criteria(mesh, criteria, new_group);
}

END_C_DECLS
