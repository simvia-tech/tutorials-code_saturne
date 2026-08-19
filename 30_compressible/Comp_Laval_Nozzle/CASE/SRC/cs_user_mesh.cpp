/*============================================================================
 * Comp_Laval_Nozzle - the nozzle geometry, built without any mesh file.
 *
 * PURPOSE OF THIS FILE
 * --------------------
 * The nozzle is a Cartesian box whose height is squeezed by the area law:
 * a slab is generated in cs_user_mesh_cartesian_define, then every vertex is
 * pulled towards the axis in cs_user_mesh_modify, in proportion to the local
 * half height. Faces normal to x stay planar and vertical, only the horizontal
 * faces tilt, so the mesh remains a structured grid of quadrangles.
 *
 * The point of doing it here rather than in a mesh file is that the area law
 * verified against quasi-1D theory is, literally, the function below: there is
 * no second copy of the geometry to keep in step with this one.
 *
 * The law is a smoothstep on each side of the throat, so dA/dx vanishes at the
 * throat (a genuine sonic section) and at both ends (the inlet and outlet
 * planes are flat, which keeps the boundary conditions one-dimensional).
 *============================================================================*/

/* code_saturne version 9.1 */

#include "cs_headers.h"

BEGIN_C_DECLS

/* Geometry, in metres. The maximum wall slope of the divergent is 6.6 degrees,
   small enough for the quasi-1D approximation to hold. */

static const cs_real_t _length   = 1.00;   /* nozzle length                  */
static const cs_real_t _x_throat = 0.35;   /* throat station                 */
static const cs_real_t _h_inlet  = 0.10;   /* half height at the inlet       */
static const cs_real_t _h_throat = 0.05;   /* half height at the throat      */
static const cs_real_t _h_outlet = 0.10;   /* half height at the outlet      */

/*----------------------------------------------------------------------------
 * Half height of the nozzle, i.e. half the area per unit depth.
 *----------------------------------------------------------------------------*/

static cs_real_t
_half_height(cs_real_t x)
{
  cs_real_t s = (x < _x_throat) ? (_x_throat - x)/_x_throat
                                : (x - _x_throat)/(_length - _x_throat);

  s = cs::min(cs::max(s, 0.), 1.);

  const cs_real_t f = s*s*(3. - 2.*s);          /* smoothstep */

  return (x < _x_throat) ? _h_throat + (_h_inlet  - _h_throat)*f
                         : _h_throat + (_h_outlet - _h_throat)*f;
}

/*----------------------------------------------------------------------------
 * Background slab: the nozzle before it is squeezed.
 *----------------------------------------------------------------------------*/

void
cs_user_mesh_cartesian_define(void)
{
  if (cs_mesh_cartesian_get_number_of_meshes() == 0)
    cs_mesh_cartesian_create(nullptr);

  cs_mesh_cartesian_params_t *mp = cs_mesh_cartesian_by_id(0);

  /* x: uniform along the nozzle */
  {
    const int nx = 500;
    cs_real_t *vx;
    CS_MALLOC(vx, nx + 1, cs_real_t);
    for (int i = 0; i <= nx; i++)
      vx[i] = _length*i/(cs_real_t)nx;
    cs_mesh_cartesian_define_dir_user(mp, 0, nx, vx);
    CS_FREE(vx);
  }

  /* y: uniform over the widest section, which the deformation then contracts */
  {
    const int ny = 50;
    cs_real_t *vy;
    CS_MALLOC(vy, ny + 1, cs_real_t);
    for (int i = 0; i <= ny; i++)
      vy[i] = -_h_inlet + 2.*_h_inlet*i/(cs_real_t)ny;
    cs_mesh_cartesian_define_dir_user(mp, 1, ny, vy);
    CS_FREE(vy);
  }

  /* z: one cell, the case is plane */
  {
    cs_real_t vz[2] = {0., 0.01};
    cs_mesh_cartesian_define_dir_user(mp, 2, 1, vz);
  }
}

/*----------------------------------------------------------------------------
 * Squeeze the slab into the nozzle.
 *----------------------------------------------------------------------------*/

void
cs_user_mesh_modify(cs_mesh_t *mesh)
{
  cs_real_3_t *vtx = (cs_real_3_t *)mesh->vtx_coord;

  for (cs_lnum_t v_id = 0; v_id < mesh->n_vertices; v_id++)
    vtx[v_id][1] *= _half_height(vtx[v_id][0])/_h_inlet;

  mesh->modified |= CS_MESH_MODIFIED;
}

END_C_DECLS
