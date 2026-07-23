/*============================================================================
 * User source term for the temperature equation.
 *============================================================================*/

/*----------------------------------------------------------------------------
 * PURPOSE OF THIS FILE
 *
 * The mesh is PERIODIC in x: what leaves through the right face re-enters
 * through the left face. Meanwhile, the pin walls heat the fluid with an
 * imposed flux. Without correction, heat is added at every pass and nothing
 * removes it: the temperature would grow indefinitely and the computation
 * would never reach a steady state.
 *
 * Remedy: remove from the volume, at every iteration, exactly the power
 * added by the walls, through a volumetric heat sink. This is the
 * source-term counterpart of the linear temperature drift in the
 * streamwise-periodic decomposition of Patankar, Liu & Sparrow (1977).
 *
 * More heat is removed where the fluid moves faster (proportional to u_x),
 * and the volume integral of the sink is exactly the opposite of the wall
 * heat input.
 *
 * Formula applied in each cell i:
 *     st_exp[i] = volume_i * u_x_i * tot_flux / u_mean
 * with tot_flux = (wall flux) * (heated surface) / (total volume).
 *
 * Reference: Patankar, Liu & Sparrow, J. Heat Transfer 99(2), 1977.
 *----------------------------------------------------------------------------*/

#include "cs_headers.h"

#include <math.h>

BEGIN_C_DECLS

/*----------------------------------------------------------------------------*/
/*
 * Called automatically by code_saturne for EACH solved variable.
 * Only the temperature is handled here.
 *
 * st_exp : explicit part of the source term (in W, already times volume)
 * st_imp : implicit part (left at zero here)
 */
/*----------------------------------------------------------------------------*/

void
cs_user_source_terms(cs_domain_t  *domain,
                     int           f_id,
                     cs_real_t    *st_exp,
                     cs_real_t    *st_imp)
{
  CS_UNUSED(domain);

  /* Field (variable) currently being solved by code_saturne. */
  const cs_field_t *f = cs_field_by_id(f_id);

  /* Act ONLY on the temperature. For the other variables (velocity,
     k, omega...), return immediately without doing anything. */
  if (f != cs_thermal_model_field())
    return;

  /* --- Shortcuts to the mesh data --- */
  const cs_lnum_t    n_cells     = cs_glob_mesh->n_cells;               /* number of cells        */
  const cs_real_t   *cell_vol    = cs_glob_mesh_quantities->cell_vol;   /* volume of each cell    */
  const cs_real_t   *b_face_surf = cs_glob_mesh_quantities->b_face_surf;/* boundary face surfaces */
  const cs_real_3_t *vel         = (const cs_real_3_t *)(CS_F_(vel)->val); /* velocity (3 comp.) */
  const cs_real_t    vol_total   = cs_glob_mesh_quantities->tot_vol;    /* total domain volume    */

  /* ========================================================================
   * STEP 1: total heat power added by the heated walls.
   *
   * Sum the surfaces of the 3 pin walls (wall1, wall2, wall3).
   * The flux "qflux" keeps the same sign as the boundary condition in
   * setup.xml (Neumann = -5e5). The negative sign means heat is removed
   * (a sink), which is exactly what we want.
   * ====================================================================== */
  const cs_real_t qflux = -500000.0;                 /* wall flux [W/m2] */
  const char *heated_walls[] = {"wall1", "wall2", "wall3"};

  cs_real_t surface = 0.0;
  for (int k = 0; k < 3; k++) {
    const cs_zone_t *zone = cs_boundary_zone_by_name(heated_walls[k]);
    for (cs_lnum_t j = 0; j < zone->n_elts; j++)
      surface += b_face_surf[zone->elt_ids[j]];
  }
  cs_parall_sum(1, CS_REAL_TYPE, &surface);          /* sum over all ranks (parallel runs) */

  /* Volumetric coefficient: its integral over the domain equals
     qflux*surface, i.e. the opposite of the wall heat input. Unit: W/m3.
     Remark: rho and Cp cancel out, they do not appear here. */
  const cs_real_t tot_flux = qflux * surface / vol_total;

  /* ========================================================================
   * STEP 2: mean velocity in the x direction (u_mean).
   *
   * u_mean = volume-weighted average of u_x.
   * ====================================================================== */
  cs_real_t u_mean = 0.0;
  for (cs_lnum_t i = 0; i < n_cells; i++)
    u_mean += vel[i][0] * cell_vol[i];

  cs_parall_sum(1, CS_REAL_TYPE, &u_mean);           /* sum over all ranks */
  u_mean /= vol_total;

  /* ========================================================================
   * STEP 3: set the source term in each cell.
   *
   * Heat is removed proportionally to the local velocity u_x.
   * st_imp stays at zero (no implicit part).
   * ====================================================================== */
  for (cs_lnum_t i = 0; i < n_cells; i++) {
    st_imp[i] = 0.0;
    st_exp[i] = cell_vol[i] * vel[i][0] * tot_flux / u_mean;
  }
}

/*----------------------------------------------------------------------------*/

END_C_DECLS
